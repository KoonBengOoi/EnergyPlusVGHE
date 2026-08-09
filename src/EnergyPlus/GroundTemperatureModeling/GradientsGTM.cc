// GradientsGTM.cc

#include <nlohmann/json.hpp>
#include "GradientsGTM.hh"
#include "EnergyPlus/Data/EnergyPlusData.hh"
#include "EnergyPlus/DataEnvironment.hh"
#include "EnergyPlus/InputProcessing/InputProcessor.hh"
#include "EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh"
#include "EnergyPlus/UtilityRoutines.hh"
#include <algorithm>
#include <cmath>

namespace EnergyPlus {
namespace GroundTemp {

// ──────────────────────────────────────────────────────────────
// Factory method
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
// parseGradientSegments
// ──────────────────────────────────────────────────────────────
void GradientsGTM::parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object)
{
    if (object.find("gradient_segments") != object.end()) {
        idfSegments_.clear();

        for (auto const &seg : object["gradient_segments"]) {
            GradientSegment gs;
            gs.upperDepth = seg.at("z_start").get<Real64>();
            gs.lowerDepth = seg.at("z_end").get<Real64>();
            gs.gradient = seg.at("gradient").get<Real64>();
            idfSegments_.push_back(gs);
        }
    }

    if (object.find("transition_depth") != object.end()) {
        transitionDepth = object["transition_depth"].get<Real64>();
    }
    if (object.find("blend_width") != object.end()) {
        blendWidth = object["blend_width"].get<Real64>();
    }
}
// ──────────────────────────────────────────────────────────────
// initialize
// ──────────────────────────────────────────────────────────────
void GradientsGTM::initialize(EnergyPlusData &state, const std::vector<GradientSegment> &idfSegments, Real64 H)
{
    boreholeDepth = H;
    segments.clear();
    for (const auto &s : idfSegments) {
        segments.push_back({s.upperDepth, s.lowerDepth, s.gradient});
    }

    // Sort segments by upperDepth
    std::sort(segments.begin(), segments.end(), [](const GradientSegment& a, const GradientSegment& b) {
        return a.upperDepth < b.upperDepth;
    });

    // ── 1. Kusuda temperature (time-varying far field) valid only for 0<H<15m,  
    void GradientsGTM::setBoreholeDepth(Real64 H)
{
        boreholeDepth = H; // check name of the variable, it should be consistent with the class member, also in Vertical.cc/.hh
    segments.clear();
    for (const auto &s : idfSegments) {
        segments.push_back({s.upperDepth, s.lowerDepth, s.gradient});
    }

    std::sort(segments.begin(), segments.end(), [](const GradientSegment& a, const GradientSegment& b) {
        return a.upperDepth < b.upperDepth;
    });

    // --- ADD THE KUSUDA LOOP HERE (state is available because setBoreholeDepth takes it) ---
    Real64 sumT = 0.0;
    for (int month = 1; month <= 12; ++month) {
        for (const auto &m : state.dataGroundTempMgr->groundTempModels) {
            if (m->modelType == GroundTemp::ModelType::Kusuda) {
                sumT += m->getGroundTempAtTimeInMonths(state, 0.0, month);
                break;
            }
        }
    }
    referenceTemp = sumT / 12.0;
}
Real64 kaTemp = 0.0;
    for (const auto &m : state.dataGroundTempMgr->groundTempModels) {
        if (m->modelType == GroundTemp::ModelType::Kusuda) {
            kaTemp = m->getGroundTempAtTimeInMonths(state, depth, month);
            break;
        }
    }
    
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
// Override methods required by BaseGroundTemperatureModel
// ──────────────────────────────────────────────────────────────

Real64 GradientsGTM::getGroundTemp(EnergyPlusData &state)
{
    return getHybridFarfieldTemp(Month, boreholeDepth / 2.0);
}

Real64 GradientsGTM::getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 /* timeInSeconds */)
{
    int month = state.dataEnvrn->Month; 
    return getHybridFarfieldTemp(seconds, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month)
{
    return getHybridFarfieldTemp(month, depth);
}
} // namespace GroundTemp
} // namespace EnergyPlus
