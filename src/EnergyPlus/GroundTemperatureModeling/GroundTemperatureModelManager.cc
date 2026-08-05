// GroundTemperatureModelManager.cc
#include "GroundTemperatureModelManager.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh>
#include <EnergyPlus/GroundTemperatureModeling/XingGroundTemperatureModel.hh>
#include <EnergyPlus/GroundTemperatureModeling/GradientsGTM.hh>

namespace EnergyPlus {
namespace GroundTemp {

std::unique_ptr<BaseGroundTempsModel> GetGroundTempModelAndInit(
    EnergyPlusData &state,
    ModelType modelType,
    std::string const &objectName)
{
    switch (modelType) {
        case ModelType::KusudaAchenbach:
            return KusudaAchenbachGroundTemperatureModel::factory(state, objectName);
        case ModelType::Xing:
            return XingGroundTemperatureModel::factory(state, objectName);
        case ModelType::FiniteDifference:
            // If you have a finite difference factory, add it here
            // return FiniteDifferenceGroundTemperatureModel::factory(state, objectName);
            // Otherwise fall through
        case ModelType::GradientSegments:   // <-- your new type
            return GradientsGTM::factory(state, objectName);
        default:
            ShowFatalError(state, fmt::format("Invalid ground temperature model type: {}", static_cast<int>(modelType)));
            return nullptr;
    }
}

} // namespace GroundTemp
} // namespace EnergyPlus