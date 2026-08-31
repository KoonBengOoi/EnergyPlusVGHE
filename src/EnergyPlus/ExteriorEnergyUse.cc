// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
<<<<<<< HEAD
#include <EnergyPlus/DataIPShortCuts.hh>
=======
>>>>>>> nrel/develop
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace ExteriorEnergyUse {

    // MODULE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   January 2001
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module provides the reporting for exterior energy usage.  This usage does not directly
    // affect simulation results for the energy usage in a building but may affect the "metered"
    // usage of a facility.

    void ManageExteriorEnergyUse(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine provides the usual call for the Simulation Manager.

        if (state.dataExteriorEnergyUse->GetExteriorEnergyInputFlag) {
            ExteriorEnergyUse::GetExteriorEnergyUseInput(state);
            state.dataExteriorEnergyUse->GetExteriorEnergyInputFlag = false;
        }

        ExteriorEnergyUse::ReportExteriorEnergyUse(state);
    }

    void GetExteriorEnergyUseInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the Exterior Lights and Equipment.

        // Using/Aliasing
        using namespace OutputReportPredefined;
        // SUBROUTINE PARAMETER DEFINITIONS:
        std::string_view constexpr routineName = "GetExteriorEnergyUseInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
<<<<<<< HEAD
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        std::string EndUseSubcategoryName;

        auto &s_ipsc = state.dataIPShortCut;
=======
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        std::string EndUseSubcategoryName;
        auto *inputProcessor = state.dataInputProcessing->inputProcessor.get();
>>>>>>> nrel/develop

        state.dataExteriorEnergyUse->NumExteriorLights = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Exterior:Lights");
        state.dataExteriorEnergyUse->ExteriorLights.allocate(state.dataExteriorEnergyUse->NumExteriorLights);

        int NumFuelEq = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Exterior:FuelEquipment");
        int NumWtrEq = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Exterior:WaterEquipment");
        state.dataExteriorEnergyUse->ExteriorEquipment.allocate(NumFuelEq + NumWtrEq);
        state.dataExteriorEnergyUse->UniqueExteriorEquipNames.reserve(NumFuelEq + NumWtrEq);

        state.dataExteriorEnergyUse->GetExteriorEnergyInputFlag = false;
        state.dataExteriorEnergyUse->NumExteriorEqs = 0;

        // =================================  Get Exterior Lights
<<<<<<< HEAD
        std::string_view cCurrentModuleObject = "Exterior:Lights";
        for (int Item = 1; Item <= state.dataExteriorEnergyUse->NumExteriorLights; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            state.dataExteriorEnergyUse->ExteriorLights(Item).Name = s_ipsc->cAlphaArgs(1);

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
                ErrorsFound = true;
            } else if ((state.dataExteriorEnergyUse->ExteriorLights(Item).sched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            } else if (int SchMin = state.dataExteriorEnergyUse->ExteriorLights(Item).sched->getMinVal(state); SchMin < 0.0) {
                ShowSevereCustom(
                    state,
                    eoh,
                    EnergyPlus::format(
                        "{} = {} minimum is [{:.1R}]. Values must be >= 0.0.", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), SchMin));
                ErrorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode = ExteriorEnergyUse::LightControlType::ScheduleOnly;
            } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(3), "ScheduleNameOnly")) {
                state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode = ExteriorEnergyUse::LightControlType::ScheduleOnly;
            } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(3), "AstronomicalClock")) {
                state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode = ExteriorEnergyUse::LightControlType::AstroClockOverride;
            } else {
                ShowSevereInvalidKey(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3));
            }

            if (NumAlphas > 3) {
                EndUseSubcategoryName = state.dataIPShortCut->cAlphaArgs(4);
            } else {
                EndUseSubcategoryName = "General";
            }

            state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel = state.dataIPShortCut->rNumericArgs(1);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "ExteriorLights",
                                 state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                 "Electricity Rate",
                                 "W",
                                 state.dataExteriorEnergyUse->ExteriorLights(Item).PowerActuatorOn,
                                 state.dataExteriorEnergyUse->ExteriorLights(Item).PowerActuatorValue);
            }

            SetupOutputVariable(state,
                                "Exterior Lights Electricity Rate",
                                Constant::Units::W,
                                state.dataExteriorEnergyUse->ExteriorLights(Item).Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataExteriorEnergyUse->ExteriorLights(Item).Name);

            SetupOutputVariable(state,
                                "Exterior Lights Electricity Energy",
                                Constant::Units::J,
                                state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Invalid,
                                OutputProcessor::EndUseCat::ExteriorLights,
                                EndUseSubcategoryName);

            // entries for predefined tables
            PreDefTableEntry(state,
                             state.dataOutRptPredefined->pdchExLtPower,
                             state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                             state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel);
            state.dataExteriorEnergyUse->sumDesignLevel += state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel;
            if (state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode ==
                ExteriorEnergyUse::LightControlType::AstroClockOverride) { // photocell/schedule
                PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchExLtClock, state.dataExteriorEnergyUse->ExteriorLights(Item).Name, "AstronomicalClock");
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchExLtSchd, state.dataExteriorEnergyUse->ExteriorLights(Item).Name, "-");
            } else {
                PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchExLtClock, state.dataExteriorEnergyUse->ExteriorLights(Item).Name, "Schedule");
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchExLtSchd,
                                 state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                 state.dataExteriorEnergyUse->ExteriorLights(Item).sched->Name);
=======
        std::string cCurrentModuleObject = "Exterior:Lights";
        auto const &exteriorLightsSchemaProps = inputProcessor->getObjectSchemaProps(state, cCurrentModuleObject);
        auto const exteriorLightsObjects = inputProcessor->epJSON.find(cCurrentModuleObject);
        if (exteriorLightsObjects != inputProcessor->epJSON.end()) {
            int Item = 1;
            for (auto const &lightInstance : exteriorLightsObjects.value().items()) {
                auto const &lightFields = lightInstance.value();
                auto const lightName = Util::makeUPPER(lightInstance.key());
                auto const scheduleName = inputProcessor->getAlphaFieldValue(lightFields, exteriorLightsSchemaProps, "schedule_name");
                auto const controlOption = lightFields.contains("control_option")
                                               ? inputProcessor->getAlphaFieldValue(lightFields, exteriorLightsSchemaProps, "control_option")
                                               : std::string();

                inputProcessor->markObjectAsUsed(cCurrentModuleObject, lightInstance.key());

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, lightName};

                state.dataExteriorEnergyUse->ExteriorLights(Item).Name = lightName;

                if (scheduleName.empty()) {
                    ShowSevereEmptyField(state, eoh, "schedule_name");
                    ErrorsFound = true;
                } else if ((state.dataExteriorEnergyUse->ExteriorLights(Item).sched = Sched::GetSchedule(state, scheduleName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "schedule_name", scheduleName);
                    ErrorsFound = true;
                } else if (int SchMin = state.dataExteriorEnergyUse->ExteriorLights(Item).sched->getMinVal(state); SchMin < 0.0) {
                    ShowSevereCustom(
                        state, eoh, std::format("{} = {} minimum is [{}]. Values must be >= 0.0.", "schedule_name", scheduleName, SchMin));
                    ErrorsFound = true;
                }

                if (controlOption.empty()) {
                    state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode = ExteriorEnergyUse::LightControlType::ScheduleOnly;
                } else if (Util::SameString(controlOption, "ScheduleNameOnly")) {
                    state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode = ExteriorEnergyUse::LightControlType::ScheduleOnly;
                } else if (Util::SameString(controlOption, "AstronomicalClock")) {
                    state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode = ExteriorEnergyUse::LightControlType::AstroClockOverride;
                } else {
                    ShowSevereInvalidKey(state, eoh, "control_option", controlOption);
                }

                if (lightFields.find("end_use_subcategory") != lightFields.end()) {
                    EndUseSubcategoryName = inputProcessor->getAlphaFieldValue(lightFields, exteriorLightsSchemaProps, "end_use_subcategory");
                } else {
                    EndUseSubcategoryName = "General";
                }

                state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel =
                    inputProcessor->getRealFieldValue(lightFields, exteriorLightsSchemaProps, "design_level");
                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "ExteriorLights",
                                     state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                     "Electricity Rate",
                                     "W",
                                     state.dataExteriorEnergyUse->ExteriorLights(Item).PowerActuatorOn,
                                     state.dataExteriorEnergyUse->ExteriorLights(Item).PowerActuatorValue);
                }

                SetupOutputVariable(state,
                                    "Exterior Lights Electricity Rate",
                                    Constant::Units::W,
                                    state.dataExteriorEnergyUse->ExteriorLights(Item).Power,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataExteriorEnergyUse->ExteriorLights(Item).Name);

                SetupOutputVariable(state,
                                    "Exterior Lights Electricity Energy",
                                    Constant::Units::J,
                                    state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::Invalid,
                                    OutputProcessor::EndUseCat::ExteriorLights,
                                    EndUseSubcategoryName);

                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchExLtPower,
                                 state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                 state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel);
                state.dataExteriorEnergyUse->sumDesignLevel += state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel;
                if (state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode == ExteriorEnergyUse::LightControlType::AstroClockOverride) {
                    PreDefTableEntry(state,
                                     state.dataOutRptPredefined->pdchExLtClock,
                                     state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                     "AstronomicalClock");
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchExLtSchd, state.dataExteriorEnergyUse->ExteriorLights(Item).Name, "-");
                } else {
                    PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchExLtClock, state.dataExteriorEnergyUse->ExteriorLights(Item).Name, "Schedule");
                    PreDefTableEntry(state,
                                     state.dataOutRptPredefined->pdchExLtSchd,
                                     state.dataExteriorEnergyUse->ExteriorLights(Item).Name,
                                     state.dataExteriorEnergyUse->ExteriorLights(Item).sched->Name);
                }
                ++Item;
>>>>>>> nrel/develop
            }
        }
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchExLtPower, "Exterior Lighting Total", state.dataExteriorEnergyUse->sumDesignLevel);

        // =================================  Get Exterior Fuel Equipment

        cCurrentModuleObject = "Exterior:FuelEquipment";
<<<<<<< HEAD
        for (int Item = 1; Item <= NumFuelEq; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Item,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataExteriorEnergyUse->UniqueExteriorEquipNames,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     cCurrentModuleObject,
                                                     state.dataIPShortCut->cAlphaFieldNames(1),
                                                     ErrorsFound);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            ++state.dataExteriorEnergyUse->NumExteriorEqs;

            auto &exteriorEquip = state.dataExteriorEnergyUse->ExteriorEquipment(state.dataExteriorEnergyUse->NumExteriorEqs);
            exteriorEquip.Name = state.dataIPShortCut->cAlphaArgs(1);

            if (NumAlphas > 3) {
                EndUseSubcategoryName = state.dataIPShortCut->cAlphaArgs(4);
            } else {
                EndUseSubcategoryName = "General";
            }

            if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
                ErrorsFound = true;

            } else if ((exteriorEquip.FuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, s_ipsc->cAlphaArgs(2)))) ==
                       Constant::eFuel::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;

            } else if (exteriorEquip.FuelType != Constant::eFuel::Water) {
                SetupOutputVariable(state,
                                    "Exterior Equipment Fuel Rate",
                                    Constant::Units::W,
                                    exteriorEquip.Power,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    exteriorEquip.Name);
                SetupOutputVariable(state,
                                    EnergyPlus::format("Exterior Equipment {} Energy", Constant::eFuelNames[(int)exteriorEquip.FuelType]),
                                    Constant::Units::J,
                                    exteriorEquip.CurrentUse,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    exteriorEquip.Name,
                                    Constant::eFuel2eResource[(int)exteriorEquip.FuelType],
                                    OutputProcessor::Group::Invalid,
                                    OutputProcessor::EndUseCat::ExteriorEquipment,
                                    EndUseSubcategoryName);
            } else {
=======
        auto const &exteriorFuelSchemaProps = inputProcessor->getObjectSchemaProps(state, cCurrentModuleObject);
        auto const exteriorFuelObjects = inputProcessor->epJSON.find(cCurrentModuleObject);
        if (exteriorFuelObjects != inputProcessor->epJSON.end()) {
            for (auto const &fuelEquipInstance : exteriorFuelObjects.value().items()) {
                auto const &fuelEquipFields = fuelEquipInstance.value();
                auto const equipName = Util::makeUPPER(fuelEquipInstance.key());
                auto const fuelUseType = inputProcessor->getAlphaFieldValue(fuelEquipFields, exteriorFuelSchemaProps, "fuel_use_type");
                auto const scheduleName = inputProcessor->getAlphaFieldValue(fuelEquipFields, exteriorFuelSchemaProps, "schedule_name");

                inputProcessor->markObjectAsUsed(cCurrentModuleObject, fuelEquipInstance.key());
                GlobalNames::VerifyUniqueInterObjectName(
                    state, state.dataExteriorEnergyUse->UniqueExteriorEquipNames, equipName, cCurrentModuleObject, "Name", ErrorsFound);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, equipName};

                ++state.dataExteriorEnergyUse->NumExteriorEqs;

                auto &exteriorEquip = state.dataExteriorEnergyUse->ExteriorEquipment(state.dataExteriorEnergyUse->NumExteriorEqs);
                exteriorEquip.Name = equipName;

                if (fuelEquipFields.find("end_use_subcategory") != fuelEquipFields.end()) {
                    EndUseSubcategoryName = inputProcessor->getAlphaFieldValue(fuelEquipFields, exteriorFuelSchemaProps, "end_use_subcategory");
                } else {
                    EndUseSubcategoryName = "General";
                }

                if (fuelUseType.empty()) {
                    ShowSevereEmptyField(state, eoh, "fuel_use_type");
                    ErrorsFound = true;
                } else if ((exteriorEquip.FuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, fuelUseType))) ==
                           Constant::eFuel::Invalid) {
                    ShowSevereInvalidKey(state, eoh, "fuel_use_type", fuelUseType);
                    ErrorsFound = true;
                } else if (exteriorEquip.FuelType != Constant::eFuel::Water) {
                    SetupOutputVariable(state,
                                        "Exterior Equipment Fuel Rate",
                                        Constant::Units::W,
                                        exteriorEquip.Power,
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        exteriorEquip.Name);
                    SetupOutputVariable(state,
                                        std::format("Exterior Equipment {} Energy", Constant::eFuelNames[(int)exteriorEquip.FuelType]),
                                        Constant::Units::J,
                                        exteriorEquip.CurrentUse,
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        exteriorEquip.Name,
                                        Constant::eFuel2eResource[(int)exteriorEquip.FuelType],
                                        OutputProcessor::Group::Invalid,
                                        OutputProcessor::EndUseCat::ExteriorEquipment,
                                        EndUseSubcategoryName);
                } else {
                    SetupOutputVariable(state,
                                        "Exterior Equipment Water Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        exteriorEquip.Power,
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        exteriorEquip.Name);
                    SetupOutputVariable(state,
                                        std::format("Exterior Equipment {} Volume", Constant::eFuelNames[(int)exteriorEquip.FuelType]),
                                        Constant::Units::m3,
                                        exteriorEquip.CurrentUse,
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        exteriorEquip.Name,
                                        Constant::eFuel2eResource[(int)exteriorEquip.FuelType],
                                        OutputProcessor::Group::Invalid,
                                        OutputProcessor::EndUseCat::ExteriorEquipment,
                                        EndUseSubcategoryName);
                }

                if (scheduleName.empty()) {
                    ShowSevereEmptyField(state, eoh, "schedule_name");
                    ErrorsFound = true;
                } else if ((exteriorEquip.sched = Sched::GetSchedule(state, scheduleName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "schedule_name", scheduleName);
                    ErrorsFound = true;
                } else if (int SchMin = exteriorEquip.sched->getMinVal(state); SchMin < 0.0) {
                    ShowSevereCustom(
                        state, eoh, std::format("{} = {} minimum is [{}]. Values must be >= 0.0.", "schedule_name", scheduleName, SchMin));
                    ErrorsFound = true;
                }
                exteriorEquip.DesignLevel = inputProcessor->getRealFieldValue(fuelEquipFields, exteriorFuelSchemaProps, "design_level");
            }
        }

        // =================================  Get Exterior Water Equipment

        cCurrentModuleObject = "Exterior:WaterEquipment";
        auto const &exteriorWaterSchemaProps = inputProcessor->getObjectSchemaProps(state, cCurrentModuleObject);
        auto const exteriorWaterObjects = inputProcessor->epJSON.find(cCurrentModuleObject);
        if (exteriorWaterObjects != inputProcessor->epJSON.end()) {
            for (auto const &waterEquipInstance : exteriorWaterObjects.value().items()) {
                auto const &waterEquipFields = waterEquipInstance.value();
                auto const equipName = Util::makeUPPER(waterEquipInstance.key());
                auto const scheduleName = inputProcessor->getAlphaFieldValue(waterEquipFields, exteriorWaterSchemaProps, "schedule_name");

                inputProcessor->markObjectAsUsed(cCurrentModuleObject, waterEquipInstance.key());

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, equipName};

                GlobalNames::VerifyUniqueInterObjectName(
                    state, state.dataExteriorEnergyUse->UniqueExteriorEquipNames, equipName, cCurrentModuleObject, "Name", ErrorsFound);

                ++state.dataExteriorEnergyUse->NumExteriorEqs;

                auto &exteriorEquip = state.dataExteriorEnergyUse->ExteriorEquipment(state.dataExteriorEnergyUse->NumExteriorEqs);
                exteriorEquip.Name = equipName;
                exteriorEquip.FuelType = Constant::eFuel::Water;

                if (scheduleName.empty()) {
                    ShowSevereEmptyField(state, eoh, "schedule_name");
                    ErrorsFound = true;
                } else if ((exteriorEquip.sched = Sched::GetSchedule(state, scheduleName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "schedule_name", scheduleName);
                    ErrorsFound = true;
                } else if (int SchMin = exteriorEquip.sched->getMinVal(state); SchMin < 0.0) {
                    ShowSevereCustom(
                        state, eoh, std::format("{} = {} minimum is [{}]. Values must be >= 0.0.", "schedule_name", scheduleName, SchMin));
                    ErrorsFound = true;
                }

                if (waterEquipFields.find("end_use_subcategory") != waterEquipFields.end()) {
                    EndUseSubcategoryName = inputProcessor->getAlphaFieldValue(waterEquipFields, exteriorWaterSchemaProps, "end_use_subcategory");
                } else {
                    EndUseSubcategoryName = "General";
                }

                exteriorEquip.DesignLevel = inputProcessor->getRealFieldValue(waterEquipFields, exteriorWaterSchemaProps, "design_level");

>>>>>>> nrel/develop
                SetupOutputVariable(state,
                                    "Exterior Equipment Water Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    exteriorEquip.Power,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    exteriorEquip.Name);
<<<<<<< HEAD
                SetupOutputVariable(state,
                                    EnergyPlus::format("Exterior Equipment {} Volume", Constant::eFuelNames[(int)exteriorEquip.FuelType]),
=======

                SetupOutputVariable(state,
                                    "Exterior Equipment Water Volume",
>>>>>>> nrel/develop
                                    Constant::Units::m3,
                                    exteriorEquip.CurrentUse,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    exteriorEquip.Name,
<<<<<<< HEAD
                                    Constant::eFuel2eResource[(int)exteriorEquip.FuelType],
=======
                                    Constant::eResource::Water,
                                    OutputProcessor::Group::Invalid,
                                    OutputProcessor::EndUseCat::ExteriorEquipment,
                                    EndUseSubcategoryName);
                SetupOutputVariable(state,
                                    "Exterior Equipment Mains Water Volume",
                                    Constant::Units::m3,
                                    exteriorEquip.CurrentUse,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    exteriorEquip.Name,
                                    Constant::eResource::MainsWater,
>>>>>>> nrel/develop
                                    OutputProcessor::Group::Invalid,
                                    OutputProcessor::EndUseCat::ExteriorEquipment,
                                    EndUseSubcategoryName);
            }
<<<<<<< HEAD

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                ErrorsFound = true;
            } else if ((exteriorEquip.sched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if (int SchMin = exteriorEquip.sched->getMinVal(state); SchMin < 0.0) {
                ShowSevereCustom(
                    state,
                    eoh,
                    EnergyPlus::format(
                        "{} = {} minimum is [{:.1R}]. Values must be >= 0.0.", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), SchMin));
                ErrorsFound = true;
            }
            exteriorEquip.DesignLevel = s_ipsc->rNumericArgs(1);
        }

        // =================================  Get Exterior Water Equipment

        cCurrentModuleObject = "Exterior:WaterEquipment";
        for (int Item = 1; Item <= NumWtrEq; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataExteriorEnergyUse->UniqueExteriorEquipNames,
                                                     s_ipsc->cAlphaArgs(1),
                                                     cCurrentModuleObject,
                                                     s_ipsc->cAlphaFieldNames(1),
                                                     ErrorsFound);

            ++state.dataExteriorEnergyUse->NumExteriorEqs;

            auto &exteriorEquip = state.dataExteriorEnergyUse->ExteriorEquipment(state.dataExteriorEnergyUse->NumExteriorEqs);
            exteriorEquip.Name = s_ipsc->cAlphaArgs(1);
            exteriorEquip.FuelType = Constant::eFuel::Water;

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                ErrorsFound = true;
            } else if ((exteriorEquip.sched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if (int SchMin = exteriorEquip.sched->getMinVal(state); SchMin < 0.0) {
                ShowSevereCustom(
                    state,
                    eoh,
                    EnergyPlus::format(
                        "{} = {} minimum is [{:.1R}]. Values must be >= 0.0.", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), SchMin));
                ErrorsFound = true;
            }

            if (NumAlphas > 3) {
                EndUseSubcategoryName = s_ipsc->cAlphaArgs(4);
            } else {
                EndUseSubcategoryName = "General";
            }

            exteriorEquip.DesignLevel = s_ipsc->rNumericArgs(1);

            SetupOutputVariable(state,
                                "Exterior Equipment Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                exteriorEquip.Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                exteriorEquip.Name);

            SetupOutputVariable(state,
                                "Exterior Equipment Water Volume",
                                Constant::Units::m3,
                                exteriorEquip.CurrentUse,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                exteriorEquip.Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::Invalid,
                                OutputProcessor::EndUseCat::ExteriorEquipment,
                                EndUseSubcategoryName);
            SetupOutputVariable(state,
                                "Exterior Equipment Mains Water Volume",
                                Constant::Units::m3,
                                exteriorEquip.CurrentUse,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                exteriorEquip.Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::Invalid,
                                OutputProcessor::EndUseCat::ExteriorEquipment,
                                EndUseSubcategoryName);
        }

        if (ErrorsFound) {
            ShowFatalError(state, EnergyPlus::format("{}Errors found in input.  Program terminates.", routineName));
=======
        }

        if (ErrorsFound) {
            ShowFatalError(state, std::format("{}Errors found in input.  Program terminates.", routineName));
>>>>>>> nrel/develop
        }
    } // GetExteriorEnergyUseInput()

    void ReportExteriorEnergyUse(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs the calculations necessary to report
        // the exterior energy use types.

        for (int Item = 1; Item <= state.dataExteriorEnergyUse->NumExteriorLights; ++Item) {
            switch (state.dataExteriorEnergyUse->ExteriorLights(Item).ControlMode) {
            case ExteriorEnergyUse::LightControlType::ScheduleOnly:
                state.dataExteriorEnergyUse->ExteriorLights(Item).Power = state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel *
                                                                          state.dataExteriorEnergyUse->ExteriorLights(Item).sched->getCurrentVal();
                state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse =
                    state.dataExteriorEnergyUse->ExteriorLights(Item).Power * state.dataGlobal->TimeStepZoneSec;
                break;
            case ExteriorEnergyUse::LightControlType::AstroClockOverride:
                if (state.dataEnvrn->SunIsUp) {
                    state.dataExteriorEnergyUse->ExteriorLights(Item).Power = 0.0;
                    state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse = 0.0;
                } else {
                    state.dataExteriorEnergyUse->ExteriorLights(Item).Power =
                        state.dataExteriorEnergyUse->ExteriorLights(Item).DesignLevel *
                        state.dataExteriorEnergyUse->ExteriorLights(Item).sched->getCurrentVal();
                    state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse =
                        state.dataExteriorEnergyUse->ExteriorLights(Item).Power * state.dataGlobal->TimeStepZoneSec;
                }
                break;
            default:
                // should not happen
                break;
            }

            // Reduce lighting power due to demand limiting
            if (state.dataExteriorEnergyUse->ExteriorLights(Item).ManageDemand &&
                (state.dataExteriorEnergyUse->ExteriorLights(Item).Power > state.dataExteriorEnergyUse->ExteriorLights(Item).DemandLimit)) {
                state.dataExteriorEnergyUse->ExteriorLights(Item).Power = state.dataExteriorEnergyUse->ExteriorLights(Item).DemandLimit;
                state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse =
                    state.dataExteriorEnergyUse->ExteriorLights(Item).Power * state.dataGlobal->TimeStepZoneSec;
            }
            // EMS controls
            if (state.dataExteriorEnergyUse->ExteriorLights(Item).PowerActuatorOn) {
                state.dataExteriorEnergyUse->ExteriorLights(Item).Power = state.dataExteriorEnergyUse->ExteriorLights(Item).PowerActuatorValue;
            }

            state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse =
                state.dataExteriorEnergyUse->ExteriorLights(Item).Power * state.dataGlobal->TimeStepZoneSec;

            // gather for tabular reports
            if (!state.dataGlobal->WarmupFlag) {
                //      IF (DoOutputReporting .AND.  WriteTabularFiles .and. (KindOfSim == ksRunPeriodWeather)) THEN !for weather simulations only
                if (state.dataGlobal->DoOutputReporting &&
                    (state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather)) { // for weather simulations only
                    // for tabular report, accumua the total electricity used for each ExteriorLights object
                    state.dataExteriorEnergyUse->ExteriorLights(Item).SumConsumption += state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse;
                    // for tabular report, accumulate the time when each ExteriorLights has consumption
                    //(using a very small threshold instead of zero)
                    if (state.dataExteriorEnergyUse->ExteriorLights(Item).CurrentUse > 0.01) {
                        state.dataExteriorEnergyUse->ExteriorLights(Item).SumTimeNotZeroCons += state.dataGlobal->TimeStepZone;
                    }
                }
            }
        }

        for (int Item = 1; Item <= state.dataExteriorEnergyUse->NumExteriorEqs; ++Item) {
            state.dataExteriorEnergyUse->ExteriorEquipment(Item).Power = state.dataExteriorEnergyUse->ExteriorEquipment(Item).DesignLevel *
                                                                         state.dataExteriorEnergyUse->ExteriorEquipment(Item).sched->getCurrentVal();
            state.dataExteriorEnergyUse->ExteriorEquipment(Item).CurrentUse =
                state.dataExteriorEnergyUse->ExteriorEquipment(Item).Power * state.dataGlobal->TimeStepZoneSec;
        }
    }

} // namespace ExteriorEnergyUse

} // namespace EnergyPlus
