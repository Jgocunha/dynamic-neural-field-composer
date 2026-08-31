#include <gtest/gtest.h>
#include <string>

#include "elements/asymmetric_gauss_kernel.h"
#include "elements/gauss_kernel.h"
#include "elements/gauss_stimulus.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/oscillatory_kernel.h"

using namespace dnf_composer::element;

// Tests for the concrete per-element parameter structs of the kernel and
// stimulus elements. tests/element_parameters/test_element_parameters.cpp
// covers only the base types (ElementCommonParameters, ElementDimensions,
// ElementIdentifiers, ElementSpecificParameters).
//
// Every struct here compares doubles with a 1e-6 epsilon rather than exactly,
// so operator== is asserted both for differences below that epsilon (which must
// still compare equal) and above it (which must not).

namespace
{
    constexpr double kBelowEpsilon = 1e-9;
    constexpr double kAboveEpsilon = 1e-3;

    ::testing::AssertionResult ContainsAll(const std::string& text,
                                           const std::initializer_list<const char*> needles)
    {
        for (const char* needle : needles)
        {
            if (text.find(needle) == std::string::npos) {
                return ::testing::AssertionFailure()
                    << "\"" << text << "\" does not contain \"" << needle << "\"";
            }
        }
        return ::testing::AssertionSuccess();
    }
}

// ---------------------------------------------------------------------------
// GaussKernelParameters
// ---------------------------------------------------------------------------

TEST(GaussKernelParametersStruct, DefaultConstruction)
{
    const GaussKernelParameters p;
    EXPECT_DOUBLE_EQ(p.width, 3.0);
    EXPECT_DOUBLE_EQ(p.amplitude, 3.0);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, -0.01);
    EXPECT_TRUE(p.circular);
    EXPECT_TRUE(p.normalized);
}

TEST(GaussKernelParametersStruct, ExplicitConstruction)
{
    const GaussKernelParameters p{ 4.5, 7.0, -0.25, false, false };
    EXPECT_DOUBLE_EQ(p.width, 4.5);
    EXPECT_DOUBLE_EQ(p.amplitude, 7.0);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, -0.25);
    EXPECT_FALSE(p.circular);
    EXPECT_FALSE(p.normalized);
}

TEST(GaussKernelParametersStruct, EqualityIdentical)
{
    const GaussKernelParameters a{ 4.5, 7.0, -0.25, false, true };
    const GaussKernelParameters b{ 4.5, 7.0, -0.25, false, true };
    EXPECT_TRUE(a == b);
}

TEST(GaussKernelParametersStruct, EqualityToleratesSubEpsilonDifference)
{
    const GaussKernelParameters a{ 4.5, 7.0, -0.25, false, true };
    const GaussKernelParameters b{ 4.5 + kBelowEpsilon, 7.0, -0.25, false, true };
    EXPECT_TRUE(a == b);
}

TEST(GaussKernelParametersStruct, EqualityDifferentWidth)
{
    const GaussKernelParameters a{ 4.5, 7.0, -0.25, false, true };
    const GaussKernelParameters b{ 4.5 + kAboveEpsilon, 7.0, -0.25, false, true };
    EXPECT_FALSE(a == b);
}

TEST(GaussKernelParametersStruct, EqualityDifferentAmplitude)
{
    const GaussKernelParameters a{ 4.5, 7.0, -0.25, false, true };
    const GaussKernelParameters b{ 4.5, 8.0, -0.25, false, true };
    EXPECT_FALSE(a == b);
}

TEST(GaussKernelParametersStruct, EqualityDifferentAmplitudeGlobal)
{
    const GaussKernelParameters a{ 4.5, 7.0, -0.25, false, true };
    const GaussKernelParameters b{ 4.5, 7.0, -0.5, false, true };
    EXPECT_FALSE(a == b);
}

TEST(GaussKernelParametersStruct, EqualityDifferentCircular)
{
    const GaussKernelParameters a{ 4.5, 7.0, -0.25, false, true };
    const GaussKernelParameters b{ 4.5, 7.0, -0.25, true, true };
    EXPECT_FALSE(a == b);
}

TEST(GaussKernelParametersStruct, EqualityDifferentNormalized)
{
    const GaussKernelParameters a{ 4.5, 7.0, -0.25, false, true };
    const GaussKernelParameters b{ 4.5, 7.0, -0.25, false, false };
    EXPECT_FALSE(a == b);
}

TEST(GaussKernelParametersStruct, ToStringContainsValues)
{
    const GaussKernelParameters p{ 4.5, 7.0, -0.25, false, true };
    const std::string s = p.toString();
    EXPECT_FALSE(s.empty());
    EXPECT_TRUE(ContainsAll(s, { "4.50", "7.00", "-0.25", "false", "true" }));
}

// ---------------------------------------------------------------------------
// MexicanHatKernelParameters
// ---------------------------------------------------------------------------

TEST(MexicanHatKernelParametersStruct, DefaultConstruction)
{
    const MexicanHatKernelParameters p;
    EXPECT_DOUBLE_EQ(p.widthExc, 2.5);
    EXPECT_DOUBLE_EQ(p.amplitudeExc, 11.0);
    EXPECT_DOUBLE_EQ(p.widthInh, 5.0);
    EXPECT_DOUBLE_EQ(p.amplitudeInh, 15.0);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, -0.1);
    EXPECT_TRUE(p.circular);
    EXPECT_TRUE(p.normalized);
}

TEST(MexicanHatKernelParametersStruct, ExplicitConstruction)
{
    const MexicanHatKernelParameters p{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    EXPECT_DOUBLE_EQ(p.widthExc, 3.0);
    EXPECT_DOUBLE_EQ(p.amplitudeExc, 12.0);
    EXPECT_DOUBLE_EQ(p.widthInh, 6.0);
    EXPECT_DOUBLE_EQ(p.amplitudeInh, 18.0);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, -0.2);
    EXPECT_FALSE(p.circular);
    EXPECT_FALSE(p.normalized);
}

TEST(MexicanHatKernelParametersStruct, EqualityIdentical)
{
    const MexicanHatKernelParameters a{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    const MexicanHatKernelParameters b{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    EXPECT_TRUE(a == b);
}

TEST(MexicanHatKernelParametersStruct, EqualityToleratesSubEpsilonDifference)
{
    const MexicanHatKernelParameters a{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    const MexicanHatKernelParameters b{ 3.0, 12.0 + kBelowEpsilon, 6.0, 18.0, -0.2, false, false };
    EXPECT_TRUE(a == b);
}

TEST(MexicanHatKernelParametersStruct, EqualityDifferentWidthExc)
{
    const MexicanHatKernelParameters a{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    const MexicanHatKernelParameters b{ 3.0 + kAboveEpsilon, 12.0, 6.0, 18.0, -0.2, false, false };
    EXPECT_FALSE(a == b);
}

TEST(MexicanHatKernelParametersStruct, EqualityDifferentWidthInh)
{
    const MexicanHatKernelParameters a{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    const MexicanHatKernelParameters b{ 3.0, 12.0, 7.0, 18.0, -0.2, false, false };
    EXPECT_FALSE(a == b);
}

TEST(MexicanHatKernelParametersStruct, EqualityDifferentAmplitudeInh)
{
    const MexicanHatKernelParameters a{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    const MexicanHatKernelParameters b{ 3.0, 12.0, 6.0, 20.0, -0.2, false, false };
    EXPECT_FALSE(a == b);
}

TEST(MexicanHatKernelParametersStruct, EqualityDifferentFlags)
{
    const MexicanHatKernelParameters a{ 3.0, 12.0, 6.0, 18.0, -0.2, false, false };
    const MexicanHatKernelParameters b{ 3.0, 12.0, 6.0, 18.0, -0.2, true, false };
    EXPECT_FALSE(a == b);

    const MexicanHatKernelParameters c{ 3.0, 12.0, 6.0, 18.0, -0.2, false, true };
    EXPECT_FALSE(a == c);
}

TEST(MexicanHatKernelParametersStruct, ToStringContainsValues)
{
    const MexicanHatKernelParameters p{ 3.0, 12.0, 6.0, 18.0, -0.2, false, true };
    const std::string s = p.toString();
    EXPECT_FALSE(s.empty());
    EXPECT_TRUE(ContainsAll(s, { "3.00", "12.00", "6.00", "18.00", "-0.20", "false", "true" }));
}

// ---------------------------------------------------------------------------
// OscillatoryKernelParameters
//
// This is the only struct of the six whose constructor clamps its inputs:
// zeroCrossings into [0, 1], and a non-positive decay to 0.01.
// ---------------------------------------------------------------------------

TEST(OscillatoryKernelParametersStruct, DefaultConstruction)
{
    const OscillatoryKernelParameters p;
    EXPECT_DOUBLE_EQ(p.amplitude, 1.0);
    EXPECT_DOUBLE_EQ(p.decay, 0.08);
    EXPECT_DOUBLE_EQ(p.zeroCrossings, 0.3);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, -0.01);
    EXPECT_TRUE(p.circular);
    EXPECT_FALSE(p.normalized);
}

TEST(OscillatoryKernelParametersStruct, ExplicitConstructionWithinValidRanges)
{
    const OscillatoryKernelParameters p{ 2.0, 0.05, 0.7, -0.02, false, true };
    EXPECT_DOUBLE_EQ(p.amplitude, 2.0);
    EXPECT_DOUBLE_EQ(p.decay, 0.05);
    EXPECT_DOUBLE_EQ(p.zeroCrossings, 0.7);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, -0.02);
    EXPECT_FALSE(p.circular);
    EXPECT_TRUE(p.normalized);
}

TEST(OscillatoryKernelParametersStruct, EqualityIdentical)
{
    const OscillatoryKernelParameters a{ 2.0, 0.05, 0.7, -0.02, false, true };
    const OscillatoryKernelParameters b{ 2.0, 0.05, 0.7, -0.02, false, true };
    EXPECT_TRUE(a == b);
}

TEST(OscillatoryKernelParametersStruct, EqualityToleratesSubEpsilonDifference)
{
    const OscillatoryKernelParameters a{ 2.0, 0.05, 0.7, -0.02, false, true };
    const OscillatoryKernelParameters b{ 2.0, 0.05 + kBelowEpsilon, 0.7, -0.02, false, true };
    EXPECT_TRUE(a == b);
}

TEST(OscillatoryKernelParametersStruct, EqualityDifferentDecay)
{
    const OscillatoryKernelParameters a{ 2.0, 0.05, 0.7, -0.02, false, true };
    const OscillatoryKernelParameters b{ 2.0, 0.05 + kAboveEpsilon, 0.7, -0.02, false, true };
    EXPECT_FALSE(a == b);
}

TEST(OscillatoryKernelParametersStruct, EqualityDifferentZeroCrossings)
{
    const OscillatoryKernelParameters a{ 2.0, 0.05, 0.7, -0.02, false, true };
    const OscillatoryKernelParameters b{ 2.0, 0.05, 0.6, -0.02, false, true };
    EXPECT_FALSE(a == b);
}

TEST(OscillatoryKernelParametersStruct, EqualityDifferentAmplitudeGlobal)
{
    const OscillatoryKernelParameters a{ 2.0, 0.05, 0.7, -0.02, false, true };
    const OscillatoryKernelParameters b{ 2.0, 0.05, 0.7, -0.05, false, true };
    EXPECT_FALSE(a == b);
}

// Two parameter sets whose raw zeroCrossings differ but which clamp to the same
// value must compare equal — operator== sees the clamped members.
TEST(OscillatoryKernelParametersStruct, EqualityComparesClampedValues)
{
    const OscillatoryKernelParameters a{ 1.0, 0.08, 1.5, -0.01, true, false };
    const OscillatoryKernelParameters b{ 1.0, 0.08, 2.5, -0.01, true, false };
    EXPECT_TRUE(a == b);
}

TEST(OscillatoryKernelParametersStruct, ToStringContainsValues)
{
    const OscillatoryKernelParameters p{ 2.0, 0.05, 0.7, -0.02, false, true };
    const std::string s = p.toString();
    EXPECT_FALSE(s.empty());
    EXPECT_TRUE(ContainsAll(s, { "2.00", "0.05", "0.70", "-0.02", "false", "true" }));
}

TEST(OscillatoryKernelParametersStruct, ToStringReflectsClampedZeroCrossings)
{
    const OscillatoryKernelParameters p{ 1.0, 0.08, 1.5, -0.01, true, false };
    EXPECT_TRUE(ContainsAll(p.toString(), { "Zero crossings: 1.00" }));
}

// ---------------------------------------------------------------------------
// AsymmetricGaussKernelParameters
// ---------------------------------------------------------------------------

TEST(AsymmetricGaussKernelParametersStruct, DefaultConstruction)
{
    const AsymmetricGaussKernelParameters p;
    EXPECT_DOUBLE_EQ(p.width, 3.0);
    EXPECT_DOUBLE_EQ(p.amplitude, 3.0);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, 0.0);
    EXPECT_DOUBLE_EQ(p.timeShift, 0.0);
    EXPECT_TRUE(p.circular);
    EXPECT_TRUE(p.normalized);
}

TEST(AsymmetricGaussKernelParametersStruct, ExplicitConstruction)
{
    const AsymmetricGaussKernelParameters p{ 6.0, 5.0, 0.01, 2.0, false, false };
    EXPECT_DOUBLE_EQ(p.width, 6.0);
    EXPECT_DOUBLE_EQ(p.amplitude, 5.0);
    EXPECT_DOUBLE_EQ(p.amplitudeGlobal, 0.01);
    EXPECT_DOUBLE_EQ(p.timeShift, 2.0);
    EXPECT_FALSE(p.circular);
    EXPECT_FALSE(p.normalized);
}

TEST(AsymmetricGaussKernelParametersStruct, EqualityIdentical)
{
    const AsymmetricGaussKernelParameters a{ 6.0, 5.0, 0.01, 2.0, false, false };
    const AsymmetricGaussKernelParameters b{ 6.0, 5.0, 0.01, 2.0, false, false };
    EXPECT_TRUE(a == b);
}

TEST(AsymmetricGaussKernelParametersStruct, EqualityToleratesSubEpsilonDifference)
{
    const AsymmetricGaussKernelParameters a{ 6.0, 5.0, 0.01, 2.0, false, false };
    const AsymmetricGaussKernelParameters b{ 6.0, 5.0, 0.01, 2.0 + kBelowEpsilon, false, false };
    EXPECT_TRUE(a == b);
}

// timeShift is the member that distinguishes this struct from GaussKernelParameters,
// so it must participate in the comparison.
TEST(AsymmetricGaussKernelParametersStruct, EqualityDifferentTimeShift)
{
    const AsymmetricGaussKernelParameters a{ 6.0, 5.0, 0.01, 2.0, false, false };
    const AsymmetricGaussKernelParameters b{ 6.0, 5.0, 0.01, 2.0 + kAboveEpsilon, false, false };
    EXPECT_FALSE(a == b);
}

TEST(AsymmetricGaussKernelParametersStruct, EqualityDifferentWidth)
{
    const AsymmetricGaussKernelParameters a{ 6.0, 5.0, 0.01, 2.0, false, false };
    const AsymmetricGaussKernelParameters b{ 7.0, 5.0, 0.01, 2.0, false, false };
    EXPECT_FALSE(a == b);
}

TEST(AsymmetricGaussKernelParametersStruct, EqualityDifferentFlags)
{
    const AsymmetricGaussKernelParameters a{ 6.0, 5.0, 0.01, 2.0, false, false };
    const AsymmetricGaussKernelParameters b{ 6.0, 5.0, 0.01, 2.0, true, false };
    EXPECT_FALSE(a == b);

    const AsymmetricGaussKernelParameters c{ 6.0, 5.0, 0.01, 2.0, false, true };
    EXPECT_FALSE(a == c);
}

TEST(AsymmetricGaussKernelParametersStruct, ToStringContainsValues)
{
    const AsymmetricGaussKernelParameters p{ 6.0, 5.0, 0.01, 2.0, false, true };
    const std::string s = p.toString();
    EXPECT_FALSE(s.empty());
    EXPECT_TRUE(ContainsAll(s, { "6.00", "5.00", "0.01", "2.00", "false", "true" }));
}

// ---------------------------------------------------------------------------
// GaussStimulusParameters
// ---------------------------------------------------------------------------

TEST(GaussStimulusParametersStruct, DefaultConstruction)
{
    const GaussStimulusParameters p;
    EXPECT_DOUBLE_EQ(p.width, 5.0);
    EXPECT_DOUBLE_EQ(p.amplitude, 15.0);
    EXPECT_DOUBLE_EQ(p.position, 50.0);
    EXPECT_TRUE(p.circular);
    EXPECT_FALSE(p.normalized);
}

TEST(GaussStimulusParametersStruct, ExplicitConstruction)
{
    const GaussStimulusParameters p{ 7.5, 20.0, 33.0, false, true };
    EXPECT_DOUBLE_EQ(p.width, 7.5);
    EXPECT_DOUBLE_EQ(p.amplitude, 20.0);
    EXPECT_DOUBLE_EQ(p.position, 33.0);
    EXPECT_FALSE(p.circular);
    EXPECT_TRUE(p.normalized);
}

TEST(GaussStimulusParametersStruct, EqualityIdentical)
{
    const GaussStimulusParameters a{ 7.5, 20.0, 33.0, false, true };
    const GaussStimulusParameters b{ 7.5, 20.0, 33.0, false, true };
    EXPECT_TRUE(a == b);
}

TEST(GaussStimulusParametersStruct, EqualityToleratesSubEpsilonDifference)
{
    const GaussStimulusParameters a{ 7.5, 20.0, 33.0, false, true };
    const GaussStimulusParameters b{ 7.5, 20.0, 33.0 + kBelowEpsilon, false, true };
    EXPECT_TRUE(a == b);
}

TEST(GaussStimulusParametersStruct, EqualityDifferentPosition)
{
    const GaussStimulusParameters a{ 7.5, 20.0, 33.0, false, true };
    const GaussStimulusParameters b{ 7.5, 20.0, 33.0 + kAboveEpsilon, false, true };
    EXPECT_FALSE(a == b);
}

TEST(GaussStimulusParametersStruct, EqualityDifferentWidth)
{
    const GaussStimulusParameters a{ 7.5, 20.0, 33.0, false, true };
    const GaussStimulusParameters b{ 8.5, 20.0, 33.0, false, true };
    EXPECT_FALSE(a == b);
}

TEST(GaussStimulusParametersStruct, EqualityDifferentAmplitude)
{
    const GaussStimulusParameters a{ 7.5, 20.0, 33.0, false, true };
    const GaussStimulusParameters b{ 7.5, 25.0, 33.0, false, true };
    EXPECT_FALSE(a == b);
}

TEST(GaussStimulusParametersStruct, EqualityDifferentFlags)
{
    const GaussStimulusParameters a{ 7.5, 20.0, 33.0, false, true };
    const GaussStimulusParameters b{ 7.5, 20.0, 33.0, true, true };
    EXPECT_FALSE(a == b);

    const GaussStimulusParameters c{ 7.5, 20.0, 33.0, false, false };
    EXPECT_FALSE(a == c);
}

TEST(GaussStimulusParametersStruct, ToStringContainsValues)
{
    const GaussStimulusParameters p{ 7.5, 20.0, 33.0, false, true };
    const std::string s = p.toString();
    EXPECT_FALSE(s.empty());
    EXPECT_TRUE(ContainsAll(s, { "7.50", "20.00", "33.00", "false", "true" }));
}

// ---------------------------------------------------------------------------
// NormalNoiseParameters
// ---------------------------------------------------------------------------

TEST(NormalNoiseParametersStruct, DefaultConstruction)
{
    const NormalNoiseParameters p;
    EXPECT_DOUBLE_EQ(p.amplitude, 0.2);
}

TEST(NormalNoiseParametersStruct, ExplicitConstruction)
{
    const NormalNoiseParameters p{ 1.25 };
    EXPECT_DOUBLE_EQ(p.amplitude, 1.25);
}

TEST(NormalNoiseParametersStruct, EqualityIdentical)
{
    const NormalNoiseParameters a{ 1.25 };
    const NormalNoiseParameters b{ 1.25 };
    EXPECT_TRUE(a == b);
}

TEST(NormalNoiseParametersStruct, EqualityToleratesSubEpsilonDifference)
{
    const NormalNoiseParameters a{ 1.25 };
    const NormalNoiseParameters b{ 1.25 + kBelowEpsilon };
    EXPECT_TRUE(a == b);
}

TEST(NormalNoiseParametersStruct, EqualityDifferentAmplitude)
{
    const NormalNoiseParameters a{ 1.25 };
    const NormalNoiseParameters b{ 1.25 + kAboveEpsilon };
    EXPECT_FALSE(a == b);
}

TEST(NormalNoiseParametersStruct, ToStringContainsValues)
{
    const NormalNoiseParameters p{ 1.25 };
    const std::string s = p.toString();
    EXPECT_FALSE(s.empty());
    EXPECT_TRUE(ContainsAll(s, { "Amplitude", "1.25" }));
}
