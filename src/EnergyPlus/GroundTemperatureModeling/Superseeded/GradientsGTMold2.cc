// GradientsGTM.cc
#include <nlohmann/json.hpp>
#include "GradientsGTM.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include "EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh"
#include <EnergyPlus/UtilityRoutines.hh>
#include <algorithm>
#include <cmath>
#define KUSUDA_GROUND_TEMP_MODEL

namespace EnergyPlus {
namespace GroundTemp {

// ──────────────────────────────────────────────────────────────
// Factory method – called by GroundTemperatureModelManager
// ──────────────────────────────────────────────────────────────
std::unique_ptr<BaseGroundTempsModel> GradientsGTM::factory(
    EnergyPlusData &state,
    std::string const &objectName)
{
    auto model = std::make_unique<GradientsGTM>();

    auto const &epJSON = state.dataInputProcessing->inputProcessor->epJSON;

    auto const it = epJSON.find("Site:GroundTemperature:Undisturbed:GradientSegments");
    if (it != epJSON.end()) {
        for (auto const &item : it.value().items()) {
            if (Util::makeUPPER(item.key()) == Util::makeUPPER(objectName)) {
                model->parseGradientSegments(state, item.value());
                break;
            }
        }
    }

    return model;
}

// ──────────────────────────────────────────────────────────────
// parseGradientSegments – reads the IDF object
// ──────────────────────────────────────────────────────────────
void GradientsGTM::parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object)
{
    if (object.find("gradient_segments") != object.end()) {
        std::vector<GradientSegment> idfSegments;

        for (auto const &seg : object["gradient_segments"]) {
            GradientSegment gs;
            gs.upperDepth = seg.at("z_start").get<Real64>();
            gs.lowerDepth = seg.at("z_end").get<Real64>();
            gs.gradient = seg.at("gradient").get<Real64>();
            idfSegments.push_back(gs);
        }

        // H will be set later via setBoreholeDepth() from the GHE system
        Real64 H = 200.0;  // default, will be updated
        initialize(idfSegments, H);
    }

    if (object.find("transition_depth") != object.end()) {
        transitionDepth = object["transition_depth"].get<Real64>();
    }
    if (object.find("blend_width") != object.end()) {
        blendWidth = object["blend_width"].get<Real64>();
    }
}

// ──────────────────────────────────────────────────────────────
// initialize – stores segments and computes reference temperature
// ──────────────────────────────────────────────────────────────
void GradientsGTM::initialize(const std::vector<GradientSegment> &idfSegments, Real64 H)
{
    boreholeDepth = H;
    segments.clear();
    for (const auto &s : idfSegments) {
        segments.push_back({s.upperDepth, s.lowerDepth, s.gradient});
    }

    // Compute reference temperature as Kusuda-Achenbach at surface (depth 0), annual average
    Real64 sumT = 0.0;
    for (int month = 1; month <= 12; ++month) {
       // sumT += GroundTemp::KusudaGroundTemperatureModel::compute(month, 0.0);
    Real64 kaTemp = BaseGroundTempsModel::getKusudaTemperature(month, depth);
    }
    referenceTemp = sumT / 12.0;
}

// ──────────────────────────────────────────────────────────────
// setBoreholeDepth – called from the GHE system (e.g., Vertical.cc)
// ──────────────────────────────────────────────────────────────
void GradientsGTM::setBoreholeDepth(Real64 H)
{
    boreholeDepth = H;
}

// ──────────────────────────────────────────────────────────────
// getHybridFarfieldTemp – core weighted average calculation
// ──────────────────────────────────────────────────────────────
Real64 GradientsGTM::getHybridFarfieldTemp(int month, Real64 depth) const
{
    // ── 1. Kusuda-Achenbach temperature (far-field, time-varying) ──
    Real64 kaTemp = GroundTemp::KusudaGroundTemperatureModel::compute(month, depth);

    // ── 2. Gradient-based temperature (user-defined segments) ──
    Real64 gradTemp = referenceTemp;  // fallback
    for (const auto &seg : segments) {
        if (depth >= seg.upperDepth && depth <= seg.lowerDepth) {
            gradTemp = referenceTemp + seg.gradient * (depth - seg.upperDepth);
            break;
        }
    }
    // If depth is below the last segment, extrapolate from the last segment
    if (!segments.empty() && depth > segments.back().lowerDepth) {
        const auto &last = segments.back();
        gradTemp = referenceTemp + last.gradient * (depth - last.upperDepth);
    }

    // ── 3. Smooth blending weight ──
    //   - depth <= transitionDepth - blendWidth/2  → weight = 0 (pure KA)
    //   - depth >= transitionDepth + blendWidth/2  → weight = 1 (pure gradients)
    //   - in between, linear blend
    Real64 weight = 0.0;
    Real64 low = transitionDepth - blendWidth / 2.0;
    Real64 high = transitionDepth + blendWidth / 2.0;

    if (depth <= low) {
        weight = 0.0;                    // Pure Kusuda-Achenbach
    } else if (depth >= high) {
        weight = 1.0;                    // Pure gradient segments
    } else {
        weight = (depth - low) / (high - low);  // Linear transition
    }

    // ── 4. Return weighted average ──
    return (1.0 - weight) * kaTemp + weight * gradTemp;
}

// ──────────────────────────────────────────────────────────────
// Override methods required by BaseGroundTempsModel
// ──────────────────────────────────────────────────────────────

Real64 GradientsGTM::getGroundTemp(EnergyPlusData &state)
{
    // Return average over the borehole depth (simplified)
    return getHybridFarfieldTemp(1, boreholeDepth / 2.0);
}

Real64 GradientsGTM::getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds)
{
    // Convert time to month (1–12)
    int month = 1 + static_cast<int>(std::fmod(timeInSeconds / (86400.0 * 30.44), 12.0));
    if (month < 1) month = 1;
    if (month > 12) month = 12;
    return getHybridFarfieldTemp(month, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month)
{
    return getHybridFarfieldTemp(month, depth);
}

} // namespace GroundTemp
} // namespace EnergyPlus