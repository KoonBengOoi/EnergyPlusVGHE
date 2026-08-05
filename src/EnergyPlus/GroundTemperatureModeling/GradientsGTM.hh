#pragma once

#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <vector>
#include <string>
#include <memory>

namespace EnergyPlus {

struct GradientSegment {
    Real64 upperDepth; // upper depth [m]
    Real64 lowerDepth; // lower depth [m]
    Real64 gradient;   // K/m
};

class GradientsGTM : public BaseGroundTempModel
{
public:
    GradientsGTM() = default;
    ~GradientsGTM() override = default;

    // Factory method
    static std::unique_ptr<BaseGroundTempModel>
    GradientsGTMFactory(EnergyPlusData &state,
                       GroundTemperatureManager::GroundTempObjType objectType,
                       std::string const &objectName);

    // Input parsing
    void getInputs(EnergyPlusData &state, std::string const &objectName) override;

    // Temperature calculation
    Real64 getGroundTemperature(EnergyPlusData &state, Real64 depth) override;

    // Initialization with borehole depth
    void initialize(const std::vector<GradientSegment> &idfSegments, Real64 H);

    // Hybrid far-field routine (month-specific)
    Real64 getHybridFarfieldTemp(int month, Real64 depth);

private:
    Real64 referenceTemp{0.0};
    Real64 boreholeDepth{200.0}; // H from ResponseFactors
    std::vector<GradientSegment> segments;
};

} // namespace EnergyPlus