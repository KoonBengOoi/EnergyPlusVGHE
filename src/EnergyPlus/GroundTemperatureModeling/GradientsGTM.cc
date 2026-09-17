#include "GradientsGTM.hh"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include <fmt/format.h>

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {
namespace GroundTemp {

std::unique_ptr<BaseGroundTempsModel> GradientsGTM::factory(
    EnergyPlusData &state,
    std::string const &objectName)
{
    auto model = std::make_unique<GradientsGTM>();

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

    if (object.find("transition_depth") != object.end()) {
        transitionDepth = object["transition_depth"].get<Real64>();
    }
    if (object.find("blend_width") != object.end()) {
        blendWidth = object["blend_width"].get<Real64>();
    }

    for (int i = 1; i <= 20; ++i) {
        std::string upperKey = "upper_depth_" + std::to_string(i);
        std::string lowerKey = "lower_depth_" + std::to_string(i);
        std::string gradKey  = "gradient_" + std::to_string(i);

        if (object.find(upperKey) == object.end()) break;

        GradientSegment gs;
        gs.upperDepth = object[upperKey].get<Real64>();
        gs.lowerDepth = object[lowerKey].get<Real64>();
        gs.gradient   = object[gradKey].get<Real64>();
        idfSegments_.push_back(gs);
    }
}

void GradientsGTM::initialize(EnergyPlusData & /*state*/,
                              const std::vector<GradientSegment> &idfSegments,
                              Real64 H)
{
    segments_.clear();
    for (const auto &s : idfSegments) {
        segments_.push_back(s);
    }
    std::sort(segments_.begin(), segments_.end(),
              [](const GradientSegment &a, const GradientSegment &b) {
                  return a.upperDepth < b.upperDepth;
              });
    boreholeDepth = H;
}

void GradientsGTM::setBoreholeDepth(EnergyPlusData &state, Real64 H)
{
    boreholeDepth = H;

    segments_.clear();
    for (const auto &s : idfSegments_) {
        segments_.push_back(s);
    }
    std::sort(segments_.begin(), segments_.end(),
              [](const GradientSegment &a, const GradientSegment &b) {
                  return a.upperDepth < b.upperDepth;
              });

    Real64 sumT = 0.0;
    int kusudaCount = 0;
    if (state.dataGrndTempModelMgr) {
        for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
            if (m && m != this && m->modelType == ModelType::Kusuda) {
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
        referenceTemp = 15.0;
    }
}

Real64 GradientsGTM::segmentTemperatureAt(Real64 depth) const
{
    if (segments_.empty()) {
        return referenceTemp;
    }

    Real64 T = referenceTemp;
    Real64 zRef = segments_.front().upperDepth;

    if (depth < zRef) {
        return referenceTemp + segments_.front().gradient * (depth - zRef);
    }

    for (const auto &seg : segments_) {
        if (depth <= seg.upperDepth) break;
        Real64 zTop    = seg.upperDepth;
        Real64 zBottom = seg.lowerDepth;
        Real64 zEff    = std::min(depth, zBottom);
        if (zEff > zTop) {
            T += seg.gradient * (zEff - zTop);
        }
        if (depth <= zBottom) {
            return T;
        }
    }

    const auto &last = segments_.back();
    T += last.gradient * (depth - last.lowerDepth);
    return T;
}

Real64 GradientsGTM::blendWeight(Real64 depth) const
{
    Real64 low  = transitionDepth - blendWidth / 2.0;
    Real64 high = transitionDepth + blendWidth / 2.0;
    if (depth <= low)  return 0.0;
    if (depth >= high) return 1.0;
    Real64 w = (depth - low) / (high - low);
    return std::clamp(w, 0.0, 1.0);
}

Real64 GradientsGTM::getHybridFarfieldTempAtTime(EnergyPlusData &state,
                                                 Real64 timeInSeconds,
                                                 Real64 depth) const
{
    Real64 kaTemp = referenceTemp;
    if (state.dataGrndTempModelMgr) {
        for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
            if (m && m != this && m->modelType == ModelType::Kusuda) {
                kaTemp = m->getGroundTempAtTimeInSeconds(state, depth, timeInSeconds);
                break;
            }
        }
    }

    Real64 gradTemp = segmentTemperatureAt(depth);
    Real64 w = blendWeight(depth);
    return (1.0 - w) * kaTemp + w * gradTemp;
}

Real64 GradientsGTM::getHybridFarfieldTemp(EnergyPlusData &state,
                                           int month,
                                           Real64 depth) const
{
    Real64 kaTemp = referenceTemp;
    if (state.dataGrndTempModelMgr) {
        for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
            if (m && m != this && m->modelType == ModelType::Kusuda) {
                kaTemp = m->getGroundTempAtTimeInMonths(state, depth, month);
                break;
            }
        }
    }

    Real64 gradTemp = segmentTemperatureAt(depth);
    Real64 w = blendWeight(depth);
    return (1.0 - w) * kaTemp + w * gradTemp;
}

Real64 GradientsGTM::getGroundTemp(EnergyPlusData &state)
{
    Real64 depth = (boreholeDepth > 0.0) ? (boreholeDepth / 2.0) : 0.0;
    int month = 1;
    if (state.dataEnvrn) month = state.dataEnvrn->Month;
    return getHybridFarfieldTemp(state, month, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInSeconds(EnergyPlusData &state,
                                                  Real64 depth,
                                                  Real64 timeInSeconds)
{
    return getHybridFarfieldTempAtTime(state, timeInSeconds, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInMonths(EnergyPlusData &state,
                                                 Real64 depth,
                                                 int month)
{
    return getHybridFarfieldTemp(state, month, depth);
}

void GradientsGTM::setSurfaceTemperature(Real64 T_surface)
{
    referenceTemp = T_surface;
}

void GradientsGTM::setVerticalGradient(Real64 gradient)
{
    uniformGradient = gradient;
}

void GradientsGTM::initializeSegments(EnergyPlusData &state)
{
    if (uniformGradient != 0.0) {
        idfSegments_.clear();
        GradientSegment seg;
        seg.upperDepth = 0.0;
        seg.lowerDepth = 200.0;
        seg.gradient = uniformGradient;
        idfSegments_.push_back(seg);
    }
    if (boreholeDepth > 0.0) {
        initialize(state, idfSegments_, boreholeDepth);
    }
}

} // namespace GroundTemp
} // namespace EnergyPlus