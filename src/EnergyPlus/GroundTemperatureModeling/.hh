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

private:
    // --- Define GradientSegment FIRST, before any vectors use it ---
    struct GradientSegment {
        Real64 upperDepth;
        Real64 lowerDepth;
        Real64 gradient;
    };

public:
    // --- Constructor and Factory ---
    GradientsGTM() = default;
    static std::unique_ptr<BaseGroundTempsModel> factory(EnergyPlusData &state, std::string const &objectName);
    
    // --- Public setter ---
    myGroundTempModel->setBoreholeDepth(state, boreholeDepth);
    void setBoreholeDepth(EnergyPlusData &state, Real64 H);
   // void setBoreholeDepth(Real64 H);
   // void initialize(const std::vector<GradientSegment> &idfSegments, Real64 H); // <-- ADD THIS
void initialize(EnergyPlusData &state, const std::vector<GradientSegment> &idfSegments, Real64 H);
    // --- EXACT OVERRIDES FROM THE BASE CLASS ---
    Real64 getGroundTemp(EnergyPlusData &state) override;
    Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds) override;
    Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month) override;

private:
    // --- Private helper functions ---
    Real64 computeKusudaTemp(int month, Real64 depth) const;
    void parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object);
    
    Real64 getHybridFarfieldTemp(int month, Real64 depth) const;

    // --- Member variables ---
    std::vector<GradientSegment> idfSegments_; 
    std::vector<GradientSegment> segments;     
    Real64 boreholeDepth = 0.0;
    Real64 referenceTemp = 0.0;
    Real64 transitionDepth = 15.0;
    Real64 blendWidth = 5.0;
};

} // namespace GroundTemp
} // namespace EnergyPlus

#endif