#include "GradientsGTM.hh"

#include <algorithm>
#include <cmath>

#include "EnergyPlus/DataEnvironment.hh"
#include "EnergyPlus/InputProcessing/InputProcessor.hh"
#include "EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh"
#include "EnergyPlus/UtilityRoutines.hh"

namespace EnergyPlus {
namespace GroundTemp {

std::unique_ptr<BaseGroundTempsModel> GradientsGTM::factory(
    EnergyPlusData &state,
    std::string const &objectName)
{
    auto model = std::make_unique<GradientsGTM>();

    // Look for IDF object and parse segments if present
    auto const &epJSON = state.dataInputProcessing->inputProcessor->epJSON;
    auto it = epJSON.find("Site:GroundTemperature:Undisturbed:GradientSegments");
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

void GradientsGTM::parseGradientSegments(EnergyPlusData & /*state*/, nlohmann::json const &object)
{
    idfSegments_.clear();
    if (object.find("gradient_segments") != object.end()) {
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

void GradientsGTM::initialize(EnergyPlusData &state, const std::vector<GradientSegment> &idfSegments, Real64 H)
{
    // copy IDF segments and sort
    idfSegments_ = idfSegments;
    segments_.clear();
    for (const auto &s : idfSegments_) {
        segments_.push_back(s);
    }
    std::sort(segments_.begin(), segments_.end(), [](const GradientSegment &a, const GradientSegment &b) {
        return a.upperDepth < b.upperDepth;
    });

    // set borehole depth and compute reference temp (uses Kusuda average if present)
    setBoreholeDepth(state, H);
}

void GradientsGTM::setBoreholeDepth(EnergyPlusData &state, Real64 H)
{
    boreholeDepth = H;

    // Compute average Kusuda temp at ground surface (depth = 0) across 12 months, if Kusuda model exists.
    Real64 sumT = 0.0;
    int kusudaCount = 0;
    if (state.dataGrndTempModelMgr && state.dataGrndTempModelMgr->groundTempModels.size() > 0) {
        for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
            if (m) {
                for (int month = 1; month <= 12; ++month) {
                    sumT += m->getGroundTempAtTimeInMonths(state, 0.0, month);
                }
                kusudaCount = 12;
                break;
            }
        }
    }

    if (kusudaCount > 0) {
        referenceTemp = sumT / static_cast<Real64>(kusudaCount);
    } else {
        if (state.dataEnvrn) {
            referenceTemp = state.dataEnvrn->OutDryBulbTemp;
        } else {
            referenceTemp = 10.0;
        }
    }
}

Real64 GradientsGTM::getHybridFarfieldTemp(EnergyPlusData &state, int month, Real64 depth) const
{
    // 1) Get Kusuda-based temperature for this month/depth (fallback to referenceTemp)
    Real64 kaTemp = referenceTemp;
    if (state.dataGrndTempModelMgr && state.dataGrndTempModelMgr->groundTempModels.size() > 0) {
        for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
            if (m) {
                kaTemp = m->getGroundTempAtTimeInMonths(state, depth, month);
                break;
            }
        }
    }

    // 2) Compute gradient-based temperature from user segments (relative to referenceTemp)
    Real64 gradTemp = referenceTemp;
    if (!segments_.empty()) {
        bool found = false;
        for (const auto &seg : segments_) {
            if (depth >= seg.upperDepth && depth <= seg.lowerDepth) {
                gradTemp = referenceTemp + seg.gradient * (depth - seg.upperDepth);
                found = true;
                break;
            }
        }
        if (!found) {
            if (depth > segments_.back().lowerDepth) {
                const auto &last = segments_.back();
                gradTemp = referenceTemp + last.gradient * (depth - last.upperDepth);
            } else if (depth < segments_.front().upperDepth) {
                const auto &first = segments_.front();
                gradTemp = referenceTemp + first.gradient * (depth - first.upperDepth);
            }
        }
    }

    // 3) Blend between Kusuda (near-surface) and gradient (deep) using transitionDepth/blendWidth
    Real64 low = transitionDepth - blendWidth / 2.0;
    Real64 high = transitionDepth + blendWidth / 2.0;
    Real64 weight = 0.0;
    if (depth <= low) {
        weight = 0.0;
    } else if (depth >= high) {
        weight = 1.0;
    } else {
        weight = (depth - low) / (high - low);
        weight = std::clamp(weight, 0.0, 1.0);
    }

    return (1.0 - weight) * kaTemp + weight * gradTemp;
}

Real64 GradientsGTM::getGroundTemp(EnergyPlusData &state)
{
    int month = 1;
    if (state.dataEnvrn) month = state.dataEnvrn->Month;
    Real64 depth = (boreholeDepth > 0.0) ? (boreholeDepth / 2.0) : 0.0;
    return getHybridFarfieldTemp(state, month, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 /* timeInSeconds */)
{
    int month = 1;
    if (state.dataEnvrn) month = state.dataEnvrn->Month;
    return getHybridFarfieldTemp(state, month, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month)
{
    return getHybridFarfieldTemp(state, month, depth);
}

} // namespace GroundTemp
} // namespace EnergyPlus
