#include "GradientsGTM.hh"

#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>

// If the Kusuda/Achenbach model is in a separate header, include it.
// Adjust the path if your tree uses a different header name/location.
#include <EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh>

#include <algorithm>

namespace EnergyPlus {

// initialize: simple copy of segments and store borehole depth
void GradientsGTM::initialize(const std::vector<GradientSegment> &idfSegments,
                              Real64 H)
{
    boreholeDepth = H;
    segments.clear();
    for (const auto &s : idfSegments) {
        segments.push_back({s.upperDepth, s.lowerDepth, s.gradient});
    }
}

// Factory (unchanged behavior, kept here for completeness)
std::unique_ptr<BaseGroundTempModel>
GradientsGTM::GradientsGTMFactory(EnergyPlusData &state,
                                  GroundTemperatureManager::GroundTempObjType /*objectType*/,
                                  std::string const & /*objectName*/)
{
    auto model = std::make_unique<GradientsGTM>();

    // Step 1: Get H from ResponseFactors (fallback 200.0)
    Real64 H = 200.0;
    int numRF = state.dataInputProcessing->inputProcessor->getNumObjectsFound("GroundHeatExchanger:Vertical:ResponseFactors");
    if (numRF > 0) {
        auto rfFields = state.dataInputProcessing->inputProcessor->getObjectItem("GroundHeatExchanger:Vertical:ResponseFactors", 1);
        // Note: verify index with your version of InputProcessor; this follows the earlier code/assumption
        H = rfFields.getReal(2); // N3: Borehole depth (H)
    }

    // Step 2: Parse all GradientSegments objects (if present) and initialize model
    int numObjs = state.dataInputProcessing->inputProcessor->getNumObjectsFound("Site:GroundTemperature:GradientSegments");
    for (int objNum = 1; objNum <= numObjs; ++objNum) {
        auto fields = state.dataInputProcessing->inputProcessor->getObjectItem("Site:GroundTemperature:GradientSegments", objNum);

        int nSeg = fields.getInt(0); // N1: Number of Segments

        std::vector<GradientSegment> idfSegments;
        for (int i = 0; i < nSeg; ++i) {
            Real64 upper = fields.getReal(1 + i * 3);
            Real64 lower = fields.getReal(2 + i * 3);
            if (lower == 200.0) lower = H; // replace default with ResponseFactors depth
            Real64 grad = fields.getReal(3 + i * 3);
            idfSegments.push_back({upper, lower, grad});
        }

        model->initialize(idfSegments, H);
    }

    return model;
}

// getInputs: parse the Site:GroundTemperature:GradientSegments object matching objectName
// If objectName is not found, this attempts to use the first matching object.
void GradientsGTM::getInputs(EnergyPlusData &state, std::string const &objectName)
{
    // Get borehole depth H (ResponseFactors) as in factory
    Real64 H = 200.0;
    int numRF = state.dataInputProcessing->inputProcessor->getNumObjectsFound("GroundHeatExchanger:Vertical:ResponseFactors");
    if (numRF > 0) {
        auto rfFields = state.dataInputProcessing->inputProcessor->getObjectItem("GroundHeatExchanger:Vertical:ResponseFactors", 1);
        H = rfFields.getReal(2);
    }

    int numObjs = state.dataInputProcessing->inputProcessor->getNumObjectsFound("Site:GroundTemperature:GradientSegments");
    if (numObjs == 0) {
        // No gradient segments provided; keep defaults (empty segments, default referenceTemp)
        return;
    }

    bool found = false;
    for (int objNum = 1; objNum <= numObjs; ++objNum) {
        auto fields = state.dataInputProcessing->inputProcessor->getObjectItem("Site:GroundTemperature:GradientSegments", objNum);

        // Try to read the object name if the InputProcessor supports it.
        // The exact accessor may vary by EnergyPlus version; this follows the style used elsewhere in this codebase.
        std::string name;
        try {
            name = fields.getString(0);
        } catch (...) {
            name = "";
        }

        if (!objectName.empty() && name != objectName) {
            continue; // not the object we're looking for
        }

        // Parse count and segments (same layout assumed as in the factory)
        int nSeg = fields.getInt(0);
        std::vector<GradientSegment> idfSegments;
        for (int i = 0; i < nSeg; ++i) {
            Real64 upper = fields.getReal(1 + i * 3);
            Real64 lower = fields.getReal(2 + i * 3);
            if (lower == 200.0) lower = H;
            Real64 grad = fields.getReal(3 + i * 3);
            idfSegments.push_back({upper, lower, grad});
        }

        initialize(idfSegments, H);
        found = true;
        break;
    }

    if (!found) {
        // If objectName wasn't matched, fall back to the first object
        auto fields = state.dataInputProcessing->inputProcessor->getObjectItem("Site:GroundTemperature:GradientSegments", 1);
        int nSeg = fields.getInt(0);
        std::vector<GradientSegment> idfSegments;
        for (int i = 0; i < nSeg; ++i) {
            Real64 upper = fields.getReal(1 + i * 3);
            Real64 lower = fields.getReal(2 + i * 3);
            if (lower == 200.0) lower = H;
            Real64 grad = fields.getReal(3 + i * 3);
            idfSegments.push_back({upper, lower, grad});
        }
        initialize(idfSegments, H);
    }

    // Compute a reference temperature for the gradient model.
    // The IDF object doesn't include an absolute Tref in the assumed layout, so compute a reasonable baseline:
    // use the Kusuda-Achenbach model at surface (depth 0) averaged over 12 months.
    Real64 sumT = 0.0;
    for (int month = 1; month <= 12; ++month) {
        // KusudaAchenbachGroundTemperatureModel::compute(...) is assumed available in this project.
        sumT += KusudaAchenbachGroundTemperatureModel::compute(month, 0.0);
    }
    referenceTemp = sumT / 12.0;
}

// getGroundTemperature: produce an overall (annual-mean) ground temperature at depth
Real64 GradientsGTM::getGroundTemperature(EnergyPlusData & /*state*/, Real64 depth)
{
    // Many EnergyPlus undisturbed models return monthly or time-specific results.
    // For simplicity and robustness, return the 12‑month mean hybrid far‑field temperature at this depth.
    Real64 sumT = 0.0;
    for (int month = 1; month <= 12; ++month) {
        sumT += getHybridFarfieldTemp(month, depth);
    }
    return sumT / 12.0;
}

// Hybrid far-field routine (month-specific)
Real64 GradientsGTM::getHybridFarfieldTemp(int month, Real64 depth)
{
    // Use Kusuda-Achenbach as far-field baseline, then blend toward gradient segments
    Real64 kaTemp = KusudaAchenbachGroundTemperatureModel::compute(month, depth);

    Real64 gradTemp = referenceTemp; // fallback
    for (const auto &seg : segments) {
        if (depth >= seg.upperDepth && depth <= seg.lowerDepth) {
            Real64 deltaDepth = depth - seg.upperDepth;
            gradTemp = referenceTemp + seg.gradient * deltaDepth;
            break;
        }
    }

    Real64 weight = boreholeDepth > 0.0 ? (depth / boreholeDepth) : 0.0;
    weight = std::clamp(weight, 0.0, 1.0);

    return (1.0 - weight) * kaTemp + weight * gradTemp;
}

} // namespace EnergyPlus