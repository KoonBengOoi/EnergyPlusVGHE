// Verticalv2.cpp
// (cleaned and corrected copy of user-provided source; removed nested function inside constructor)

#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/GroundHeatExchangers/BoreholeArray.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/GroundHeatExchangers/Vertical.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus::GroundHeatExchangers {

struct GradientSegment {
    double zStart;
    double zEnd;
    double gradient;
    double Tref;
};

GLHEVert::GLHEVert(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j)
{
    // Check for duplicates
    for (auto &existingObj : state.dataGroundHeatExchanger->verticalGLHE) {
        if (objName == existingObj.name) {
            ShowFatalError(state, EnergyPlus::format("Invalid input for {} object: Duplicate name found: {}", moduleName, existingObj.name));
        }
    }

    bool errorsFound = false;

    this->name = objName;

    // get inlet node num
    std::string const inletNodeName = Util::makeUPPER(j["inlet_node_name"].get<std::string>());

    this->inletNodeNum = Node::GetOnlySingleNode(state,
                                                 inletNodeName,
                                                 errorsFound,
                                                 Node::ConnectionObjectType::GroundHeatExchangerSystem,
                                                 objName,
                                                 Node::FluidType::Water,
                                                 Node::ConnectionType::Inlet,
                                                 Node::CompFluidStream::Primary,
                                                 Node::ObjectIsNotParent);

    // get outlet node num
    std::string const outletNodeName = Util::makeUPPER(j["outlet_node_name"].get<std::string>());

    this->outletNodeNum = Node::GetOnlySingleNode(state,
                                                  outletNodeName,
                                                  errorsFound,
                                                  Node::ConnectionObjectType::GroundHeatExchangerSystem,
                                                  objName,
                                                  Node::FluidType::Water,
                                                  Node::ConnectionType::Outlet,
                                                  Node::CompFluidStream::Primary,
                                                  Node::ObjectIsNotParent);
    this->available = true;
    this->on = true;

    Node::TestCompSet(state, moduleName, objName, inletNodeName, outletNodeName, "Condenser Water Nodes");

    this->designFlow = j["design_flow_rate"].get<Real64>();
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->inletNodeNum, this->designFlow);

    this->soil.k = j["ground_thermal_conductivity"].get<Real64>();
    this->soil.rhoCp = j["ground_thermal_heat_capacity"].get<Real64>();

    if (j.find("ghe_vertical_responsefactors_object_name") != j.end()) {
        // Response factors come from IDF object
        this->myRespFactors = GetResponseFactor(state, Util::makeUPPER(j["ghe_vertical_responsefactors_object_name"].get<std::string>()));
        this->gFunctionsExist = true;
    }

    // no g-functions in the input file, so they need to be calculated
    if (!this->gFunctionsExist) {

        // g-function calculation method
        if (j.find("g_function_calculation_method") != j.end()) {
            std::string gFunctionMethodStr = Util::makeUPPER(j["g_function_calculation_method"].get<std::string>());
            if (gFunctionMethodStr == "UHFCALC") {
                this->gFuncCalcMethod = GFuncCalcMethod::UniformHeatFlux;
            } else if (gFunctionMethodStr == "UBHWTCALC") {
                this->gFuncCalcMethod = GFuncCalcMethod::UniformBoreholeWallTemp;
            } else if (gFunctionMethodStr == "FULLDESIGN") {
                this->gFuncCalcMethod = GFuncCalcMethod::FullDesign;
            } else {
                errorsFound = true;
                ShowSevereError(state, fmt::format("g-Function Calculation Method: \"{}\" is invalid", gFunctionMethodStr));
            }
        }

        // get borehole data from array or individual borehole instance objects
        if (this->gFuncCalcMethod == GFuncCalcMethod::FullDesign) {
#ifndef PYTHON_CLI
            ShowFatalError(state, "Attempted to use borehole field design in a build without PYTHON_CLI, which is invalid");
#endif
            // g-functions won't be calculated until after sizing is complete
            bool foundSizing = false;
            bool objTypeFound = j.find("ghe_vertical_sizing_object_type") != j.end();
            bool objNameFound = j.find("ghe_vertical_sizing_object_name") != j.end();

            if (!objTypeFound) {
                ShowSevereError(state, EnergyPlus::format("GroundHeatExchanger:System \"{}\"", this->name));
                ShowContinueError(
                    state, EnergyPlus::format("g-Function Calculation Method = \"{}\"", j["g_function_calculation_method"].get<std::string>()));
                ShowContinueError(state, "GHE:Vertical:Sizing Object Type not specified.");
                errorsFound = true;
            }
            if (!objNameFound) {
                ShowSevereError(state, EnergyPlus::format("GroundHeatExchanger:System \"{}\"", this->name));
                ShowContinueError(
                    state, EnergyPlus::format("g-Function Calculation Method = \"{}\"", j["g_function_calculation_method"].get<std::string>()));
                ShowContinueError(state, "GHE:Vertical:Sizing Object Name not specified.");
                errorsFound = true;
            }

            this->sizingData.name = j.at("ghe_vertical_sizing_object_name");
            this->sizingData.type = j.at("ghe_vertical_sizing_object_type");

            if (Util::makeUPPER(this->sizingData.type) != "GROUNDHEATEXCHANGER:VERTICAL:SIZING:RECTANGLE") {
                ShowSevereError(state, EnergyPlus::format("GroundHeatExchanger:System \"{}\"", this->name));
                ShowContinueError(state, EnergyPlus::format("GHE:Vertical:Sizing Object Type not supported \"{}\"", this->sizingData.type));
                errorsFound = true;
            }

            auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find("GroundHeatExchanger:Vertical:Sizing:Rectangle");
            if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
                ShowSevereError(
                    state,
                    EnergyPlus::format("Expected to find GroundHeatExchanger:Vertical:Sizing named {}, but it was missing", this->sizingData.name));
                errorsFound = true;
            }

            auto &instanceValues = instances.value();
            for (auto instance = instanceValues.begin(); instance != instanceValues.end(); ++instance) {
                auto const &fields = instance.value();
                std::string const &thisSizingObjName = instance.key();
                std::string const &objNameUC = Util::makeUPPER(thisSizingObjName);
                if (objNameUC == Util::makeUPPER(this->sizingData.name)) {
                    foundSizing = true;

                    this->sizingData.sizingPeriodName = fields.at("sizingperiod_weatherfiledays_name");
                    auto const spInstances = state.dataInputProcessing->inputProcessor->epJSON.find("SizingPeriod:WeatherFileDays");
                    if (spInstances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
                        ShowSevereError(state,
                                        EnergyPlus::format("Expected to find SizingPeriod:WeatherFileDays named {}, but it was missing",
                                                           this->sizingData.sizingPeriodName));
                        errorsFound = true;
                    }

                    bool spIsAnnual = false;
                    for (auto &designPeriod : state.dataWeather->RunPeriodDesignInput) {
                        if (Util::makeUPPER(designPeriod.title) == Util::makeUPPER((this->sizingData.sizingPeriodName)) &&
                            (designPeriod.totalDays == 365)) {
                            spIsAnnual = true;
                            break;
                        }
                    }

                    if (!spIsAnnual) {
                        ShowSevereError(state,
                                        EnergyPlus::format("SizingPeriod:WeatherFileDays named {}, must be an annual design period of 365 days",
                                                           this->sizingData.sizingPeriodName));
                        errorsFound = true;
                    }

                    if (auto it = fields.find("design_flow_rate_per_borehole"); it != fields.end()) {
                        this->sizingData.designFlowRatePerBorehole = it.value().get<Real64>();
                    } else {
                        state.dataInputProcessing->inputProcessor->getDefaultValue(
                            state, this->sizingData.type, "design_flow_rate_per_borehole", this->sizingData.designFlowRatePerBorehole);
                    }

                    this->sizingData.length = fields.at("available_borehole_field_length");
                    this->sizingData.width = fields.at("available_borehole_field_width");
                    this->sizingData.numBoreholes = fields.at("maximum_number_of_boreholes");

                    if (auto it = fields.find("minimum_borehole_spacing"); it != fields.end()) {
                        this->sizingData.minSpacing = it.value().get<Real64>();
                    } else {
                        state.dataInputProcessing->inputProcessor->getDefaultValue(
                            state, this->sizingData.type, "minimum_borehole_spacing", this->sizingData.minSpacing);
                    }

                    if (auto it = fields.find("maximum_borehole_spacing"); it != fields.end()) {
                        this->sizingData.maxSpacing = it.value().get<Real64>();
                    } else {
                        state.dataInputProcessing->inputProcessor->getDefaultValue(
                            state, this->sizingData.type, "maximum_borehole_spacing", this->sizingData.maxSpacing);
                    }

                    if (auto it = fields.find("minimum_borehole_vertical_length"); it != fields.end()) {
                        this->sizingData.minLength = it.value().get<Real64>();
                    } else {
                        state.dataInputProcessing->inputProcessor->getDefaultValue(
                            state, this->sizingData.type, "minimum_borehole_vertical_length", this->sizingData.minLength);
                    }

                    if (auto it = fields.find("maximum_borehole_vertical_length"); it != fields.end()) {
                        this->sizingData.maxLength = it.value().get<Real64>();
                    } else {
                        state.dataInputProcessing->inputProcessor->getDefaultValue(
                            state, this->sizingData.type, "maximum_borehole_vertical_length", this->sizingData.maxLength);
                    }

                    if (auto it = fields.find("minimum_exiting_fluid_temperature_for_sizing"); it != fields.end()) {
                        this->sizingData.minEFT = it.value().get<Real64>();
                    } else {
                        state.dataInputProcessing->inputProcessor->getDefaultValue(
                            state, this->sizingData.type, "minimum_exiting_fluid_temperature_for_sizing", this->sizingData.minEFT);
                    }

                    if (auto it = fields.find("maximum_exiting_fluid_temperature_for_sizing"); it != fields.end()) {
                        this->sizingData.maxEFT = it.value().get<Real64>();
                    } else {
                        state.dataInputProcessing->inputProcessor->getDefaultValue(
                            state, this->sizingData.type, "maximum_exiting_fluid_temperature_for_sizing", this->sizingData.maxEFT);
                    }

                    state.dataInputProcessing->inputProcessor->markObjectAsUsed("GroundHeatExchanger:Vertical:Sizing:Rectangle",
                                                                                this->sizingData.name);
                    break;
                }
            }

            if (!foundSizing) {
                ShowSevereError(state, "Could not find matching GroundHeatExchanger:Vertical:Sizing:Rectangle");
                errorsFound = true;
            }
            // Need to construct response factors with a single borehole representation, then later we'll override the system g-function
            if (j.find("vertical_well_locations") == j.end()) {
                ShowSevereError(state, "For a full design GHE simulation, you must provide a GHE:Vertical:Single object");
                ShowContinueError(state, "If you enter more than one, only the first is used to specify the borehole design");
                ShowContinueError(state, EnergyPlus::format("Check references to these objects for GHE:System object: {}", this->name));
                errorsFound = true;
            }

            std::vector<std::shared_ptr<GLHEVertSingle>> tempVectOfBHObjects;
            auto const &vars = j.at("vertical_well_locations");
            for (auto const &var : vars) {
                if (!var.at("ghe_vertical_single_object_name").empty()) {
                    std::shared_ptr<GLHEVertSingle> tempBHptr =
                        GLHEVertSingle::GetSingleBH(state, Util::makeUPPER(var.at("ghe_vertical_single_object_name").get<std::string>()));
                    tempVectOfBHObjects.push_back(tempBHptr);
                    this->myRespFactors = BuildAndGetResponseFactorsObjectFromSingleBHs(state, tempVectOfBHObjects);
                }
                break;
            }
            if (!this->myRespFactors) {
                ShowSevereError(state, "Something went wrong creating response factor for GroundHeatExchanger, check previous errors.");
                errorsFound = true;
            }

        } else if (j.find("ghe_vertical_array_object_name") != j.end()) {
            // Response factors come from array object
            this->myRespFactors = BuildAndGetResponseFactorObjectFromArray(
                state, GLHEVertArray::GetVertArray(state, Util::makeUPPER(j["ghe_vertical_array_object_name"].get<std::string>())));
        } else {
            if (j.find("vertical_well_locations") == j.end()) {
                // No ResponseFactors, GHEArray, or SingleBH object are referenced
                ShowSevereError(state, "No GHE:ResponseFactors, GHE:Vertical:Array, or GHE:Vertical:Single objects found");
                ShowContinueError(state, EnergyPlus::format("Check references to these objects for GHE:System object: {}", this->name));
                errorsFound = true;
            }

            auto const &vars = j.at("vertical_well_locations");

            // Calculate response factors from individual boreholes
            std::vector<std::shared_ptr<GLHEVertSingle>> tempVectOfBHObjects;

            for (auto const &var : vars) {
                if (!var.at("ghe_vertical_single_object_name").empty()) {
                    std::shared_ptr<GLHEVertSingle> tempBHptr =
                        GLHEVertSingle::GetSingleBH(state, Util::makeUPPER(var.at("ghe_vertical_single_object_name").get<std::string>()));
                    tempVectOfBHObjects.push_back(tempBHptr);
                } else {
                    break;
                }
            }

            this->myRespFactors = BuildAndGetResponseFactorsObjectFromSingleBHs(state, tempVectOfBHObjects);

            if (!this->myRespFactors) {
                ShowSevereError(state, "GroundHeatExchanger:Vertical:Single objects not found.");
                errorsFound = true;
            }
        }
    }

    this->bhDiameter = this->myRespFactors->props->bhDiameter;
    this->bhRadius = this->bhDiameter / 2.0;
    this->bhLength = this->myRespFactors->props->bhLength;
    this->bhUTubeDist = this->myRespFactors->props->bhUTubeDist;

    // pull pipe and grout data up from response factor struct for simplicity
    this->pipe.outDia = this->myRespFactors->props->pipe.outDia;
    this->pipe.innerDia = this->myRespFactors->props->pipe.innerDia;
    this->pipe.outRadius = this->pipe.outDia / 2;
    this->pipe.innerRadius = this->pipe.innerDia / 2;
    this->pipe.thickness = this->myRespFactors->props->pipe.thickness;
    this->pipe.k = this->myRespFactors->props->pipe.k;
    this->pipe.rhoCp = this->myRespFactors->props->pipe.rhoCp;

    this->grout.k = this->myRespFactors->props->grout.k;
    this->grout.rhoCp = this->myRespFactors->props->grout.rhoCp;

    this->myRespFactors->gRefRatio = this->bhRadius / this->bhLength;

    // Number of simulation years from RunPeriod
    this->myRespFactors->maxSimYears = state.dataEnvrn->MaxNumberSimYears;

    // total tube length
    this->totalTubeLength = this->myRespFactors->numBoreholes * this->myRespFactors->props->bhLength;

    // ground thermal diffusivity
    this->soil.diffusivity = this->soil.k / this->soil.rhoCp;

    // multipole method constants
    this->theta_1 = this->bhUTubeDist / (2 * this->bhRadius);
    this->theta_2 = this->bhRadius / this->pipe.outRadius;
    this->theta_3 = 1 / (2 * this->theta_1 * this->theta_2);
    this->sigma = (this->grout.k - this->soil.k) / (this->grout.k + this->soil.k);

    this->SubAGG = 15;
    this->AGG = 192;

    // Allocation of all the dynamic arrays
    this->QnMonthlyAgg.dimension(static_cast<int>(this->myRespFactors->maxSimYears * 12), 0.0);
    this->QnHr.dimension(730 + this->AGG + this->SubAGG, 0.0);
    this->QnSubHr.dimension(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1), 0.0);
    this->LastHourN.dimension(this->SubAGG + 1, 0);

    this->prevTimeSteps.allocate(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1));
    this->prevTimeSteps = 0.0;

    GroundTemp::ModelType modelType = static_cast<GroundTemp::ModelType>(
        getEnumValue(GroundTemp::modelTypeNamesUC, Util::makeUPPER(j["undisturbed_ground_temperature_model_type"].get<std::string>())));
    assert(modelType != GroundTemp::ModelType::Invalid);

    // Initialize ground temperature model and get pointer reference
    this->groundTempModel =
        GroundTemp::GetGroundTempModelAndInit(state, modelType, Util::makeUPPER(j["undisturbed_ground_temperature_model_name"].get<std::string>()));

    // Check for Errors
    if (errorsFound) {
        ShowFatalError(state, EnergyPlus::format("Errors found in processing input for {}", moduleName));
    }
} // end GLHEVert::GLHEVert


class GLHEVGHE : public GLHEVert {
public:
    std::vector<GradientSegment> segments;

    GLHEVGHE(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j)
        : GLHEVert(state, objName, j)
    {
        if (j.find("gradient_segments") != j.end()) {
            auto const &vars = j.at("gradient_segments");
            for (auto const &var : vars) {
                GradientSegment seg;
                seg.zStart   = var.at("z_start").get<double>();
                seg.zEnd     = var.at("z_end").get<double>();
                seg.gradient = var.at("gradient").get<double>();
                seg.Tref     = var.at("reference_temp").get<double>();
                segments.push_back(seg);
            }
        }
    }

    double calcGroundTemp(double depth) const {
        for (auto const &seg : segments) {
            if (depth >= seg.zStart && depth <= seg.zEnd) {
                return seg.Tref + seg.gradient * (depth - seg.zStart);
            }
        }
        if (!segments.empty()) {
            auto const &last = segments.back();
            return last.Tref + last.gradient * (depth - last.zStart);
        }
        return 15.0; // default fallback
    }
};

void GLHEVert::simulate(EnergyPlusData &state,
                        [[maybe_unused]] const PlantLocation &calledFromLocation,
                        [[maybe_unused]] bool const FirstHVACIteration,
                        [[maybe_unused]] Real64 &CurLoad,
                        [[maybe_unused]] bool const RunFlag)
{

    if (this->needToSetupOutputVars) {
        this->setupOutput(state);
        this->needToSetupOutputVars = false;
    }

    this->initGLHESimVars(state);
    if (state.dataGlobal->KickOffSimulation) {
        return;
    }

    if (this->gFuncCalcMethod == GFuncCalcMethod::FullDesign) {
        // we need to do some special things for the full design mode
        this->outletTemp = this->tempGround;
        this->inletTemp = state.dataLoopNodes->Node(this->inletNodeNum).Temp;
        if (this->fullDesignCompleted) {
            // nothing here
        } else if (!state.dataGlobal->WarmupFlag) {
            if (this->fullDesignLoadAccrualStarted) {
                // if load accrual is already started, continue to accrue until hvac sizing is done
                if (state.dataGlobal->DoingHVACSizingSimulations) {
                    Real64 const cpFluid =
                        state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, "GLHEVert::simulate");
                    Real64 const q = this->massFlowRate * cpFluid * (this->outletTemp - this->inletTemp);
                    Real64 const timeStamp = (state.dataGlobal->DayOfSim - 1) * 24 + state.dataGlobal->CurrentTime;
                    this->loadsDuringSizingForDesign[timeStamp] = q;
                } else {
                    this->fullDesignCompleted = true;
                    if (this->loadsDuringSizingForDesign.size() % 8760 != 0) {
                        ShowFatalError(state, "Bad number of load values found when trying to accumulate ghe loads for design");
                    }
                    std::vector<Real64> timeStepValues;
                    timeStepValues.reserve(this->loadsDuringSizingForDesign.size());
                    for (auto const &kv : this->loadsDuringSizingForDesign) {
                        timeStepValues.push_back(kv.second);
                    }
                    std::vector<Real64> hourlyValues;
                    hourlyValues.reserve(8760);
                    unsigned int const numPerHour = timeStepValues.size() / 8760;
                    for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(timeStepValues.size()); i += numPerHour) {
                        const Real64 sum = std::accumulate(timeStepValues.begin() + i, timeStepValues.begin() + i + numPerHour, 0.0);
                        hourlyValues.push_back(sum / static_cast<double>(numPerHour));
                    }
                    this->performBoreholeFieldDesignAndSizingWithGHEDesigner(state, hourlyValues);
                }
            } else {
                // if load accrual is not started yet, just do nothing until the hvac sizing simulation has begun
                if (state.dataGlobal->DoingHVACSizingSimulations) {
                    this->fullDesignLoadAccrualStarted = true;
                    Real64 const cpFluid =
                        state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, "GLHEVert::simulate");
                    Real64 const q = this->massFlowRate * cpFluid * (this->outletTemp - this->inletTemp);
                    Real64 const timeStamp = (state.dataGlobal->DayOfSim - 1) * 24 + state.dataGlobal->CurrentTime;
                    this->loadsDuringSizingForDesign[timeStamp] = q;
                } else {
                    // nothing
                }
            }
        }
        if (this->fullDesignCompleted) {
            this->calcGroundHeatExchanger(state);
        }
    } else {
        this->calcGroundHeatExchanger(state);
    }
    this->updateGHX(state);
}

void GLHEVert::getAnnualTimeConstant()
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February 2015

    // PURPOSE OF THIS SUBROUTINE:
    // calculate annual time constant for ground conduction

    constexpr Real64 hrInYear = 8760;

    this->timeSS = (pow_2(this->bhLength) / (9.0 * this->soil.diffusivity)) / Constant::rSecsInHour / hrInYear; // Excuse me?
    this->timeSSFactor = this->timeSS * 8760.0;
}

void GLHEVert::combineShortAndLongTimestepGFunctions() const
{
    std::vector<Real64> GFNC_combined;
    std::vector<Real64> LNTTS_combined;

    Real64 const t_s = pow_2(this->bhLength) / (9.0 * this->soil.diffusivity);

    // Nothing to do. Just put the short time step g-functions on the combined vector
    const unsigned int num_shortTimestepGFunctions = GFNC_shortTimestep.size();
    for (size_t index_shortTS = 0; index_shortTS < num_shortTimestepGFunctions; ++index_shortTS) {
        GFNC_combined.push_back(GFNC_shortTimestep[index_shortTS]);
        LNTTS_combined.push_back(LNTTS_shortTimestep[index_shortTS]);
    }

    // the LTS may calculate small values, but let's favor the STS ones up to the high limit of STS calculation
    Real64 const highest_lntts_from_sts = LNTTS_shortTimestep.back();

    // Add the rest of the long time-step g-functions to the combined curve
    for (size_t index_longTS = 0; index_longTS < this->myRespFactors->GFNC.size(); ++index_longTS) {
        if (this->myRespFactors->LNTTS[index_longTS] <= highest_lntts_from_sts) {
            continue;
        }
        GFNC_combined.push_back(this->myRespFactors->GFNC[index_longTS]);
        LNTTS_combined.push_back(this->myRespFactors->LNTTS[index_longTS]);
    }

    this->myRespFactors->time = LNTTS_combined;
    std::transform(this->myRespFactors->time.begin(), this->myRespFactors->time.end(), this->myRespFactors->time.begin(), [&t_s](auto const &c) {
        return exp(c) * t_s;
    });

    this->myRespFactors->LNTTS = LNTTS_combined;
    this->myRespFactors->GFNC = GFNC_combined;
}

std::vector<Real64> GLHEVert::distances(MyCartesian const &point_i, MyCartesian const &point_j)
{
    std::vector<Real64> sumVals;

    // Calculate the distance between points
    sumVals.push_back(pow_2(point_i.x - point_j.x));
    sumVals.push_back(pow_2(point_i.y - point_j.y));
    sumVals.push_back(pow_2(point_i.z - point_j.z));

    Real64 sumTot = 0.0;
    std::vector<Real64> retVals;
    std::for_each(sumVals.begin(), sumVals.end(), [&](Real64 n) { sumTot += n; });
    retVals.push_back(std::sqrt(sumTot));

    // Calculate distance to mirror point
    sumVals.pop_back();
    sumVals.push_back(pow_2(point_i.z - (-point_j.z)));

    sumTot = 0.0;
    std::for_each(sumVals.begin(), sumVals.end(), [&](Real64 n) { sumTot += n; });
    retVals.push_back(std::sqrt(sumTot));

    return retVals;
}

Real64 GLHEVert::calcResponse(std::vector<Real64> const &dists, Real64 const currTime) const
{
    const Real64 pointToPointResponse = erfc(dists[0] / (2 * sqrt(this->soil.diffusivity * currTime))) / dists[0];
    const Real64 pointToReflectedResponse = erfc(dists[1] / (2 * sqrt(this->soil.diffusivity * currTime))) / dists[1];
    return pointToPointResponse - pointToReflectedResponse;
}

Real64 GLHEVert::integral(MyCartesian const &point_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 const currTime) const
{
    Real64 sum_f = 0.0;
    int i = 0;
    int const lastIndex_j = static_cast<int>(bh_j->pointLocations_j.size() - 1u);
    for (auto const &point_j : bh_j->pointLocations_j) {
        std::vector<Real64> dists = distances(point_i, point_j);
        Real64 const f = calcResponse(dists, currTime);

        // Integrate using Simpson's
        if (i == 0 || i == lastIndex_j) {
            sum_f += f;
        } else if (isEven(i)) {
            sum_f += 2 * f;
        } else {
            sum_f += 4 * f;
        }

        ++i;
    }

    return (bh_j->dl_j / 3.0) * sum_f;
}

Real64 GLHEVert::doubleIntegral(std::shared_ptr<GLHEVertSingle> const &bh_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 const currTime) const
{
    if (bh_i == bh_j) {

        Real64 sum_f = 0;
        int i = 0;
        int const lastIndex = static_cast<int>(bh_i->pointLocations_ii.size() - 1u);
        for (auto &thisPoint : bh_i->pointLocations_ii) {

            Real64 f = integral(thisPoint, bh_j, currTime);

            // Integrate using Simpson's
            if (i == 0 || i == lastIndex) {
                sum_f += f;
            } else if (isEven(i)) {
                sum_f += 2 * f;
            } else {
                sum_f += 4 * f;
            }

            ++i;
        }

        return (bh_i->dl_ii / 3.0) * sum_f;
    }
    Real64 sum_f = 0;
    int i = 0;
    int const lastIndex = static_cast<int>(bh_i->pointLocations_i.size() - 1u);
    for (auto const &thisPoint : bh_i->pointLocations_i) {

        Real64 f = integral(thisPoint, bh_j, currTime);

        // Integrate using Simpson's
        if (i == 0 || i == lastIndex) {
            sum_f += f;
        } else if (isEven(i)) {
            sum_f += 2 * f;
        } else {
            sum_f += 4 * f;
        }

        ++i;
    }

    return (bh_i->dl_i / 3.0) * sum_f;
}

void GLHEVert::calcLongTimestepGFunctions(EnergyPlusData &state) const
{
    switch (this->gFuncCalcMethod) {
    case GFuncCalcMethod::UniformHeatFlux:
        this->calcUniformHeatFluxGFunctions(state);
        break;
    case GFuncCalcMethod::UniformBoreholeWallTemp:
        this->calcUniformBHWallTempGFunctionsWithGHEDesigner(state);
        break;
    case GFuncCalcMethod::FullDesign:
        // this->performBoreholeFieldDesignAndSizingWithGHEDesigner(state);
        break;
    default:
        assert(false);
    }
}

nlohmann::json GLHEVert::getCommonGHEDesignerInputs(EnergyPlusData &state) const
{
    nlohmann::json gheDesignerInputs;
    gheDesignerInputs["version"] = 2; // If you update GHEDesigner, you may need to use a new input version here
    gheDesignerInputs["topology"] = {{{"type", "ground_heat_exchanger"}, {"name", "ghe1"}}};

    std::string const p = fmt::format("[G-Function Calculation for GHE Named: {}] ", this->name);

    // set up the fluid to use in GHEDesigner, note that the concentration is more restrictive than in EnergyPlus
    nlohmann::json fluidObject;
    if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).FluidName == "WATER") {
        gheDesignerInputs["fluid"] = {{"fluid_name", "WATER"}, {"concentration_percent", 0}, {"temperature", 20}};
    } else if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).FluidName == "*
