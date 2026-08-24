/*
  GradientsGTM.hh
  Header for GradientsGTM model

  Author: Koon Beng Ooi <ooi_kb3@hotmail.com>
  Date: 2026-08-xx

  Brief:
  - Public class/struct declarations to support the GradientsGTM implementation
  - API notes and simple usage example included below

  Example usage:
    GradientsGTM gtm;
    gtm.setSurfaceTemperature(15.0);
    gtm.setVerticalGradient(0.057); // K/m
    gtm.initializeSegments(...);
*/

#ifndef GradientsGTM_hh
#define GradientsGTM_hh

#include <vector>
#include <memory>
#include <string>
#include "EnergyPlus/Data/EnergyPlusData.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh"

namespace EnergyPlus {
namespace GroundTemp {

class GradientsGTM : public BaseGroundTempsModel {
public:
    // --- Constructor and Factory ---
    GradientsGTM() = default;
    static std::unique_ptr<BaseGroundTempsModel> factory(EnergyPlusData &state, std::string const &objectName);
    
    // --- Public setter (TAKES 2 ARGUMENT!) ---
    void setBoreholeDepth(EnergyPlusData &state, Real64 H);

    // --- EXACT OVERRIDES FROM THE BASE CLASS ---
    Real64 getGroundTemp(EnergyPlusData &state) override;
    Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds) override;
    Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month) override;

    // --- NEW PUBLIC METHODS ---
    void setSurfaceTemperature(Real64 T_surface);
    void setVerticalGradient(Real64 gradient);
    void initializeSegments(EnergyPlusData &state);

private:
    // --- Define GradientSegment struct 
struct GradientSegment { Real64 upperDepth; Real64 lowerDepth; Real64 gradient; };

    // --- Private helper functions ---
void parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object);
    void initialize(EnergyPlusData &state, const std::vector<GradientSegment> &idfSegments, Real64 H);
    Real64 getHybridFarfieldTemp(EnergyPlusData &state, int month, Real64 depth) const;

// --- Member variables ---
    std::vector<GradientSegment> idfSegments_; 
    std::vector<GradientSegment> segments_;
    Real64 boreholeDepth{0.0};
    Real64 referenceTemp{0.0};
    Real64 transitionDepth{15.0};
    Real64 blendWidth{5.0};
    Real64 uniformGradient = 0.0;

};

} // namespace GroundTemp
} // namespace EnergyPlus

#endif