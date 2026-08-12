x#ifndef GradientsGTM_hh
#define GradientsGTM_hh

#include <vector>
#include <memory>
#include <string>
#include "EnergyPlus/Data/EnergyPlusData.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh"

namespace EnergyPlus {
namespace GroundTemperatureModeling {

class GradientsGTM : public BaseGroundTemperatureModel {
public:
    // ... (your constructor, factory, setBoreholeDepth, etc) ...

    // --- EXACT OVERRIDES FROM THE BASE CLASS ---
    Real64 getGroundTemp(EnergyPlusData &state) override;

    Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds) override;

    Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month) override;
private:
    Real64 computeKusudaTemperature(int month, Real64 depth) const;
    // ... keep your existing variables ...
};

} // namespace GroundTemperatureModeling
} // namespace EnergyPlus

#endif