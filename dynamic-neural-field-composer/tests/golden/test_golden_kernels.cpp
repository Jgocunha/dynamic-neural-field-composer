// ----------------------------------------------------------------------------
//  Golden test — lateral-interaction Kernels (1D + 2D)
//
//  For each kernel family (Gauss, MexicanHat, Oscillatory, AsymmetricGauss):
//    1. build a probe stimulus (GaussStimulus / GaussStimulus2D) with explicit
//       deterministic parameters and wire it as the kernel's input via a
//       Simulation (mirrors real usage: kernels convolve their input).
//    2. sim.init() + sim.step() once.
//    3. independently re-derive BOTH the kernel weight profile ("kernel"
//       component) and the convolution output ("output" component) in
//       reference/ref_kernels.h, using the SAME probe values (computed there
//       independently too, from the same GaussStimulus(2D) formula).
//    4. checkAgainstReference() for each component.
//
//  Sweep circular vs non-circular, several widths/amplitudes, normalized vs
//  not, and field size, for every kernel family in both 1D and 2D.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "elements/gauss_kernel.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/oscillatory_kernel.h"
#include "elements/oscillatory_kernel_2d.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/asymmetric_gauss_kernel_2d.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"
#include "simulation/simulation.h"

#include "../golden/golden_test_utils.h"
#include "../golden/reference/ref_kernels.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;
namespace r = dnf_composer::golden::ref;

namespace
{
    // Probe regime shared by every 1D kernel test: a fixed Gaussian bump.
    struct Probe1D { int size; double sigma; double position; double amplitude; bool circular; };

    std::vector<double> buildProbe1D(const Probe1D& p, Simulation& sim, const std::string& kernelName)
    {
        auto stim = std::make_shared<GaussStimulus>(
            ElementCommonParameters{ "probe", p.size },
            GaussStimulusParameters{ p.sigma, p.amplitude, p.position, p.circular, false });
        sim.addElement(stim);
        sim.createInteraction("probe", "output", kernelName);
        return r::gaussStimulus1D(p.size, p.sigma, p.position, p.amplitude, p.circular, false);
    }

    struct Probe2D { int size_x, size_y; double sigma, posX, posY, amplitude; bool circular; };

    std::vector<double> buildProbe2D(const Probe2D& p, Simulation& sim, const std::string& kernelName)
    {
        auto stim = std::make_shared<GaussStimulus2D>(
            ElementCommonParameters{ "probe", ElementDimensions(p.size_x, p.size_y, 1.0, 1.0) },
            GaussStimulus2DParameters{ p.sigma, p.amplitude, p.posX, p.posY, p.circular, false });
        sim.addElement(stim);
        sim.createInteraction("probe", "output", kernelName);
        return r::gaussStimulus2D(p.size_x, p.size_y, 1.0, 1.0, p.sigma, p.posX, p.posY, p.amplitude, p.circular, false);
    }
}

// ============================================================================
// GaussKernel (1D)
// ============================================================================

TEST(GoldenGaussKernel1D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int size; double width, amp, ampG; bool circular, normalized; Probe1D probe; };
    const std::vector<Regime> regimes = {
        { "gauss_kernel_1d_circular_s3_amp2",     100, 3.0, 2.0, -0.01, true,  true,  {100, 5.0, 50.0, 15.0, true} },
        { "gauss_kernel_1d_noncirc_s3_amp2",      100, 3.0, 2.0, -0.01, false, true,  {100, 5.0, 50.0, 15.0, false} },
        { "gauss_kernel_1d_circular_s8_unnorm",   100, 8.0, 1.5, 0.0,   true,  false, {100, 4.0, 30.0, 10.0, true} },
        { "gauss_kernel_1d_circular_edge",        100, 5.0, 2.0, -0.02, true,  true,  {100, 5.0, 99.0, 20.0, true} },
        { "gauss_kernel_1d_wide_field",           200, 10.0, 3.0, -0.01, true, true,  {200, 8.0, 120.0, 12.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("gk1d");
        auto kernel = std::make_shared<GaussKernel>(
            ElementCommonParameters{ reg.slug, reg.size },
            GaussKernelParameters{ reg.width, reg.amp, reg.ampG, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe1D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::gaussKernel1D(reg.size, reg.width, reg.amp, reg.ampG, reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}

// ============================================================================
// MexicanHatKernel (1D)
// ============================================================================

TEST(GoldenMexicanHatKernel1D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int size; double wExc, aExc, wInh, aInh, ampG; bool circular, normalized; Probe1D probe; };
    const std::vector<Regime> regimes = {
        { "mexican_hat_1d_circular_default", 100, 2.5, 11.0, 5.0, 15.0, -0.1, true,  true,  {100, 5.0, 50.0, 15.0, true} },
        { "mexican_hat_1d_noncirc_default",  100, 2.5, 11.0, 5.0, 15.0, -0.1, false, true,  {100, 5.0, 50.0, 15.0, false} },
        { "mexican_hat_1d_circular_unnorm",  100, 3.0, 8.0,  6.0, 10.0, 0.0,  true,  false, {100, 4.0, 30.0, 10.0, true} },
        { "mexican_hat_1d_circular_edge",    100, 2.0, 10.0, 4.0, 12.0, -0.05, true, true,  {100, 5.0, 99.0, 20.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("mhk1d");
        auto kernel = std::make_shared<MexicanHatKernel>(
            ElementCommonParameters{ reg.slug, reg.size },
            MexicanHatKernelParameters{ reg.wExc, reg.aExc, reg.wInh, reg.aInh, reg.ampG, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe1D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::mexicanHatKernel1D(reg.size, reg.wExc, reg.aExc, reg.wInh, reg.aInh, reg.ampG,
                                                       reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}

// ============================================================================
// OscillatoryKernel (1D)
// ============================================================================

TEST(GoldenOscillatoryKernel1D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int size; double amp, decay, zc, ampG; bool circular, normalized; Probe1D probe; };
    const std::vector<Regime> regimes = {
        { "oscillatory_1d_circular_default", 100, 1.0, 0.08, 0.3, -0.01, true,  false, {100, 5.0, 50.0, 15.0, true} },
        { "oscillatory_1d_noncirc_default",  100, 1.0, 0.08, 0.3, -0.01, false, false, {100, 5.0, 50.0, 15.0, false} },
        { "oscillatory_1d_circular_normalized", 100, 2.0, 0.05, 0.5, 0.0, true, true,  {100, 4.0, 30.0, 10.0, true} },
        { "oscillatory_1d_circular_fast_decay", 100, 1.5, 0.3,  0.2, -0.02, true, false, {100, 5.0, 99.0, 20.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("osc1d");
        auto kernel = std::make_shared<OscillatoryKernel>(
            ElementCommonParameters{ reg.slug, reg.size },
            OscillatoryKernelParameters{ reg.amp, reg.decay, reg.zc, reg.ampG, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe1D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::oscillatoryKernel1D(reg.size, reg.amp, reg.decay, reg.zc, reg.ampG,
                                                        reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}

// ============================================================================
// AsymmetricGaussKernel (1D)
// ============================================================================

TEST(GoldenAsymmetricGaussKernel1D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int size; double width, amp, ampG, shift; bool circular, normalized; Probe1D probe; };
    const std::vector<Regime> regimes = {
        { "asym_gauss_1d_circular_shift_pos", 100, 3.0, 3.0, 0.0, 2.0,  true,  true,  {100, 5.0, 50.0, 15.0, true} },
        { "asym_gauss_1d_circular_shift_neg", 100, 3.0, 3.0, 0.0, -2.0, true,  true,  {100, 5.0, 50.0, 15.0, true} },
        { "asym_gauss_1d_noncirc_shift",      100, 3.0, 3.0, 0.0, 1.5,  false, true,  {100, 5.0, 50.0, 15.0, false} },
        { "asym_gauss_1d_circular_unnorm",    100, 4.0, 2.0, 0.01, 3.0, true,  false, {100, 4.0, 30.0, 10.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("agk1d");
        auto kernel = std::make_shared<AsymmetricGaussKernel>(
            ElementCommonParameters{ reg.slug, reg.size },
            AsymmetricGaussKernelParameters{ reg.width, reg.amp, reg.ampG, reg.shift, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe1D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::asymmetricGaussKernel1D(reg.size, reg.width, reg.amp, reg.ampG, reg.shift,
                                                            reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}

// ============================================================================
// GaussKernel2D
// ============================================================================

TEST(GoldenGaussKernel2D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int sx, sy; double width, amp, ampG; bool circular, normalized; Probe2D probe; };
    const std::vector<Regime> regimes = {
        { "gauss_kernel_2d_circular_s2",   30, 30, 2.0, 2.0, -0.01, true,  true,  {30, 30, 3.0, 15.0, 15.0, 10.0, true} },
        { "gauss_kernel_2d_noncirc_s2",    30, 30, 2.0, 2.0, -0.01, false, true,  {30, 30, 3.0, 15.0, 15.0, 10.0, false} },
        { "gauss_kernel_2d_circular_unnorm", 25, 25, 3.0, 1.5, 0.0, true, false,  {25, 25, 2.5, 12.0, 12.0, 8.0, true} },
        { "gauss_kernel_2d_rect_field",    40, 20, 2.5, 2.0, -0.02, true, true,   {40, 20, 3.0, 20.0, 10.0, 15.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("gk2d");
        auto kernel = std::make_shared<GaussKernel2D>(
            ElementCommonParameters{ reg.slug, ElementDimensions(reg.sx, reg.sy, 1.0, 1.0) },
            GaussKernel2DParameters{ reg.width, reg.amp, reg.ampG, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe2D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::gaussKernel2D(reg.sx, reg.sy, reg.width, reg.amp, reg.ampG,
                                                  reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}

// ============================================================================
// MexicanHatKernel2D
// ============================================================================

TEST(GoldenMexicanHatKernel2D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int sx, sy; double wExc, aExc, wInh, aInh, ampG; bool circular, normalized; Probe2D probe; };
    const std::vector<Regime> regimes = {
        { "mexican_hat_2d_circular_default", 30, 30, 2.5, 11.0, 5.0, 15.0, -0.1, true,  true,  {30, 30, 3.0, 15.0, 15.0, 10.0, true} },
        { "mexican_hat_2d_noncirc_default",  30, 30, 2.5, 11.0, 5.0, 15.0, -0.1, false, true,  {30, 30, 3.0, 15.0, 15.0, 10.0, false} },
        { "mexican_hat_2d_circular_unnorm",  25, 25, 3.0, 8.0,  6.0, 10.0, 0.0,  true,  false, {25, 25, 2.5, 12.0, 12.0, 8.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("mhk2d");
        auto kernel = std::make_shared<MexicanHatKernel2D>(
            ElementCommonParameters{ reg.slug, ElementDimensions(reg.sx, reg.sy, 1.0, 1.0) },
            MexicanHatKernel2DParameters{ reg.wExc, reg.aExc, reg.wInh, reg.aInh, reg.ampG, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe2D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::mexicanHatKernel2D(reg.sx, reg.sy, reg.wExc, reg.aExc, reg.wInh, reg.aInh, reg.ampG,
                                                       reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}

// ============================================================================
// OscillatoryKernel2D
// ============================================================================

TEST(GoldenOscillatoryKernel2D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int sx, sy; double amp, decay, zc, ampG; bool circular, normalized; Probe2D probe; };
    const std::vector<Regime> regimes = {
        { "oscillatory_2d_circular_default", 30, 30, 1.0, 0.08, 0.3, -0.01, true,  false, {30, 30, 3.0, 15.0, 15.0, 10.0, true} },
        { "oscillatory_2d_noncirc_default",  30, 30, 1.0, 0.08, 0.3, -0.01, false, false, {30, 30, 3.0, 15.0, 15.0, 10.0, false} },
        { "oscillatory_2d_circular_normalized", 25, 25, 2.0, 0.05, 0.5, 0.0, true, true,  {25, 25, 2.5, 12.0, 12.0, 8.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("osc2d");
        auto kernel = std::make_shared<OscillatoryKernel2D>(
            ElementCommonParameters{ reg.slug, ElementDimensions(reg.sx, reg.sy, 1.0, 1.0) },
            OscillatoryKernel2DParameters{ reg.amp, reg.decay, reg.zc, reg.ampG, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe2D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::oscillatoryKernel2D(reg.sx, reg.sy, reg.amp, reg.decay, reg.zc, reg.ampG,
                                                        reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}

// ============================================================================
// AsymmetricGaussKernel2D
// ============================================================================

TEST(GoldenAsymmetricGaussKernel2D, WeightsAndOutputAcrossRegimes)
{
    struct Regime { std::string slug; int sx, sy; double width, amp, ampG, shiftX, shiftY; bool circular, normalized; Probe2D probe; };
    const std::vector<Regime> regimes = {
        { "asym_gauss_2d_circular_shift_xy", 30, 30, 3.0, 3.0, 0.0, 2.0, -1.5, true,  true,  {30, 30, 3.0, 15.0, 15.0, 10.0, true} },
        { "asym_gauss_2d_noncirc_shift_x",   30, 30, 3.0, 3.0, 0.0, 2.0, 0.0,  false, true,  {30, 30, 3.0, 15.0, 15.0, 10.0, false} },
        { "asym_gauss_2d_circular_unnorm",   25, 25, 4.0, 2.0, 0.01, 1.5, 1.0, true,  false, {25, 25, 2.5, 12.0, 12.0, 8.0, true} },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("agk2d");
        auto kernel = std::make_shared<AsymmetricGaussKernel2D>(
            ElementCommonParameters{ reg.slug, ElementDimensions(reg.sx, reg.sy, 1.0, 1.0) },
            AsymmetricGaussKernel2DParameters{ reg.width, reg.amp, reg.ampG, reg.shiftX, reg.shiftY, reg.circular, reg.normalized });
        sim.addElement(kernel);
        const auto probe = buildProbe2D(reg.probe, sim, reg.slug);
        sim.init();
        sim.step();

        const auto refResult = r::asymmetricGaussKernel2D(reg.sx, reg.sy, reg.width, reg.amp, reg.ampG,
                                                            reg.shiftX, reg.shiftY, reg.circular, reg.normalized, probe);

        g::checkAgainstReference(reg.slug + "_weights", kernel->getComponent("kernel"), refResult.weights);
        g::checkAgainstReference(reg.slug + "_output", kernel->getComponent("output"), refResult.output);
    }
}
