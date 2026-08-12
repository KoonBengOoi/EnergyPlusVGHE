x#ifndef GradientsGTM_hh
#define GradientsGTM_hh

#include <vector>
#include <memory>
#include <string>
#include "EnergyPlus/Data/EnergyPlusData.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTempsModel.hh"

namespace EnergyPlus {
namespace GroundTemperatureModeling {

class GradientsGTM : public BaseGroundTempsModel {
public:
    // ... keep your existing class definition ...
    static std::unique_ptr<BaseGroundTempsModel> factory(EnergyPlusData &state, std::string const &objectName);
    
    // Add this so the manager can find you
    void setBoreholeDepth(Real64 H);

private:
    Real64 computeKusudaTemp(int month, Real64 depth) const;
    // ... keep your existing variables ...
};

} // namespace GroundTemperatureModeling
} // namespace EnergyPlus

#endif