#ifndef GradientsGTM_hh
#define GradientsGTM_hh

// --- 1. Define Real64 FIRST by including the core EnergyPlus header ---
#include "EnergyPlus/EnergyPlus.hh"

// --- 2. Standard library includes ---
#include <vector>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

// --- 3. EnergyPlus specific includes ---
#include "EnergyPlus/Data/EnergyPlusData.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh"

namespace EnergyPlus {
namespace GroundTemp {

class GradientsGTM : public BaseGroundTempsModel {
public:
    struct GradientSegment {
        Real64 upperDepth;
        Real64 lowerDepth;
        Real64 gradient;
    };

    GradientsGTM() = default;
    static std::unique_ptr<BaseGroundTempsModel> factory(EnergyPlusData &state, std::string const &objectName);

    // Initialize from IDF segments and borehole depth
    void initialize(EnergyPlusData &state, const std::vector<GradientSegment> &idfSegments, Real64 H);

    // Set borehole depth and update internal reference temperature from Kusuda if present
    void setBoreholeDepth(EnergyPlusData &state, Real64 H);

    // Overrides from BaseGroundTempsModel
    Real64 getGroundTemp(EnergyPlusData &state) override;
    Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds) override;
    Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month) override;

private:
    void parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object);
    Real64 getHybridFarfieldTemp(EnergyPlusData &state, int month, Real64 depth) const;

    // Member variables
    std::vector<GradientSegment> idfSegments_;
    std::vector<GradientSegment> segments_;
    Real64 boreholeDepth{0.0};
    Real64 referenceTemp{0.0};      // average Kusuda-based reference (if Kusuda model found)
    Real64 transitionDepth{15.0};   // default transition depth (m)
    Real64 blendWidth{5.0};         // default blend width (m)
};

} // namespace GroundTemp
} // namespace EnergyPlus

#endif