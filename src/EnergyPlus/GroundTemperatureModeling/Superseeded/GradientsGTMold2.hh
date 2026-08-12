// GradientsGTM.hh
#pragma once

#include <nlohmann/json.hpp>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <vector>
#include <memory>

namespace EnergyPlus {
namespace GroundTemp {

struct GradientSegment {
    Real64 upperDepth;   //!< Start depth of segment [m]
    Real64 lowerDepth;   //!< End depth of segment [m]
    Real64 gradient;     //!< Temperature gradient [K/m]
};

class GradientsGTM : public BaseGroundTempsModel
{
public:
    GradientsGTM() = default;
    ~GradientsGTM() override = default;

    // ─── Factory method ───
    static std::unique_ptr<BaseGroundTempsModel> factory(
        EnergyPlusData &state,
        std::string const &objectName);

    // ─── Override base class methods ───
    Real64 getGroundTemp(EnergyPlusData &state) override;
    Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds) override;
    Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month) override;

    // ─── Set borehole depth (called from Vertical.cc) ───
    void setBoreholeDepth(Real64 H);

private:
    // ─── Private helper methods ───
    void initialize(const std::vector<GradientSegment> &idfSegments, Real64 H);
    Real64 getHybridFarfieldTemp(int month, Real64 depth) const;
    void parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object);

    // ─── Member variables ───
    Real64 boreholeDepth = 200.0;      // Will be set from GHE object
    Real64 transitionDepth = 15.0;     // Depth above which KA is used (m)
    Real64 blendWidth = 5.0;           // Width of smooth transition (m)
    Real64 referenceTemp = 15.0;       // Base temperature at surface (annual avg)
    std::vector<GradientSegment> segments;
};

} // namespace GroundTemp
} // namespace EnergyPlus