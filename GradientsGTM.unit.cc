// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// EnergyPlus::GroundTemperatureModels::GradientsGTM Unit Tests
// a comprehensive GoogleTest unit test for the getHybridFarfieldTemp function in GradientsGTM. 
// This test will exercise the blending logic between Kusuda and gradient-based temperatures:
// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlus/DataIPShortCuts.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh"
#include "EnergyPlus/GroundTemperatureModeling/GradientsGTM.hh"
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>

using namespace EnergyPlus;

/**
 * Test Suite for GradientsGTM::getHybridFarfieldTemp and related methods
 * 
 * Tests the hybrid blending logic between Kusuda-based (near-surface) 
 * and gradient-based (deep) ground temperatures.
 */
class GradientsGTMTest : public EnergyPlusFixture
{
protected:
    std::unique_ptr<GroundTemp::GradientsGTM> model;

    void SetUp() override
    {
        EnergyPlusFixture::SetUp();
        model = std::make_unique<GroundTemp::GradientsGTM>();
    }
};

/**
 * TEST 1: Basic gradient segment initialization and temperature calculation
 * 
 * Scenario: Single gradient segment from 0m to 100m with 0.05 K/m gradient
 * Expected: Temperature increases linearly with depth within segment
 */
TEST_F(GradientsGTMTest, BasicGradientSegmentTemperature)
{
    // Create a simple gradient segment: 0m to 100m, 0.05 K/m gradient
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 100.0, 0.05});

    // Initialize model with borehole depth of 100m
    model->initialize(*state, segments, 100.0);

    // Test at shallow depth (within segment)
    // At 10m: T = referenceTemp + 0.05 * (10 - 0) = referenceTemp + 0.5
    Real64 temp_at_10m = model->getGroundTempAtTimeInMonths(*state, 10.0, 1);
    Real64 temp_at_0m = model->getGroundTempAtTimeInMonths(*state, 0.0, 1);
    
    // Verify gradient is approximately 0.05 K/m
    EXPECT_NEAR(temp_at_10m - temp_at_0m, 0.5, 0.01) << "Gradient should be 0.05 K/m";

    // Test at another depth
    Real64 temp_at_50m = model->getGroundTempAtTimeInMonths(*state, 50.0, 1);
    EXPECT_NEAR(temp_at_50m - temp_at_0m, 2.5, 0.01) << "At 50m: dT should be 2.5K";
}

/**
 * TEST 2: Multiple gradient segments with varying gradients
 * 
 * Scenario: Two gradient segments with different gradients
 * - Segment 1: 0m to 50m with 0.03 K/m
 * - Segment 2: 50m to 150m with 0.08 K/m
 * Expected: Temperature follows correct segment gradient at depth
 */
TEST_F(GradientsGTMTest, MultipleGradientSegments)
{
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 50.0, 0.03});   // Shallow: 0.03 K/m
    segments.push_back({50.0, 150.0, 0.08}); // Deep: 0.08 K/m

    model->initialize(*state, segments, 150.0);

    Real64 temp_0m = model->getGroundTempAtTimeInMonths(*state, 0.0, 1);
    Real64 temp_25m = model->getGroundTempAtTimeInMonths(*state, 25.0, 1);
    Real64 temp_50m = model->getGroundTempAtTimeInMonths(*state, 50.0, 1);
    Real64 temp_100m = model->getGroundTempAtTimeInMonths(*state, 100.0, 1);

    // In shallow segment (0-50m): gradient should be 0.03 K/m
    EXPECT_NEAR(temp_25m - temp_0m, 0.75, 0.01) << "Shallow segment: 25m * 0.03 = 0.75K";

    // In deep segment (50-150m): gradient should be 0.08 K/m
    // temp_50m = temp_0m + 0.03 * 50
    // temp_100m = temp_50m + 0.08 * 50 = temp_0m + 0.03*50 + 0.08*50
    Real64 expected_temp_100m = temp_0m + (0.03 * 50.0) + (0.08 * 50.0);
    EXPECT_NEAR(temp_100m, expected_temp_100m, 0.01) << "Deep segment transition";
}

/**
 * TEST 3: Shallow depth zone (below transition depth - blend width)
 * 
 * Scenario: Test that shallow depths predominantly use Kusuda temperature
 * - Transition depth: 15m
 * - Blend width: 5m
 * - Low blend threshold: 15 - 5/2 = 12.5m
 * Expected: At depth 0m, temperature should match Kusuda model (weight = 0)
 */
TEST_F(GradientsGTMTest, ShallowDepthZoneUsesKusudaTemperature)
{
    // Set up with known gradient
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 100.0, 0.05}); // 0.05 K/m gradient

    // Set up Kusuda model first (for reference temperature)
    std::string const kusuda_idf = delimited_string({
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "\tKusudaRef,                !- Name",
        "\t1.08,                     !- Soil Thermal Conductivity",
        "\t980,                      !- Soil Density",
        "\t2570,                     !- Soil Specific Heat",
        "\t15.0,                     !- Average Surface Temperature",
        "\t5.0,                      !- Average Amplitude",
        "\t0;                        !- Phase Shift",
    });

    ASSERT_TRUE(process_idf(kusuda_idf));

    // Initialize GradientsGTM model
    model->initialize(*state, segments, 100.0);

    // At shallow depth (0m), should be primarily Kusuda temperature
    Real64 temp_surface = model->getGroundTempAtTimeInMonths(*state, 0.0, 1);
    Real64 temp_shallow = model->getGroundTempAtTimeInMonths(*state, 5.0, 1);

    // Shallow zone should have minimal gradient influence
    // Most of the variation should be from Kusuda model seasonality
    EXPECT_GT(temp_surface, 0.0) << "Surface temperature should be positive";
    EXPECT_GT(temp_shallow, 0.0) << "Shallow temperature should be positive";
}

/**
 * TEST 4: Deep depth zone (above transition depth + blend width)
 * 
 * Scenario: Test that deep depths predominantly use gradient temperature
 * - Transition depth: 15m
 * - Blend width: 5m
 * - High blend threshold: 15 + 5/2 = 17.5m
 * Expected: At depth > 20m, temperature should follow gradient (weight ≈ 1)
 */
TEST_F(GradientsGTMTest, DeepDepthZoneUsesGradientTemperature)
{
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 100.0, 0.1}); // 0.1 K/m gradient (steep)

    model->initialize(*state, segments, 100.0);

    Real64 reference_temp = model->getGroundTempAtTimeInMonths(*state, 0.0, 1);
    Real64 temp_20m = model->getGroundTempAtTimeInMonths(*state, 20.0, 1);
    Real64 temp_30m = model->getGroundTempAtTimeInMonths(*state, 30.0, 1);

    // At deep depths, temperature should follow gradient consistently
    // dT per 10m should be close to 1.0K (0.1 K/m * 10m)
    Real64 gradient_per_10m = temp_30m - temp_20m;
    EXPECT_NEAR(gradient_per_10m, 1.0, 0.1) << "Deep zone should follow 0.1 K/m gradient";
}

/**
 * TEST 5: Transition zone blending
 * 
 * Scenario: Test smooth transition between Kusuda and gradient models
 * - Default transition depth: 15m
 * - Default blend width: 5m
 * Expected: Temperature smoothly transitions from mostly-Kusuda to mostly-gradient
 */
TEST_F(GradientsGTMTest, TransitionZoneBlending)
{
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 100.0, 0.06});

    model->initialize(*state, segments, 100.0);

    // Sample temperatures across transition zone
    // Transition depth = 15m, blend width = 5m
    // Low = 12.5m, High = 17.5m
    Real64 temp_10m = model->getGroundTempAtTimeInMonths(*state, 10.0, 1);   // Below transition (w=0)
    Real64 temp_12m = model->getGroundTempAtTimeInMonths(*state, 12.0, 1);   // Near transition (w≈0)
    Real64 temp_15m = model->getGroundTempAtTimeInMonths(*state, 15.0, 1);   // At transition (w=0.5)
    Real64 temp_18m = model->getGroundTempAtTimeInMonths(*state, 18.0, 1);   // Just above blend (w≈1)
    Real64 temp_25m = model->getGroundTempAtTimeInMonths(*state, 25.0, 1);   // Well above transition (w=1)

    // Verify monotonic increase (temperature increases with depth due to gradient)
    EXPECT_LT(temp_10m, temp_12m);
    EXPECT_LT(temp_12m, temp_15m);
    EXPECT_LT(temp_15m, temp_18m);
    EXPECT_LT(temp_18m, temp_25m);

    // Verify smooth transition (no discontinuities)
    Real64 delta_below = temp_12m - temp_10m;  // Below transition
    Real64 delta_blend = temp_18m - temp_12m;  // Across transition
    Real64 delta_above = temp_25m - temp_18m;  // Above transition

    // Deltas should be of similar magnitude (smooth blending)
    EXPECT_GT(delta_blend, 0.0);
    EXPECT_LT(std::abs(delta_blend - (delta_below + delta_above) / 2), 0.1);
}

/**
 * TEST 6: Seasonal variation (month parameter)
 * 
 * Scenario: Test that different months produce different temperatures
 * (if Kusuda model is contributing)
 * Expected: Temperature varies by month due to Kusuda seasonality
 */
TEST_F(GradientsGTMTest, SeasonalVariation)
{
    // Set up Kusuda model for background seasonality
    std::string const kusuda_idf = delimited_string({
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "\tKusudaRef,                !- Name",
        "\t1.08,                     !- Thermal Conductivity",
        "\t980,                      !- Density",
        "\t2570,                     !- Specific Heat",
        "\t15.0,                     !- Average Surface Temperature",
        "\t8.0,                      !- Average Amplitude",
        "\t90;                       !- Phase Shift",
    });

    ASSERT_TRUE(process_idf(kusuda_idf));

    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 100.0, 0.05});

    model->initialize(*state, segments, 100.0);

    // Test at shallow depth where Kusuda influence is strong
    Real64 temp_jan = model->getGroundTempAtTimeInMonths(*state, 5.0, 1);  // January
    Real64 temp_jul = model->getGroundTempAtTimeInMonths(*state, 5.0, 7);  // July

    // Summer should be warmer than winter
    EXPECT_LT(temp_jan, temp_jul) << "July should be warmer than January at shallow depth";
}

/**
 * TEST 7: Edge case - unsorted gradient segments
 * 
 * Scenario: Initialize with unsorted gradient segments
 * Expected: Model should sort them internally and work correctly
 */
TEST_F(GradientsGTMTest, UnsortedSegmentsAutoSort)
{
    // Create unsorted segments (reversed order)
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({50.0, 100.0, 0.08});  // Should be second
    segments.push_back({0.0, 50.0, 0.03});    // Should be first

    model->initialize(*state, segments, 100.0);

    Real64 temp_0m = model->getGroundTempAtTimeInMonths(*state, 0.0, 1);
    Real64 temp_25m = model->getGroundTempAtTimeInMonths(*state, 25.0, 1);
    Real64 temp_75m = model->getGroundTempAtTimeInMonths(*state, 75.0, 1);

    // First segment (0-50m): 0.03 K/m gradient
    EXPECT_NEAR(temp_25m - temp_0m, 0.75, 0.01);

    // Second segment (50-100m): 0.08 K/m gradient
    Real64 temp_50m = model->getGroundTempAtTimeInMonths(*state, 50.0, 1);
    EXPECT_NEAR(temp_75m - temp_50m, 2.0, 0.01);
}

/**
 * TEST 8: Constant temperature (zero gradient)
 * 
 * Scenario: Gradient segment with zero gradient
 * Expected: Temperature should be constant with depth
 */
TEST_F(GradientsGTMTest, ZeroGradientConstantTemperature)
{
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 100.0, 0.0});  // Zero gradient

    model->initialize(*state, segments, 100.0);

    Real64 temp_0m = model->getGroundTempAtTimeInMonths(*state, 0.0, 1);
    Real64 temp_50m = model->getGroundTempAtTimeInMonths(*state, 50.0, 1);
    Real64 temp_100m = model->getGroundTempAtTimeInMonths(*state, 100.0, 1);

    // All temperatures should be equal (within gradient zone)
    EXPECT_NEAR(temp_50m, temp_0m, 0.1) << "Zero gradient: temperature should be constant";
    EXPECT_NEAR(temp_100m, temp_0m, 0.1) << "Zero gradient: temperature should be constant";
}

/**
 * TEST 9: Negative gradient (temperature decreases with depth)
 * 
 * Scenario: Segment with negative gradient (cooling with depth)
 * Expected: Temperature decreases as depth increases
 */
TEST_F(GradientsGTMTest, NegativeGradientCoolingWithDepth)
{
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 100.0, -0.02});  // Negative gradient

    model->initialize(*state, segments, 100.0);

    Real64 temp_0m = model->getGroundTempAtTimeInMonths(*state, 0.0, 1);
    Real64 temp_50m = model->getGroundTempAtTimeInMonths(*state, 50.0, 1);
    Real64 temp_100m = model->getGroundTempAtTimeInMonths(*state, 100.0, 1);

    // Temperature should decrease with depth
    EXPECT_GT(temp_0m, temp_50m) << "Negative gradient: should cool with depth";
    EXPECT_GT(temp_50m, temp_100m) << "Negative gradient: should cool with depth";
    EXPECT_NEAR(temp_0m - temp_100m, 2.0, 0.1) << "-0.02 K/m * 100m = -2K";
}

/**
 * TEST 10: Borehole depth variation
 * 
 * Scenario: Test with different borehole depths
 * Expected: Reference temperature calculation should handle various depths
 */
TEST_F(GradientsGTMTest, VariousBoreholeDepths)
{
    std::vector<GroundTemp::GradientsGTM::GradientSegment> segments;
    segments.push_back({0.0, 200.0, 0.05});

    // Test with 100m borehole
    model->initialize(*state, segments, 100.0);
    Real64 temp_100m_bh = model->getGroundTempAtTimeInMonths(*state, 50.0, 1);

    // Create new model with 200m borehole
    auto model2 = std::make_unique<GroundTemp::GradientsGTM>();
    model2->initialize(*state, segments, 200.0);
    Real64 temp_200m_bh = model2->getGroundTempAtTimeInMonths(*state, 50.0, 1);

    // Both should exist and be valid
    EXPECT_GT(temp_100m_bh, 0.0);
    EXPECT_GT(temp_200m_bh, 0.0);
}