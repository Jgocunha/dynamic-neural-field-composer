#include <gtest/gtest.h>
#include <vector>
#include <array>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdexcept>

#include "tools/math.h"
#include "tools/fft_convolution.h"

using namespace dnf_composer::tools::math;

// ---------------------------------------------------------------------------
// SpectralConvolver2D correctness — direct (conv2d_separable_into) vs spectral
// (SpectralConvolver2D) applied to identical taps/field. This is the first
// test coverage the FFT path has ever had in this repo.
//
// Tolerance: NOT the 1e-12 used for direct-vs-direct comparisons elsewhere in
// this file's sibling (test_math.cpp) -- the r2c/c2r round trip's error scale
// is set by the *spectral* magnitudes, not the output magnitudes. On a
// 128x128 grid with O(1-10) field values the DC bin alone is
// sum(field) ~ 1.6e5, putting the FFTW round-trip noise floor near 1e-11
// before the kernel multiply. 1e-9 gives an order of magnitude of headroom.
// Every *structural* bug this test class exists to catch (reversed embedding,
// x/y transpose, wrong kR0, dropped normalization) produces errors the same
// order as the signal (O(1)-O(100)), not O(1e-9) -- so the exact tolerance
// value matters far less than that it is many orders below the signal scale
// and many orders above the 1e-4 golden-CSV gate (see validation_common.h).
// ---------------------------------------------------------------------------

namespace {
    constexpr double kSpectralAbsTolerance = 1e-9;
    constexpr double kSpectralRelTolerance = 1e-10;

    std::vector<double> ramp(int n)
    {
        std::vector<double> v(n);
        for (int i = 0; i < n; ++i) v[i] = std::sin(0.017 * i) + 0.01 * i;
        return v;
    }

    std::vector<double> gaussianTaps(int half, double sigma)
    {
        std::vector<int> r(2 * half + 1);
        std::iota(r.begin(), r.end(), -half);
        return gaussNorm(r, 0.0, sigma);
    }

    std::vector<double> directConvolve(
        const std::vector<double>& field,
        const std::vector<double>& taps_x, int kR0_x,
        const std::vector<double>& taps_y, int kR0_y,
        int sx, int sy)
    {
        const std::array<int, 2> rangeX{ kR0_x, static_cast<int>(taps_x.size()) - 1 - kR0_x };
        const std::array<int, 2> rangeY{ kR0_y, static_cast<int>(taps_y.size()) - 1 - kR0_y };
        const auto extX = createExtendedIndex(sx, rangeX);
        const auto extY = createExtendedIndex(sy, rangeY);

        Conv2dScratch<double> scratch;
        scratch.ensure(sx, sy, extX.size(), extY.size());
        std::vector<double> out(static_cast<size_t>(sx) * sy), tmp(static_cast<size_t>(sx) * sy);
        conv2d_separable_into(out, tmp, scratch, field, taps_x, taps_y, sx, sy, extX, extY);
        return out;
    }

    void expectSpectralMatchesDirect(const std::vector<double>& direct, const std::vector<double>& spectral)
    {
        ASSERT_EQ(direct.size(), spectral.size());
        double maxAbsDev = 0.0, maxAbsRef = 0.0;
        for (size_t i = 0; i < direct.size(); ++i)
        {
            maxAbsDev = std::max(maxAbsDev, std::abs(spectral[i] - direct[i]));
            maxAbsRef = std::max(maxAbsRef, std::abs(direct[i]));
        }
        EXPECT_LT(maxAbsDev, kSpectralAbsTolerance) << "max abs deviation = " << maxAbsDev;
        if (maxAbsRef > 0.0)
            EXPECT_LT(maxAbsDev / maxAbsRef, kSpectralRelTolerance)
                << "max relative deviation = " << (maxAbsDev / maxAbsRef);
    }

    // Single-term spectral convolution: init + setKernel + apply in one call,
    // for tests that don't need to inspect lifecycle behaviour directly.
    std::vector<double> spectralConvolveOneTerm(
        const std::vector<double>& field,
        const std::vector<double>& taps_x, int kR0_x,
        const std::vector<double>& taps_y, int kR0_y,
        int sx, int sy)
    {
        SpectralConvolver2D conv;
        conv.init(sx, sy);
        conv.setKernel(buildWrappedSeparableKernel2D(sx, sy,
            { SeparableKernelTerm2D{ taps_x, kR0_x, taps_y, kR0_y, +1.0 } }));
        std::vector<double> out(static_cast<size_t>(sx) * sy);
        conv.apply(field.data(), out.data());
        return out;
    }
}

TEST(SpectralConvolver2D, MatchesDirectCircularSymmetricGaussian)
{
    const int sx = 100, sy = 100;
    const auto taps = gaussianTaps(10, 4.0); // 21 symmetric taps, kR0==kR1==10
    const auto field = ramp(sx * sy);

    const auto direct = directConvolve(field, taps, 10, taps, 10, sx, sy);
    const auto spectral = spectralConvolveOneTerm(field, taps, 10, taps, 10, sx, sy);
    expectSpectralMatchesDirect(direct, spectral);
}

TEST(SpectralConvolver2D, MatchesDirectAsymmetricTaps)
{
    // Non-palindromic taps (as in AsymmetricGaussKernel2D's shifted kernel) --
    // this is the ONLY shape of test that can catch a reversed wrap embedding,
    // since every symmetric-tap case is invariant to that bug. See Finding B
    // in the plan: derived correct, this pins it against regression.
    const int sx = 100, sy = 128;
    std::vector<double> taps_x(15), taps_y(11);
    for (size_t i = 0; i < taps_x.size(); ++i) taps_x[i] = 1.0 + 0.3 * static_cast<double>(i); // strictly increasing
    for (size_t i = 0; i < taps_y.size(); ++i) taps_y[i] = 2.0 - 0.15 * static_cast<double>(i); // strictly decreasing
    const int kR0_x = 6, kR0_y = 4; // kR0 != kR1 on both axes
    const auto field = ramp(sx * sy);

    const auto direct = directConvolve(field, taps_x, kR0_x, taps_y, kR0_y, sx, sy);
    const auto spectral = spectralConvolveOneTerm(field, taps_x, kR0_x, taps_y, kR0_y, sx, sy);
    expectSpectralMatchesDirect(direct, spectral);
}

TEST(SpectralConvolver2D, MatchesDirectNonSquareGrid)
{
    const int sx = 128, sy = 96;
    const auto taps_x = gaussianTaps(8, 3.0);
    const auto taps_y = gaussianTaps(12, 5.0);
    const auto field = ramp(sx * sy);

    const auto direct = directConvolve(field, taps_x, 8, taps_y, 12, sx, sy);
    const auto spectral = spectralConvolveOneTerm(field, taps_x, 8, taps_y, 12, sx, sy);
    expectSpectralMatchesDirect(direct, spectral);
}

TEST(SpectralConvolver2D, MatchesDirectAsymmetricKernelRangeOnEvenGrid)
{
    // computeKernelRange's circular clamp on an even field size yields
    // kR0 != kR1 (e.g. size=100 -> {49,50}) even for a symmetric-tap kernel.
    const int sx = 100, sy = 100;
    const auto range = computeKernelRange(/*sigma=*/60.0, /*cutOfFactor=*/5, sx, /*circular=*/true);
    ASSERT_NE(range[0], range[1]) << "test precondition: expected an asymmetric clamp";
    std::vector<int> r(range[0] + range[1] + 1);
    std::iota(r.begin(), r.end(), -range[0]);
    const auto taps = gaussNorm(r, 0.0, 60.0);
    const auto field = ramp(sx * sy);

    const auto direct = directConvolve(field, taps, range[0], taps, range[0], sx, sy);
    const auto spectral = spectralConvolveOneTerm(field, taps, range[0], taps, range[0], sx, sy);
    expectSpectralMatchesDirect(direct, spectral);
}

TEST(SpectralConvolver2D, MatchesDirectFusedTwoTermDifference)
{
    // Mexican-hat-shaped: exc - inh, mirroring MexicanHatKernel2D::init().
    const int sx = 120, sy = 120;
    const auto excX = gaussianTaps(6, 2.5), excY = gaussianTaps(6, 2.5);
    const auto inhX = gaussianTaps(14, 6.0), inhY = gaussianTaps(14, 6.0);
    const auto field = ramp(sx * sy);

    // Direct reference: two separate direct convolutions, combined as exc-inh.
    const auto directExc = directConvolve(field, excX, 6, excY, 6, sx, sy);
    const auto directInh = directConvolve(field, inhX, 14, inhY, 14, sx, sy);
    std::vector<double> direct(directExc.size());
    for (size_t i = 0; i < direct.size(); ++i) direct[i] = directExc[i] - directInh[i];

    SpectralConvolver2D conv;
    conv.init(sx, sy);
    conv.setKernel(buildWrappedSeparableKernel2D(sx, sy,
        { SeparableKernelTerm2D{ excX, 6, excY, 6, +1.0 },
          SeparableKernelTerm2D{ inhX, 14, inhY, 14, -1.0 } }));
    std::vector<double> spectral(direct.size());
    conv.apply(field.data(), spectral.data());

    expectSpectralMatchesDirect(direct, spectral);
}

// ---------------------------------------------------------------------------
// SpectralConvolver2D lifecycle -- re-init, copy, move. Untested before this
// change; these pin the size-guard behaviour in init() (SETTLED #4) and the
// deep-copy semantics MexicanHatKernel2D::clone() has always relied on.
// ---------------------------------------------------------------------------

TEST(SpectralConvolver2DLifecycle, ReInitSameSizePreservesKernelSpectrum)
{
    const int sx = 100, sy = 100;
    const auto taps = gaussianTaps(8, 3.0);
    const auto field = ramp(sx * sy);

    SpectralConvolver2D conv;
    conv.init(sx, sy);
    conv.setKernel(buildWrappedSeparableKernel2D(sx, sy, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));
    std::vector<double> before(sx * sy);
    conv.apply(field.data(), before.data());

    // Re-init at the SAME size, deliberately WITHOUT calling setKernel again.
    conv.init(sx, sy);
    std::vector<double> after(sx * sy);
    conv.apply(field.data(), after.data());

    for (size_t i = 0; i < before.size(); ++i)
        EXPECT_DOUBLE_EQ(before[i], after[i]) << "kernel spectrum was lost on a same-size re-init at " << i;
}

TEST(SpectralConvolver2DLifecycle, ReInitDifferentSizeReplans)
{
    const auto taps = gaussianTaps(8, 3.0);

    SpectralConvolver2D conv;
    conv.init(100, 100);
    conv.setKernel(buildWrappedSeparableKernel2D(100, 100, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));

    conv.init(128, 112); // different size -> must actually replan
    conv.setKernel(buildWrappedSeparableKernel2D(128, 112, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));

    const auto field = ramp(128 * 112);
    std::vector<double> spectral(128 * 112);
    conv.apply(field.data(), spectral.data());
    const auto direct = directConvolve(field, taps, 8, taps, 8, 128, 112);
    expectSpectralMatchesDirect(direct, spectral);
}

TEST(SpectralConvolver2DLifecycle, CopyConstructedProducesSameOutput)
{
    const int sx = 100, sy = 100;
    const auto taps = gaussianTaps(8, 3.0);
    SpectralConvolver2D original;
    original.init(sx, sy);
    original.setKernel(buildWrappedSeparableKernel2D(sx, sy, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));

    SpectralConvolver2D copy(original);
    const auto field = ramp(sx * sy);
    std::vector<double> outOrig(sx * sy), outCopy(sx * sy);
    original.apply(field.data(), outOrig.data());
    copy.apply(field.data(), outCopy.data());
    for (size_t i = 0; i < outOrig.size(); ++i) EXPECT_DOUBLE_EQ(outOrig[i], outCopy[i]);
}

TEST(SpectralConvolver2DLifecycle, CopyAssignedOverExistingPlanProducesSameOutput)
{
    const int sx = 100, sy = 100;
    const auto taps = gaussianTaps(8, 3.0);
    SpectralConvolver2D source;
    source.init(sx, sy);
    source.setKernel(buildWrappedSeparableKernel2D(sx, sy, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));

    SpectralConvolver2D target;
    target.init(64, 64); // a different, pre-existing plan to be overwritten
    target = source;

    const auto field = ramp(sx * sy);
    std::vector<double> outSrc(sx * sy), outTgt(sx * sy);
    source.apply(field.data(), outSrc.data());
    target.apply(field.data(), outTgt.data());
    for (size_t i = 0; i < outSrc.size(); ++i) EXPECT_DOUBLE_EQ(outSrc[i], outTgt[i]);
}

TEST(SpectralConvolver2DLifecycle, MoveConstructedProducesSameOutputAndSourceDestructs)
{
    const int sx = 100, sy = 100;
    const auto taps = gaussianTaps(8, 3.0);
    const auto field = ramp(sx * sy);

    SpectralConvolver2D original;
    original.init(sx, sy);
    original.setKernel(buildWrappedSeparableKernel2D(sx, sy, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));
    std::vector<double> expected(sx * sy);
    original.apply(field.data(), expected.data());

    SpectralConvolver2D moved(std::move(original));
    std::vector<double> got(sx * sy);
    moved.apply(field.data(), got.data());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_DOUBLE_EQ(expected[i], got[i]);
    // original is now in the moved-from state; destructor must not double-free
    // (implicitly verified by the test process not crashing at scope exit).
}

TEST(SpectralConvolver2DLifecycle, MoveAssignedProducesSameOutput)
{
    const int sx = 100, sy = 100;
    const auto taps = gaussianTaps(8, 3.0);
    const auto field = ramp(sx * sy);

    SpectralConvolver2D original;
    original.init(sx, sy);
    original.setKernel(buildWrappedSeparableKernel2D(sx, sy, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));
    std::vector<double> expected(sx * sy);
    original.apply(field.data(), expected.data());

    SpectralConvolver2D target;
    target.init(64, 64);
    target = std::move(original);
    std::vector<double> got(sx * sy);
    target.apply(field.data(), got.data());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_DOUBLE_EQ(expected[i], got[i]);
}

TEST(SpectralConvolver2DLifecycle, CopyOfDefaultConstructedIsHarmless)
{
    SpectralConvolver2D empty;
    SpectralConvolver2D copy(empty); // must not crash; copyFrom early-returns
    SpectralConvolver2D assigned;
    assigned = empty;
    SUCCEED();
}

TEST(SpectralConvolver2DLifecycle, SelfCopyAssignIsSafe)
{
    const int sx = 100, sy = 100;
    const auto taps = gaussianTaps(8, 3.0);
    SpectralConvolver2D conv;
    conv.init(sx, sy);
    conv.setKernel(buildWrappedSeparableKernel2D(sx, sy, { SeparableKernelTerm2D{ taps, 8, taps, 8, 1.0 } }));

    const auto field = ramp(sx * sy);
    std::vector<double> before(sx * sy);
    conv.apply(field.data(), before.data());

    conv = conv; // NOLINT(clang-diagnostic-self-assign-overloaded)

    std::vector<double> after(sx * sy);
    conv.apply(field.data(), after.data());
    for (size_t i = 0; i < before.size(); ++i) EXPECT_DOUBLE_EQ(before[i], after[i]);
}

// ---------------------------------------------------------------------------
// shouldUseSpectral2D / ConvolutionMode override — the single dispatch rule
// every 2D convolution element uses.
// ---------------------------------------------------------------------------

TEST(SpectralDispatchRule, AutoRespectsTapThresholdAndAxisFloor)
{
    ScopedConvolutionMode mode(ConvolutionMode::Auto);
    // Below threshold, at/above floor -> direct.
    EXPECT_FALSE(shouldUseSpectral2D(true, 119, 100, 100));
    EXPECT_FALSE(shouldUseSpectral2D(true, 120, 100, 100)); // exactly at threshold: not > threshold
    // Above threshold, at/above floor -> spectral.
    EXPECT_TRUE(shouldUseSpectral2D(true, 121, 100, 100));
    // Above threshold, below floor on either axis -> direct.
    EXPECT_FALSE(shouldUseSpectral2D(true, 121, 99, 100));
    EXPECT_FALSE(shouldUseSpectral2D(true, 121, 100, 99));
    // Above threshold, at floor, non-circular -> direct (spectral undefined).
    EXPECT_FALSE(shouldUseSpectral2D(false, 121, 100, 100));
}

TEST(SpectralDispatchRule, ForceDirectDisablesAboveThreshold)
{
    ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
    EXPECT_FALSE(shouldUseSpectral2D(true, 1000, 200, 200));
}

TEST(SpectralDispatchRule, ForceSpectralEnablesBelowThresholdButNeverNonCircular)
{
    ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
    EXPECT_TRUE(shouldUseSpectral2D(true, 1, 10, 10));  // tiny grid, tiny kernel: still spectral
    EXPECT_FALSE(shouldUseSpectral2D(false, 1000, 200, 200)); // non-circular: never spectral
}

TEST(SpectralDispatchRule, ScopedOverrideRestoresPreviousMode)
{
    setConvolutionModeOverride(ConvolutionMode::ForceDirect);
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        EXPECT_EQ(convolutionModeOverride(), ConvolutionMode::ForceSpectral);
    }
    EXPECT_EQ(convolutionModeOverride(), ConvolutionMode::ForceDirect);
    setConvolutionModeOverride(ConvolutionMode::Auto); // leave global state clean for other tests
}

TEST(SpectralDispatchRule, ScopedOverrideRestoresOnExceptionUnwind)
{
    setConvolutionModeOverride(ConvolutionMode::ForceDirect);
    try
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        throw std::runtime_error("simulated failing EXPECT/ASSERT unwind");
    }
    catch (const std::runtime_error&)
    {
        // swallow; the point is that the destructor ran during unwind
    }
    EXPECT_EQ(convolutionModeOverride(), ConvolutionMode::ForceDirect);
    setConvolutionModeOverride(ConvolutionMode::Auto);
}
