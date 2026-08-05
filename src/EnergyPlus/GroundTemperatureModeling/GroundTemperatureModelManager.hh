// GroundTemperatureModelManager.hh
#pragma once

#include <memory>
#include <string>

namespace EnergyPlus {
class EnergyPlusData;
namespace GroundTemp {
class BaseGroundTempsModel;
enum class ModelType;

std::unique_ptr<BaseGroundTempsModel> GetGroundTempModelAndInit(
    EnergyPlusData &state,
    ModelType modelType,
    std::string const &objectName);

} // namespace GroundTemp
} // namespace EnergyPlus