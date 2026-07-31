#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

#include "elements/gauss_kernel_2d.h"
#include "elements/asymmetric_gauss_kernel_2d.h"
#include "elements/oscillatory_kernel_2d.h"
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/correlated_normal_noise_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "simulation/simulation.h"
#include "tools/fft_convolution.h"
#include "tools/math.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
using namespace dnf_composer::tools::math;

// ---------------------------------------------------------------------------
// Direct-vs-spectral differential coverage for the hybrid convolution path,
// now wired into all five 2D convolution elements (previously only
// MexicanHatKernel2D). Every test here drives the kernel element OPEN LOOP
// from a fixed stimulus for a single step -- never through a NeuralField2D
// closed loop, where the per-step ~1e-9 difference between paths gets
// amplified by the abssigmoid attractor (see the >=100-axis-floor comment in
// mexican_hat_kernel_2d.cpp / fft_convolution.h). Closed-loop behavioural
// equivalence is the golden-fixture layer's job, at 1e-4.
// ---------------------------------------------------------------------------

namespace {
    ElementCommonParameters makeCP(const std::string& name, int x_max, int y_max)
    {
        return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
    }

    std::shared_ptr<GaussStimulus2D> makeStimulus(int x_max, int y_max)
    {
        auto stim = std::make_shared<GaussStimulus2D>(makeCP("stim", x_max, y_max),
            GaussStimulus2DParameters{ 5.0, 10.0, x_max / 2.0, y_max / 2.0, true, false });
        stim->init();
        return stim;
    }

    // Same tolerance rationale as tests/tools/test_fft_convolution.cpp: FFTW's
    // r2c/c2r round-trip error scale is set by spectral magnitudes, not output
    // magnitudes, so the 1e-12 used for direct-vs-direct comparisons elsewhere
    // does not transfer. 1e-9 gives an order of magnitude of headroom over the
    // measured worst case, and every structural bug this is meant to catch
    // (reversed embedding, x/y transpose, wrong kR0, dropped normalization,
    // amplitude applied twice) produces errors orders of magnitude larger.
    void expectNearlyEqual(const std::vector<double>& a, const std::vector<double>& b)
    {
        ASSERT_EQ(a.size(), b.size());
        double maxAbsDev = 0.0, maxAbsRef = 0.0;
        for (size_t i = 0; i < a.size(); ++i)
        {
            maxAbsDev = std::max(maxAbsDev, std::abs(a[i] - b[i]));
            maxAbsRef = std::max(maxAbsRef, std::abs(a[i]));
        }
        EXPECT_LT(maxAbsDev, 1e-9) << "max abs deviation = " << maxAbsDev;
        if (maxAbsRef > 0.0)
            EXPECT_LT(maxAbsDev / maxAbsRef, 1e-10) << "max relative deviation = " << (maxAbsDev / maxAbsRef);
    }

    void expectBitIdentical(const std::vector<double>& a, const std::vector<double>& b)
    {
        ASSERT_EQ(a.size(), b.size());
        for (size_t i = 0; i < a.size(); ++i)
            EXPECT_DOUBLE_EQ(a[i], b[i]) << "mismatch at " << i;
    }

    constexpr int N = 128; // >= kFFTMinAxisSize on both axes
}

// ---------------------------------------------------------------------------
// One differential test per element
// ---------------------------------------------------------------------------

TEST(SpectralDispatch2D, GaussKernelDirectMatchesSpectral)
{
    auto stim = makeStimulus(N, N);
    const GaussKernel2DParameters p{ 6.0, 2.0, 0.0, /*circular=*/true, /*normalized=*/true };

    std::shared_ptr<GaussKernel2D> direct, spectral;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        direct = std::make_shared<GaussKernel2D>(makeCP("gk_direct", N, N), p);
        direct->addInput(stim);
        direct->init();
    }
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        spectral = std::make_shared<GaussKernel2D>(makeCP("gk_spectral", N, N), p);
        spectral->addInput(stim);
        spectral->init();
    }
    direct->step(0.0, 1.0);
    spectral->step(0.0, 1.0);

    expectNearlyEqual(direct->getComponent("output"), spectral->getComponent("output"));
}

TEST(SpectralDispatch2D, AsymmetricGaussKernelDirectMatchesSpectral)
{
    // Non-palindromic taps (timeShift_x != timeShift_y != 0) -- the only shape
    // of test that can catch a reversed wrap-embedding sign convention, since
    // every symmetric-tap element is invariant to that class of bug.
    auto stim = makeStimulus(N, N);
    const AsymmetricGaussKernel2DParameters p{ 6.0, 2.0, 0.0, 1.5, -0.7, true, true };

    std::shared_ptr<AsymmetricGaussKernel2D> direct, spectral;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        direct = std::make_shared<AsymmetricGaussKernel2D>(makeCP("agk_direct", N, N), p);
        direct->addInput(stim);
        direct->init();
    }
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        spectral = std::make_shared<AsymmetricGaussKernel2D>(makeCP("agk_spectral", N, N), p);
        spectral->addInput(stim);
        spectral->init();
    }
    direct->step(0.0, 1.0);
    spectral->step(0.0, 1.0);

    expectNearlyEqual(direct->getComponent("output"), spectral->getComponent("output"));
}

TEST(SpectralDispatch2D, OscillatoryKernelDirectMatchesSpectral)
{
    auto stim = makeStimulus(N, N);
    const OscillatoryKernel2DParameters p{ 1.0, 0.08, 0.3, 0.0, true, true };

    std::shared_ptr<OscillatoryKernel2D> direct, spectral;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        direct = std::make_shared<OscillatoryKernel2D>(makeCP("ok_direct", N, N), p);
        direct->addInput(stim);
        direct->init();
    }
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        spectral = std::make_shared<OscillatoryKernel2D>(makeCP("ok_spectral", N, N), p);
        spectral->addInput(stim);
        spectral->init();
    }
    direct->step(0.0, 1.0);
    spectral->step(0.0, 1.0);

    expectNearlyEqual(direct->getComponent("output"), spectral->getComponent("output"));
}

TEST(SpectralDispatch2D, MexicanHatKernelDirectMatchesSpectral)
{
    auto stim = makeStimulus(N, N);
    const MexicanHatKernel2DParameters p{ 3.0, 10.0, 6.0, 12.0, 0.0, true, true };

    std::shared_ptr<MexicanHatKernel2D> direct, spectral;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        direct = std::make_shared<MexicanHatKernel2D>(makeCP("mhk_direct", N, N), p);
        direct->addInput(stim);
        direct->init();
    }
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        spectral = std::make_shared<MexicanHatKernel2D>(makeCP("mhk_spectral", N, N), p);
        spectral->addInput(stim);
        spectral->init();
    }
    direct->step(0.0, 1.0);
    spectral->step(0.0, 1.0);

    expectNearlyEqual(direct->getComponent("output"), spectral->getComponent("output"));
}

TEST(SpectralDispatch2D, CorrelatedNormalNoiseDirectMatchesSpectral)
{
    // Stochastic input (fresh white noise every step): re-seed the shared
    // thread_local generator before each run so both paths convolve the
    // IDENTICAL noise field. seedNormal is exactly this seam.
    const CorrelatedNormalNoise2DParameters p{ 1.0, 6.0, true };

    std::shared_ptr<CorrelatedNormalNoise2D> direct, spectral;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        direct = std::make_shared<CorrelatedNormalNoise2D>(makeCP("cnn_direct", N, N), p);
        direct->init();
    }
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        spectral = std::make_shared<CorrelatedNormalNoise2D>(makeCP("cnn_spectral", N, N), p);
        spectral->init();
    }

    seedNormal(2024);
    direct->step(0.0, 1.0);
    seedNormal(2024);
    spectral->step(0.0, 1.0);

    expectNearlyEqual(direct->getComponent("output"), spectral->getComponent("output"));
}

// ---------------------------------------------------------------------------
// Cross-cutting cases
// ---------------------------------------------------------------------------

TEST(SpectralDispatch2D, NonCircularUnderForceSpectralStaysDirect)
{
    // circular=false must keep the direct path regardless of the override --
    // spectral is only DEFINED for circular boundaries (see
    // shouldUseSpectral2D). Compared with EXPECT_DOUBLE_EQ (bit-identical),
    // proving the spectral branch was never taken, not just "close enough".
    auto stim = makeStimulus(N, N);
    const GaussKernel2DParameters p{ 6.0, 2.0, 0.0, /*circular=*/false, true };

    std::shared_ptr<GaussKernel2D> a, b;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        a = std::make_shared<GaussKernel2D>(makeCP("gk_a", N, N), p);
        a->addInput(stim);
        a->init();
    }
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        b = std::make_shared<GaussKernel2D>(makeCP("gk_b", N, N), p);
        b->addInput(stim);
        b->init();
    }
    a->step(0.0, 1.0);
    b->step(0.0, 1.0);

    expectBitIdentical(a->getComponent("output"), b->getComponent("output"));
}

TEST(SpectralDispatch2D, GlobalOffsetIdenticalOnBothPaths)
{
    auto stim = makeStimulus(N, N);
    const GaussKernel2DParameters p{ 6.0, 2.0, -0.05, true, true }; // amplitudeGlobal != 0

    std::shared_ptr<GaussKernel2D> direct, spectral;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        direct = std::make_shared<GaussKernel2D>(makeCP("gk_go_direct", N, N), p);
        direct->addInput(stim);
        direct->init();
    }
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        spectral = std::make_shared<GaussKernel2D>(makeCP("gk_go_spectral", N, N), p);
        spectral->addInput(stim);
        spectral->init();
    }
    direct->step(0.0, 1.0);
    spectral->step(0.0, 1.0);

    expectNearlyEqual(direct->getComponent("output"), spectral->getComponent("output"));
}

TEST(SpectralDispatch2D, ZeroAmplitudeYieldsZeroOnSpectralPath)
{
    const CorrelatedNormalNoise2DParameters p{ 0.0, 6.0, true };
    ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
    auto n = std::make_shared<CorrelatedNormalNoise2D>(makeCP("cnn_zero", N, N), p);
    n->init();
    n->step(0.0, 1.0);
    for (double v : n->getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// Runtime adaptation: width/parameter changes and grid-dimension changes must
// correctly re-derive useFFT_ and, when it stays true, re-run setKernel() even
// when SpectralConvolver2D::init()'s size guard skips re-planning.
// ---------------------------------------------------------------------------

TEST(SpectralDispatch2DRuntime, WidthIncreaseFlipsToSpectralAndMatchesDirectTwin)
{
    auto stim = makeStimulus(N, N);
    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk_grow_w", N, N), GaussKernel2DParameters{ 2.0, 2.0, 0.0, true, true });
    gk->addInput(stim);
    gk->init(); // narrow width: Auto picks direct

    const GaussKernel2DParameters wideParams{ 13.0, 2.0, 0.0, true, true };
    gk->setParameters(wideParams); // totalTaps now > 120 at N=128: Auto should flip to spectral
    gk->step(0.0, 1.0);
    const auto outAuto = gk->getComponent("output");

    // If Auto actually took the spectral branch, outAuto is BIT-IDENTICAL to
    // an independently-built ForceSpectral twin (same code path executed);
    // had it incorrectly stayed on direct, the two would still agree
    // numerically (see expectNearlyEqual's tolerance) but not bit-for-bit --
    // EXPECT_DOUBLE_EQ makes this assertion specific to "Auto took spectral".
    std::shared_ptr<GaussKernel2D> spectralTwin;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceSpectral);
        spectralTwin = std::make_shared<GaussKernel2D>(makeCP("gk_twin_spectral", N, N), wideParams);
        spectralTwin->addInput(stim);
        spectralTwin->init();
    }
    spectralTwin->step(0.0, 1.0);

    expectBitIdentical(outAuto, spectralTwin->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, WidthDecreaseFlipsBackToDirect)
{
    auto stim = makeStimulus(N, N);
    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk_shrink_w", N, N), GaussKernel2DParameters{ 13.0, 2.0, 0.0, true, true });
    gk->addInput(stim);
    gk->init(); // wide: Auto picks spectral

    const GaussKernel2DParameters narrowParams{ 2.0, 2.0, 0.0, true, true };
    gk->setParameters(narrowParams); // totalTaps drops well below 120: Auto should flip back to direct
    gk->step(0.0, 1.0);
    const auto outAuto = gk->getComponent("output");

    std::shared_ptr<GaussKernel2D> directTwin;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        directTwin = std::make_shared<GaussKernel2D>(makeCP("gk_twin_direct", N, N), narrowParams);
        directTwin->addInput(stim);
        directTwin->init();
    }
    directTwin->step(0.0, 1.0);

    expectBitIdentical(outAuto, directTwin->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, RepeatedSetParametersSameGridStaysCorrect)
{
    // Same grid every time -> SpectralConvolver2D::init() takes the
    // no-replan branch on every call after the first. This is the test that
    // fails if setKernel() is ever skipped alongside it: each width here
    // stays above the spectral threshold at N=128, so the taps genuinely
    // change step to step even though the geometry never does.
    auto stim = makeStimulus(N, N);
    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk_repeat", N, N), GaussKernel2DParameters{ 13.0, 2.0, 0.0, true, true });
    gk->addInput(stim);
    gk->init();

    for (const double w : { 13.0, 14.0, 13.5, 15.0, 13.0 })
    {
        SCOPED_TRACE(::testing::Message() << "width=" << w);

        const GaussKernel2DParameters p{ w, 2.0, 0.0, true, true };
        gk->setParameters(p);
        gk->step(0.0, 1.0);
        const auto out = gk->getComponent("output");

        // A fresh element at the same width always plans + setKernel from
        // cold, so it cannot be affected by the bug this test targets.
        auto fresh = std::make_shared<GaussKernel2D>(makeCP("gk_fresh", N, N), p);
        fresh->addInput(stim);
        fresh->init();
        fresh->step(0.0, 1.0);

        expectBitIdentical(out, fresh->getComponent("output"));
    }
}

TEST(SpectralDispatch2DRuntime, ChangeDimensionsGrowMatchesFreshElement)
{
    // Start below the spectral floor (50x50), grow across it to 128x128.
    auto stim50 = makeStimulus(50, 50);
    const GaussKernel2DParameters p{ 6.0, 2.0, 0.0, true, true };
    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk_grow_dim", 50, 50), p);
    gk->addInput(stim50);
    gk->init();

    gk->removeInputs();
    gk->changeDimensions(ElementDimensions(N, N, 1.0, 1.0)); // calls init() internally at the new size

    auto stimN = makeStimulus(N, N);
    gk->addInput(stimN);
    gk->step(0.0, 1.0);
    const auto outGrown = gk->getComponent("output");

    auto fresh = std::make_shared<GaussKernel2D>(makeCP("gk_fresh_grown", N, N), p);
    fresh->addInput(stimN);
    fresh->init();
    fresh->step(0.0, 1.0);

    expectBitIdentical(outGrown, fresh->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, ChangeDimensionsShrinkBelowFloorFallsBackToDirect)
{
    auto stimN = makeStimulus(N, N);
    const GaussKernel2DParameters p{ 13.0, 2.0, 0.0, true, true }; // wide: spectral at N=128
    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk_shrink_dim", N, N), p);
    gk->addInput(stimN);
    gk->init();

    gk->removeInputs();
    gk->changeDimensions(ElementDimensions(50, 50, 1.0, 1.0)); // below kFFTMinAxisSize

    auto stim50 = makeStimulus(50, 50);
    gk->addInput(stim50);
    gk->step(0.0, 1.0);
    const auto outAuto = gk->getComponent("output");

    std::shared_ptr<GaussKernel2D> directTwin;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        directTwin = std::make_shared<GaussKernel2D>(makeCP("gk_twin_small", 50, 50), p);
        directTwin->addInput(stim50);
        directTwin->init();
    }
    directTwin->step(0.0, 1.0);

    expectBitIdentical(outAuto, directTwin->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, SimulationChangeDimensionsOnSpectralKernel)
{
    // The full path a UI resize takes: Simulation::changeDimensions
    // disconnects, resizes (Element::changeDimensions -> init()), then the
    // caller reconnects. Exercises the dispatch rule through that pipeline
    // rather than by calling Element::changeDimensions directly.
    Simulation sim("spectral-resize-2d", 1.0, 0.0, 0.0);
    const GaussKernel2DParameters p{ 13.0, 2.0, 0.0, true, true }; // wide: spectral at 128x128

    sim.addElement(std::make_shared<GaussStimulus2D>(makeCP("stim", N, N),
        GaussStimulus2DParameters{ 5.0, 10.0, N / 2.0, N / 2.0, true, false }));
    sim.addElement(std::make_shared<GaussKernel2D>(makeCP("gk", N, N), p));
    sim.createInteraction("stim", "output", "gk");
    sim.init();
    sim.step();

    sim.changeDimensions("gk", ElementDimensions(50, 50, 1.0, 1.0)); // below the spectral floor
    sim.removeElement("stim");
    sim.addElement(std::make_shared<GaussStimulus2D>(makeCP("stim50", 50, 50),
        GaussStimulus2DParameters{ 5.0, 10.0, 25.0, 25.0, true, false }));
    sim.createInteraction("stim50", "output", "gk");
    sim.init();
    sim.step();

    const auto outAuto = sim.getElement("gk")->getComponent("output");

    auto stim50Twin = makeStimulus(50, 50);
    std::shared_ptr<GaussKernel2D> directTwin;
    {
        ScopedConvolutionMode mode(ConvolutionMode::ForceDirect);
        directTwin = std::make_shared<GaussKernel2D>(makeCP("gk_twin_sim", 50, 50), p);
        directTwin->addInput(stim50Twin);
        directTwin->init();
    }
    directTwin->step(0.0, 1.0);

    expectBitIdentical(outAuto, directTwin->getComponent("output"));
}

// ---------------------------------------------------------------------------
// Clone after spectral init: every element's clone() must produce a
// correctly-behaving copy, including its SpectralConvolver2D member (deep
// copy, not shared FFTW plan/buffer pointers -- see SpectralConvolver2D's
// class comment) and, since the fix in this change, a correctly-rebuilt
// input cache (see Element's copy ctor).
// ---------------------------------------------------------------------------

TEST(SpectralDispatch2DRuntime, CloneAfterSpectralInitProducesSameOutputGauss)
{
    auto stim = makeStimulus(N, N);
    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk_clone", N, N), GaussKernel2DParameters{ 13.0, 2.0, 0.0, true, true });
    gk->addInput(stim);
    gk->init();
    gk->step(0.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<GaussKernel2D>(gk->clone());
    ASSERT_NE(cloned, nullptr);
    cloned->step(0.0, 1.0);

    expectBitIdentical(gk->getComponent("output"), cloned->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, CloneAfterSpectralInitProducesSameOutputAsymmetricGauss)
{
    auto stim = makeStimulus(N, N);
    auto agk = std::make_shared<AsymmetricGaussKernel2D>(makeCP("agk_clone", N, N),
        AsymmetricGaussKernel2DParameters{ 6.0, 2.0, 0.0, 1.5, -0.7, true, true });
    agk->addInput(stim);
    agk->init();
    agk->step(0.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<AsymmetricGaussKernel2D>(agk->clone());
    ASSERT_NE(cloned, nullptr);
    cloned->step(0.0, 1.0);

    expectBitIdentical(agk->getComponent("output"), cloned->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, CloneAfterSpectralInitProducesSameOutputOscillatory)
{
    // The element whose clone() was fixed in this change (constructor form ->
    // copy-ctor form). At N=128 this exercises the fix at the spectral-path
    // size that motivated it, complementing the existing 20x20 direct-path
    // OscillatoryKernel2DClone.CloneProducesSameOutput test.
    auto stim = makeStimulus(N, N);
    auto ok = std::make_shared<OscillatoryKernel2D>(makeCP("ok_clone", N, N),
        OscillatoryKernel2DParameters{ 1.0, 0.08, 0.3, 0.0, true, true });
    ok->addInput(stim);
    ok->init();
    ok->step(0.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<OscillatoryKernel2D>(ok->clone());
    ASSERT_NE(cloned, nullptr);
    cloned->step(0.0, 1.0);

    expectBitIdentical(ok->getComponent("output"), cloned->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, CloneAfterSpectralInitProducesSameOutputMexicanHat)
{
    auto stim = makeStimulus(N, N);
    auto mhk = std::make_shared<MexicanHatKernel2D>(makeCP("mhk_clone", N, N),
        MexicanHatKernel2DParameters{ 3.0, 10.0, 6.0, 12.0, 0.0, true, true });
    mhk->addInput(stim);
    mhk->init();
    mhk->step(0.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<MexicanHatKernel2D>(mhk->clone());
    ASSERT_NE(cloned, nullptr);
    cloned->step(0.0, 1.0);

    expectBitIdentical(mhk->getComponent("output"), cloned->getComponent("output"));
}

TEST(SpectralDispatch2DRuntime, CloneAfterSpectralInitProducesSameOutputCorrelatedNoise)
{
    auto n = std::make_shared<CorrelatedNormalNoise2D>(makeCP("cnn_clone", N, N),
        CorrelatedNormalNoise2DParameters{ 1.0, 6.0, true });
    n->init();
    const auto cloned = std::dynamic_pointer_cast<CorrelatedNormalNoise2D>(n->clone());
    ASSERT_NE(cloned, nullptr);

    seedNormal(555);
    n->step(0.0, 1.0);
    seedNormal(555);
    cloned->step(0.0, 1.0);

    expectBitIdentical(n->getComponent("output"), cloned->getComponent("output"));
}
