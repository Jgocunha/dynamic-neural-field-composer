#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "elements/gauss_kernel.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/gauss_stimulus.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "simulation/simulation.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Shared helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 100)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<GaussStimulus> makeStimulus(const std::string& name,
                                                    const double pos = 50.0, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    GaussStimulusParameters gsp{ 5.0, 15.0, pos, true, false };
    return std::make_shared<GaussStimulus>(cp, gsp);
}

// ---------------------------------------------------------------------------
// Analytic references for the exact-value tests
//
// These re-derive each kernel from its documented formula independently of the
// library (no call into tools::math), so a change in the production maths shows
// up as a test failure instead of being silently absorbed. Every Kernel
// subclass hardcodes cutOfFactor = 5 in Kernel's constructor.
//
// Tolerance: the references perform the same handful of double-precision
// operations as the implementation (an exp, a divide, a multiply, and for the
// normalized variants a sum over at most a few hundred terms). Worst-case
// relative error is a few ULP, i.e. ~1e-16 relative; 1e-12 relative leaves four
// orders of magnitude of headroom while still being far tighter than any real
// formula change could slip through. Values are O(1) or smaller, so an absolute
// 1e-12 bound on values scaled by the kernel amplitude is used directly.
// ---------------------------------------------------------------------------

static constexpr int kCutOfFactor = 5;
static constexpr double kExactTolerance = 1e-12;

/// Mirror of tools::math::computeKernelRange, re-derived from its documented
/// behaviour: the kernel is truncated at sigma * cutOfFactor, then clamped to
/// half the field (circular) or the whole field (non-circular).
static std::array<int, 2> referenceKernelRange(const double sigma, const int fieldSize,
                                               const bool circular)
{
    const int cut = static_cast<int>(std::ceil(sigma * kCutOfFactor));
    if (circular)
    {
        const double half = (static_cast<double>(fieldSize) - 1) / 2;
        return std::min(std::array<int, 2>{ cut, cut },
                        std::array<int, 2>{ static_cast<int>(std::floor(half)),
                                            static_cast<int>(std::ceil(half)) });
    }
    return std::min(std::array<int, 2>{ cut, cut },
                    std::array<int, 2>{ fieldSize - 1, fieldSize - 1 });
}

/// The integer offsets -kernelRange[0] .. +kernelRange[1] the kernels build with std::iota.
static std::vector<int> referenceRangeX(const std::array<int, 2>& kernelRange)
{
    std::vector<int> rangeX(static_cast<std::size_t>(kernelRange[0] + kernelRange[1] + 1));
    std::iota(rangeX.begin(), rangeX.end(), -kernelRange[0]);
    return rangeX;
}

/// exp(-0.5 * x^2 / sigma^2), optionally divided by the sum over the range.
static std::vector<double> referenceGauss(const std::vector<int>& rangeX, const double sigma,
                                          const bool normalized)
{
    std::vector<double> g(rangeX.size());
    for (std::size_t i = 0; i < rangeX.size(); ++i)
    {
        const double x = rangeX[i];
        g[i] = std::exp(-0.5 * (x * x) / (sigma * sigma));
    }
    if (normalized)
    {
        const double sum = std::accumulate(g.begin(), g.end(), 0.0);
        for (double& v : g) {
            v /= sum;
        }
    }
    return g;
}

/// d/dx of amplitude * exp(-0.5 * x^2 / sigma^2), optionally normalized by the
/// sum of absolute values (which preserves sign, unlike the plain-sum
/// normalization the Gaussians use).
static std::vector<double> referenceGaussDerivative(const std::vector<int>& rangeX,
                                                    const double sigma, const double amplitude,
                                                    const bool normalized)
{
    const double variance = sigma * sigma;
    std::vector<double> d(rangeX.size());
    for (std::size_t i = 0; i < rangeX.size(); ++i)
    {
        const double x = rangeX[i];
        d[i] = -x / variance * amplitude * std::exp(-0.5 * (x * x) / variance);
    }
    if (normalized)
    {
        const double sumOfAbs = std::accumulate(d.begin(), d.end(), 0.0,
            [](const double s, const double v) { return s + std::abs(v); });
        if (sumOfAbs > 0.0)
        {
            for (double& v : d) {
                v /= sumOfAbs;
            }
        }
    }
    return d;
}

/// Assert `actual` matches `expected` elementwise, reporting the offending index.
static void expectKernelNear(const std::vector<double>& actual,
                             const std::vector<double>& expected)
{
    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(actual[i], expected[i], kExactTolerance) << "at kernel index " << i;
    }
}

// ============================================================================
// GaussKernel
// ============================================================================

TEST(GaussKernelConstruction, LabelIsGaussKernel)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    const GaussKernelParameters gkp;
    const GaussKernel k(cp, gkp);
    EXPECT_EQ(k.getLabel(), ElementLabel::GAUSS_KERNEL);
}

TEST(GaussKernelConstruction, KernelComponentExistsAfterConstruction)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{});
    EXPECT_NO_THROW(k.getComponent("kernel"));
}

TEST(GaussKernelInit, KernelComponentHasNonZeroValues)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 5.0, 3.0, -0.01, true, true });
    k.init();
    auto kernel = k.getComponent("kernel");
    const double sum = std::accumulate(kernel.begin(), kernel.end(), 0.0,
                                  [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(sum, 0.0);
}

TEST(GaussKernelInit, KernelRangeIsPositive)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 5.0, 3.0, -0.01, true, true });
    k.init();
    const auto range = k.getKernelRange();
    EXPECT_GE(range[0], 0);
    EXPECT_GE(range[1], 0);
}

TEST(GaussKernelInit, ExtIndexIsNonEmptyForCircular)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 5.0, 3.0, -0.01, true, true });
    k.init();
    EXPECT_FALSE(k.getExtIndex().empty());
}

TEST(GaussKernelParameters, GetParametersRoundtrip)
{
    const GaussKernelParameters gkp{ 4.0, 2.5, -0.05, false, false };
    const ElementCommonParameters cp{ std::string("k"), 100 };
    const GaussKernel k(cp, gkp);
    EXPECT_EQ(k.getParameters(), gkp);
}

TEST(GaussKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 3.0, 2.0, -0.01, true, true });
    k.init();
    const auto before = k.getComponent("kernel");

    const GaussKernelParameters newP{ 8.0, 5.0, -0.02, true, true };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");

    EXPECT_NE(before.size(), 0u);
    EXPECT_NE(after.size(), 0u);
    // Different widths should produce different kernel vectors
    EXPECT_NE(before, after);
}

TEST(GaussKernelClone, CloneHasSameParameters)
{
    GaussKernelParameters gkp{ 5.0, 3.0, -0.01, true, true };
    ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, gkp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<GaussKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), gkp);
}

TEST(GaussKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    const GaussKernel k(cp, GaussKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}

// Integration: a kernel step produces non-trivial output when driven by a neural field
TEST(GaussKernelIntegration, StepProducesOutputWhenDrivenByField)
{
    Simulation sim("gk-test", 1.0, 0.0, 0.0);
    const auto stim  = makeStimulus("stim", 50.0);
    const auto field = makeField("field");
    ElementCommonParameters kcp{ std::string("kernel"), 100 };
    const auto kernel = std::make_shared<GaussKernel>(kcp, GaussKernelParameters{ 5.0, 3.0, -0.01,
        true, true });

    sim.addElement(stim);
    sim.addElement(field);
    sim.addElement(kernel);
    sim.createInteraction("stim",  "output", "field");
    sim.createInteraction("field", "output", "kernel");
    sim.createInteraction("kernel","output", "field");
    sim.init();

    for (int i = 0; i < 10; ++i)
        sim.step();

    auto out = sim.getComponent("kernel", "output");
    const double absSum = std::accumulate(out.begin(), out.end(), 0.0,
                                     [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(absSum, 0.0);
}

// ============================================================================
// MexicanHatKernel
// ============================================================================

TEST(MexicanHatKernelConstruction, LabelIsMexicanHatKernel)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernel k(cp, MexicanHatKernelParameters{});
    EXPECT_EQ(k.getLabel(), ElementLabel::MEXICAN_HAT_KERNEL);
}

TEST(MexicanHatKernelInit, KernelHasPositiveAndNegativeValues)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernelParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1,
        true, true };
    MexicanHatKernel k(cp, mhkp);
    k.init();
    auto kernel = k.getComponent("kernel");
    const bool hasPositive = std::ranges::any_of(kernel, [](const double v){ return v > 0.0; });
    const bool hasNegative = std::ranges::any_of(kernel, [](const double v){ return v < 0.0; });
    EXPECT_TRUE(hasPositive);
    EXPECT_TRUE(hasNegative);
}

TEST(MexicanHatKernelParameters, GetParametersRoundtrip)
{
    const MexicanHatKernelParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1,
        true, true };
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernel k(cp, mhkp);
    EXPECT_EQ(k.getParameters(), mhkp);
}

TEST(MexicanHatKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    MexicanHatKernel k(cp, MexicanHatKernelParameters{});
    k.init();
    const auto before = k.getComponent("kernel");

    const MexicanHatKernelParameters newP{ 5.0, 20.0, 10.0, 25.0, -0.2,
        true, true };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");
    EXPECT_NE(before, after);
}

TEST(MexicanHatKernelClone, CloneHasSameParameters)
{
    const MexicanHatKernelParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1,
        true, true };
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    MexicanHatKernel k(cp, mhkp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<MexicanHatKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), mhkp);
}

TEST(MexicanHatKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernel k(cp, MexicanHatKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}

// ============================================================================
// OscillatoryKernel
// ============================================================================

TEST(OscillatoryKernelParameters, ZeroCrossingsClampedToZeroWhenNegative)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, -0.5, -0.01,
        true, false };
    EXPECT_DOUBLE_EQ(okp.zeroCrossings, 0.0);
}

TEST(OscillatoryKernelParameters, ZeroCrossingsClampedToOneWhenAboveOne)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, 1.5, -0.01,
        true, false };
    EXPECT_DOUBLE_EQ(okp.zeroCrossings, 1.0);
}

TEST(OscillatoryKernelParameters, DecayClampedToPositiveWhenZeroOrNegative)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.0, 0.3, -0.01,
        true, false };
    EXPECT_GT(okp.decay, 0.0);

    const OscillatoryKernelParameters okp2{ 1.0, -1.0, 0.3, -0.01,
        true, false };
    EXPECT_GT(okp2.decay, 0.0);
}

TEST(OscillatoryKernelConstruction, LabelIsOscillatoryKernel)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    const OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    EXPECT_EQ(k.getLabel(), ElementLabel::OSCILLATORY_KERNEL);
}

TEST(OscillatoryKernelInit, KernelHasNonZeroValues)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    k.init();
    auto kernel = k.getComponent("kernel");
    const double absSum = std::accumulate(kernel.begin(), kernel.end(), 0.0,
                                     [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(absSum, 0.0);
}

TEST(OscillatoryKernelParameters, GetParametersRoundtrip)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, 0.3, -0.01,
        true, false };
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    const OscillatoryKernel k(cp, okp);
    EXPECT_EQ(k.getParameters(), okp);
}

TEST(OscillatoryKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    k.init();
    const auto before = k.getComponent("kernel");

    const OscillatoryKernelParameters newP{ 2.0, 0.05, 0.5, -0.02,
        true, false };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");
    EXPECT_NE(before, after);
}

TEST(OscillatoryKernelClone, CloneHasSameParameters)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, 0.3, -0.01,
        true, false };
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    OscillatoryKernel k(cp, okp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<OscillatoryKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), okp);
}

TEST(OscillatoryKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    const OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}

// ============================================================================
// AsymmetricGaussKernel
// ============================================================================

TEST(AsymmetricGaussKernelConstruction, LabelIsAsymmetricGaussKernel)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{});
    EXPECT_EQ(k.getLabel(), ElementLabel::ASYMMETRIC_GAUSS_KERNEL);
}

TEST(AsymmetricGaussKernelInit, KernelHasNonZeroValues)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernelParameters agkp{ 3.0, 3.0, 0.0, 0.0,
        true, true };
    AsymmetricGaussKernel k(cp, agkp);
    k.init();
    auto kernel = k.getComponent("kernel");
    const double absSum = std::accumulate(kernel.begin(), kernel.end(), 0.0,
                                     [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(absSum, 0.0);
}

TEST(AsymmetricGaussKernelParameters, GetParametersRoundtrip)
{
    const AsymmetricGaussKernelParameters agkp{ 3.0, 3.0, 0.0, 1.0,
        true, true };
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernel k(cp, agkp);
    EXPECT_EQ(k.getParameters(), agkp);
}

TEST(AsymmetricGaussKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{});
    k.init();
    const auto before = k.getComponent("kernel");

    const AsymmetricGaussKernelParameters newP{ 6.0, 5.0, 0.01, 2.0,
        true, true };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");
    EXPECT_NE(before, after);
}

TEST(AsymmetricGaussKernelClone, CloneHasSameParameters)
{
    const AsymmetricGaussKernelParameters agkp{ 3.0, 3.0, 0.0, 1.0,
        true, true };
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    AsymmetricGaussKernel k(cp, agkp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<AsymmetricGaussKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), agkp);
}

TEST(AsymmetricGaussKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}

// ============================================================================
// Exact kernel values
//
// The tests above assert only that a kernel is non-degenerate (non-zero sum,
// has both signs). These assert the whole kernel vector against an analytic
// reference derived from each kernel's documented formula, plus the kernel
// size, which is kernelRange[0] + kernelRange[1] + 1 for every 1D kernel.
// ============================================================================

// ---------------------------------------------------------------------------
// GaussKernel: amplitude * exp(-0.5 * x^2 / width^2), normalized by the sum
// over the kernel range when `normalized`.
// ---------------------------------------------------------------------------

namespace
{
    void checkGaussKernelExactValues(const double width, const double amplitude,
                                     const bool circular, const bool normalized,
                                     const int fieldSize = 100)
    {
        const ElementCommonParameters cp{ std::string("gk"), fieldSize };
        GaussKernel k(cp, GaussKernelParameters{ width, amplitude, -0.01, circular, normalized });
        k.init();

        const auto expectedRange = referenceKernelRange(width, fieldSize, circular);
        EXPECT_EQ(k.getKernelRange(), expectedRange);

        const auto rangeX = referenceRangeX(expectedRange);
        auto expected = referenceGauss(rangeX, width, normalized);
        for (double& v : expected) {
            v *= amplitude;
        }

        const auto kernel = k.getComponent("kernel");
        EXPECT_EQ(kernel.size(), static_cast<std::size_t>(expectedRange[0] + expectedRange[1] + 1));
        expectKernelNear(kernel, expected);
    }
}

TEST(GaussKernelExactValues, NonCircularUnnormalized)
{
    checkGaussKernelExactValues(3.0, 5.0, false, false);
}

TEST(GaussKernelExactValues, NonCircularNormalized)
{
    checkGaussKernelExactValues(3.0, 5.0, false, true);
}

TEST(GaussKernelExactValues, CircularUnnormalized)
{
    checkGaussKernelExactValues(3.0, 5.0, true, false);
}

TEST(GaussKernelExactValues, CircularNormalized)
{
    checkGaussKernelExactValues(3.0, 5.0, true, true);
}

// A width wide enough that sigma * cutOfFactor exceeds half the field forces
// computeKernelRange's circular clamp, which is where the asymmetric
// {floor(half), ceil(half)} range appears.
TEST(GaussKernelExactValues, CircularWideWidthClampsRangeAsymmetrically)
{
    constexpr int fieldSize = 40;
    const auto expectedRange = referenceKernelRange(20.0, fieldSize, true);
    ASSERT_EQ(expectedRange[0], 19);
    ASSERT_EQ(expectedRange[1], 20);

    checkGaussKernelExactValues(20.0, 2.0, true, false, fieldSize);
}

TEST(GaussKernelExactValues, CenterTapIsAmplitudeWhenUnnormalized)
{
    const ElementCommonParameters cp{ std::string("gk"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 3.0, 5.0, -0.01, false, false });
    k.init();

    // rangeX runs -kernelRange[0]..+kernelRange[1], so index kernelRange[0] is x = 0,
    // where exp(0) == 1 and the tap is exactly the amplitude.
    const auto kernel = k.getComponent("kernel");
    EXPECT_NEAR(kernel[k.getKernelRange()[0]], 5.0, kExactTolerance);
}

TEST(GaussKernelExactValues, NormalizedKernelSumsToAmplitude)
{
    const ElementCommonParameters cp{ std::string("gk"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 3.0, 5.0, -0.01, false, true });
    k.init();

    const auto kernel = k.getComponent("kernel");
    EXPECT_NEAR(std::accumulate(kernel.begin(), kernel.end(), 0.0), 5.0, kExactTolerance);
}

// ---------------------------------------------------------------------------
// MexicanHatKernel: amplitudeExc * gauss(widthExc) - amplitudeInh * gauss(widthInh).
// The kernel range comes from the larger of the two widths (whichever has a
// non-zero amplitude).
// ---------------------------------------------------------------------------

namespace
{
    void checkMexicanHatKernelExactValues(const double widthExc, const double amplitudeExc,
                                          const double widthInh, const double amplitudeInh,
                                          const bool circular, const bool normalized,
                                          const int fieldSize = 100)
    {
        const ElementCommonParameters cp{ std::string("mhk"), fieldSize };
        MexicanHatKernel k(cp, MexicanHatKernelParameters{ widthExc, amplitudeExc, widthInh,
            amplitudeInh, -0.1, circular, normalized });
        k.init();

        const double maxWidth = std::max(amplitudeExc != 0.0 ? widthExc : 0.0,
                                         amplitudeInh != 0.0 ? widthInh : 0.0);
        const auto expectedRange = referenceKernelRange(maxWidth, fieldSize, circular);
        EXPECT_EQ(k.getKernelRange(), expectedRange);

        const auto rangeX = referenceRangeX(expectedRange);
        const auto gaussExc = referenceGauss(rangeX, widthExc, normalized);
        const auto gaussInh = referenceGauss(rangeX, widthInh, normalized);

        std::vector<double> expected(rangeX.size());
        for (std::size_t i = 0; i < expected.size(); ++i) {
            expected[i] = amplitudeExc * gaussExc[i] - amplitudeInh * gaussInh[i];
        }

        const auto kernel = k.getComponent("kernel");
        EXPECT_EQ(kernel.size(), static_cast<std::size_t>(expectedRange[0] + expectedRange[1] + 1));
        expectKernelNear(kernel, expected);
    }
}

TEST(MexicanHatKernelExactValues, NonCircularUnnormalized)
{
    checkMexicanHatKernelExactValues(2.5, 11.0, 5.0, 15.0, false, false);
}

TEST(MexicanHatKernelExactValues, NonCircularNormalized)
{
    checkMexicanHatKernelExactValues(2.5, 11.0, 5.0, 15.0, false, true);
}

TEST(MexicanHatKernelExactValues, CircularUnnormalized)
{
    checkMexicanHatKernelExactValues(2.5, 11.0, 5.0, 15.0, true, false);
}

TEST(MexicanHatKernelExactValues, CircularNormalized)
{
    checkMexicanHatKernelExactValues(2.5, 11.0, 5.0, 15.0, true, true);
}

// The kernel range is driven by the wider Gaussian, but only when its amplitude
// is non-zero — a zero inhibitory amplitude must fall back to the excitatory width.
TEST(MexicanHatKernelExactValues, ZeroInhibitoryAmplitudeUsesExcitatoryWidthForRange)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    MexicanHatKernel k(cp, MexicanHatKernelParameters{ 2.5, 11.0, 5.0, 0.0, -0.1, false, false });
    k.init();
    EXPECT_EQ(k.getKernelRange(), referenceKernelRange(2.5, 100, false));

    checkMexicanHatKernelExactValues(2.5, 11.0, 5.0, 0.0, false, false);
}

// ---------------------------------------------------------------------------
// OscillatoryKernel:
//   amplitude * exp(-decay * |x|)
//              * (sin(decay * |zeroCrossings * x|) + cos(zeroCrossings * x))
// then divided by the plain sum of the whole kernel when `normalized`.
// The kernel range uses max(1 / decay, zeroCrossings * cutOfFactor) as sigma.
// ---------------------------------------------------------------------------

namespace
{
    void checkOscillatoryKernelExactValues(const double amplitude, const double decay,
                                           const double zeroCrossings, const bool circular,
                                           const bool normalized, const int fieldSize = 100)
    {
        const ElementCommonParameters cp{ std::string("ok"), fieldSize };
        OscillatoryKernel k(cp, OscillatoryKernelParameters{ amplitude, decay, zeroCrossings,
            -0.01, circular, normalized });
        k.init();

        const double effectiveRange = std::max(1.0 / decay, zeroCrossings * kCutOfFactor);
        const auto expectedRange = referenceKernelRange(effectiveRange, fieldSize, circular);
        EXPECT_EQ(k.getKernelRange(), expectedRange);

        const auto rangeX = referenceRangeX(expectedRange);
        std::vector<double> expected(rangeX.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            const double distance = rangeX[i];
            const double decayFactor = std::exp(-decay * std::abs(distance));
            const double oscillation = std::sin(decay * std::abs(zeroCrossings * distance))
                                     + std::cos(zeroCrossings * distance);
            expected[i] = amplitude * decayFactor * oscillation;
        }
        if (normalized)
        {
            const double sum = std::accumulate(expected.begin(), expected.end(), 0.0);
            if (sum != 0.0)
            {
                for (double& v : expected) {
                    v /= sum;
                }
            }
        }

        const auto kernel = k.getComponent("kernel");
        EXPECT_EQ(kernel.size(), static_cast<std::size_t>(expectedRange[0] + expectedRange[1] + 1));
        expectKernelNear(kernel, expected);
    }
}

TEST(OscillatoryKernelExactValues, NonCircularUnnormalized)
{
    checkOscillatoryKernelExactValues(1.0, 0.08, 0.3, false, false);
}

TEST(OscillatoryKernelExactValues, NonCircularNormalized)
{
    checkOscillatoryKernelExactValues(1.0, 0.08, 0.3, false, true);
}

TEST(OscillatoryKernelExactValues, CircularUnnormalized)
{
    checkOscillatoryKernelExactValues(1.0, 0.08, 0.3, true, false);
}

TEST(OscillatoryKernelExactValues, CircularNormalized)
{
    checkOscillatoryKernelExactValues(1.0, 0.08, 0.3, true, true);
}

TEST(OscillatoryKernelExactValues, CenterTapIsAmplitudeWhenUnnormalized)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    OscillatoryKernel k(cp, OscillatoryKernelParameters{ 2.0, 0.08, 0.3, -0.01, false, false });
    k.init();

    // At x = 0: exp(0) * (sin(0) + cos(0)) == 1, so the centre tap is the amplitude.
    const auto kernel = k.getComponent("kernel");
    EXPECT_NEAR(kernel[k.getKernelRange()[0]], 2.0, kExactTolerance);
}

// ---------------------------------------------------------------------------
// AsymmetricGaussKernel:
//   amplitude * gauss(width) + timeShift * gaussDerivative(width, amplitude)
// The derivative uses sum-of-absolute-values normalization (sign preserving),
// unlike the plain-sum normalization of the Gaussian itself.
// ---------------------------------------------------------------------------

namespace
{
    void checkAsymmetricGaussKernelExactValues(const double width, const double amplitude,
                                               const double timeShift, const bool circular,
                                               const bool normalized, const int fieldSize = 100)
    {
        const ElementCommonParameters cp{ std::string("agk"), fieldSize };
        AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{ width, amplitude, 0.0,
            timeShift, circular, normalized });
        k.init();

        const auto expectedRange = referenceKernelRange(width, fieldSize, circular);
        EXPECT_EQ(k.getKernelRange(), expectedRange);

        const auto rangeX = referenceRangeX(expectedRange);
        const auto gauss = referenceGauss(rangeX, width, normalized);
        const auto derivative = referenceGaussDerivative(rangeX, width, amplitude, normalized);

        std::vector<double> expected(rangeX.size());
        for (std::size_t i = 0; i < expected.size(); ++i) {
            expected[i] = amplitude * gauss[i] + timeShift * derivative[i];
        }

        const auto kernel = k.getComponent("kernel");
        EXPECT_EQ(kernel.size(), static_cast<std::size_t>(expectedRange[0] + expectedRange[1] + 1));
        expectKernelNear(kernel, expected);
    }
}

TEST(AsymmetricGaussKernelExactValues, NonCircularUnnormalized)
{
    checkAsymmetricGaussKernelExactValues(3.0, 3.0, 1.0, false, false);
}

TEST(AsymmetricGaussKernelExactValues, NonCircularNormalized)
{
    checkAsymmetricGaussKernelExactValues(3.0, 3.0, 1.0, false, true);
}

TEST(AsymmetricGaussKernelExactValues, CircularUnnormalized)
{
    checkAsymmetricGaussKernelExactValues(3.0, 3.0, 1.0, true, false);
}

TEST(AsymmetricGaussKernelExactValues, CircularNormalized)
{
    checkAsymmetricGaussKernelExactValues(3.0, 3.0, 1.0, true, true);
}

// timeShift == 0 removes the derivative term entirely, leaving a plain Gaussian.
TEST(AsymmetricGaussKernelExactValues, ZeroTimeShiftReducesToGaussKernel)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    AsymmetricGaussKernel agk(cp, AsymmetricGaussKernelParameters{ 3.0, 3.0, 0.0, 0.0,
        false, false });
    agk.init();

    const ElementCommonParameters gcp{ std::string("gk"), 100 };
    GaussKernel gk(gcp, GaussKernelParameters{ 3.0, 3.0, 0.0, false, false });
    gk.init();

    expectKernelNear(agk.getComponent("kernel"), gk.getComponent("kernel"));
}

// The derivative term is odd in x, so a non-zero timeShift must break the
// symmetry a plain Gaussian has about its centre tap.
TEST(AsymmetricGaussKernelExactValues, NonZeroTimeShiftBreaksSymmetry)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{ 3.0, 3.0, 0.0, 1.0,
        false, false });
    k.init();

    const auto kernel = k.getComponent("kernel");
    const auto centre = static_cast<std::size_t>(k.getKernelRange()[0]);
    ASSERT_GT(centre, 0u);
    EXPECT_GT(std::abs(kernel[centre - 1] - kernel[centre + 1]), kExactTolerance);
}

// ---------------------------------------------------------------------------
// GaussKernel2D: the separable outer product of two 1D Gaussians, laid out
// row-major as kernel[j * kx + i] = (amplitude * gauss_x[i]) * gauss_y[j].
// The amplitude is folded into the x taps only, so it is applied once.
// ---------------------------------------------------------------------------

namespace
{
    void checkGaussKernel2DExactValues(const double width, const double amplitude,
                                       const bool circular, const bool normalized,
                                       const int sizeX = 20, const int sizeY = 16)
    {
        const ElementCommonParameters cp{ std::string("gk2d"),
            ElementDimensions(sizeX, sizeY, 1.0, 1.0) };
        GaussKernel2D k(cp, GaussKernel2DParameters{ width, amplitude, 0.0, circular, normalized });
        k.init();

        const auto rangeXBounds = referenceKernelRange(width, sizeX, circular);
        const auto rangeYBounds = referenceKernelRange(width, sizeY, circular);
        const auto rangeX = referenceRangeX(rangeXBounds);
        const auto rangeY = referenceRangeX(rangeYBounds);

        auto gaussX = referenceGauss(rangeX, width, normalized);
        const auto gaussY = referenceGauss(rangeY, width, normalized);
        for (double& v : gaussX) {
            v *= amplitude;
        }

        const auto kx = gaussX.size();
        const auto ky = gaussY.size();
        std::vector<double> expected(kx * ky);
        for (std::size_t i = 0; i < kx; ++i) {
            for (std::size_t j = 0; j < ky; ++j) {
                expected[j * kx + i] = gaussX[i] * gaussY[j];
            }
        }

        const auto kernel = k.getComponent("kernel");
        EXPECT_EQ(kernel.size(), kx * ky);
        expectKernelNear(kernel, expected);
    }
}

TEST(GaussKernel2DExactValues, NonCircularUnnormalized)
{
    checkGaussKernel2DExactValues(2.0, 3.0, false, false);
}

TEST(GaussKernel2DExactValues, NonCircularNormalized)
{
    checkGaussKernel2DExactValues(2.0, 3.0, false, true);
}

TEST(GaussKernel2DExactValues, CircularUnnormalized)
{
    checkGaussKernel2DExactValues(2.0, 3.0, true, false);
}

TEST(GaussKernel2DExactValues, CircularNormalized)
{
    checkGaussKernel2DExactValues(2.0, 3.0, true, true);
}
