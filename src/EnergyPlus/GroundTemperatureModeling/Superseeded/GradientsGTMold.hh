#pragma once

#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <vector>
#include <string>
#include <memory>

namespace EnergyPlus {
namespace GroundTemp {
    
}
struct GradientSegment {
    Real64 upperDepth; // upper depth [m]
    Real64 lowerDepth; // lower depth [m]
    Real64 gradient;   // K/m
};
class GradientsGTM : public BaseGroundTempsModel
{
public:
    GradientsGTM() = default;
    ~GradientsGTM() override = default;

    // Factory method – takes object name, NOT GroundTempObjType
    static std::unique_ptr<BaseGroundTempsModel> factory(
        EnergyPlusData &state,
        std::string const &objectName);

    // Override base class methods
    Real64 getGroundTemp(EnergyPlusData &state) override;
    Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds) override;
    Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month) override;

    void setBoreholeDepth(Real64 depth) { boreholeDepth = depth; }
private:
// private member method
// Initialization with borehole depth
void initialize(const std::vector<GradientSegment> &idfSegments, Real64 H);
    // Hybrid far-field routine (month-specific)
    Real64 getHybridFarfieldTemp(int month, Real64 depth);
// member variables
    Real64 boreholeDepth = 200.0;
    Real64 transitionDepth = 15.0;
    Real64 blendWidth = 5.0;
    Real64 referenceTemp = 15.0;
    std::vector<GradientSegment> segments;

    // Parsing method (called by factory)
    void parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object);
};
} // namespace GroundTemp
} // namespace EnergyPlus