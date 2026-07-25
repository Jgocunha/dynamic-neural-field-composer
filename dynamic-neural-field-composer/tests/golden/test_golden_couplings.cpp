// ----------------------------------------------------------------------------
//  Golden test — FieldCoupling, GaussFieldCoupling
//
//  FieldCoupling: learning is disabled (default after init()), so the
//  algebraic contract under test is purely the matmul
//    output[i] = sum_j scalar * weights[j*outputSize + i] * input[j].
//  We wire a GaussStimulus probe as input, hand-write a deterministic
//  synthetic weight matrix directly into the "weights" component (bypassing
//  the learning rules, which are a separate — non-closed-form-in-scope —
//  concern), step once, and check output against an independent matmul.
//
//  GaussFieldCoupling: the "weights" component itself has a closed form
//  (a sum of Gaussian point-to-point projections — see GaussCoupling), which
//  we independently re-derive and check, in addition to the resulting
//  output = W * input.
//
//  NOTE on issue #54: reading src/elements/gauss_field_coupling.cpp as of
//  this worktree, the Gaussian-coupling weight computation is NOT
//  commented out — both the circular (gaussian_2d_periodic) and
//  non-circular (gaussian_2d) branches are live and populate "weights" from
//  parameters.couplings. The golden tests below therefore characterize the
//  ACTUAL current (functional) behaviour. If #54 refers to an earlier
//  revision, it may already be resolved — flagged to god either way.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <cmath>

#include "elements/field_coupling.h"
#include "elements/gauss_field_coupling.h"
#include "elements/gauss_stimulus.h"
#include "simulation/simulation.h"

#include "../golden/golden_test_utils.h"
#include "../golden/reference/ref_couplings.h"
#include "../golden/reference/ref_kernels.h" // reuse gaussStimulus1D (independent GaussStimulus re-derivation)

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;
namespace r = dnf_composer::golden::ref;

namespace
{
    // Deterministic (non-learned) synthetic weight pattern, indexed exactly as
    // production stores it: weights[j*outputSize + i], j = input idx, i = output idx.
    std::vector<double> syntheticWeights(int inputSize, int outputSize)
    {
        std::vector<double> w(static_cast<std::size_t>(inputSize) * outputSize);
        for (int j = 0; j < inputSize; ++j)
            for (int i = 0; i < outputSize; ++i)
                w[static_cast<std::size_t>(j) * outputSize + i] =
                    std::sin(0.37 * (j + 1)) * std::cos(0.21 * (i + 1)) + 0.05 * (j - i);
        return w;
    }
}

// ============================================================================
// FieldCoupling — output = scalar * W * input
// ============================================================================

TEST(GoldenFieldCoupling, MatmulAcrossRegimes)
{
    struct Regime { std::string slug; int inSize, outSize; double scalar; double sigma, position, amplitude; bool circular; };
    const std::vector<Regime> regimes = {
        { "field_coupling_30x20_scalar1",   30, 20, 1.0,  5.0, 15.0, 10.0, true },
        { "field_coupling_50x50_scalar1_5", 50, 50, 1.5,  6.0, 25.0, 12.0, true },
        { "field_coupling_15x40_scalar0_75", 15, 40, 0.75, 3.0, 7.0,  8.0,  false },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("fc");
        ElementDimensions inDim{ reg.inSize, 1.0 };
        FieldCouplingParameters fcp{ inDim, LearningRule::HEBB, reg.scalar, 0.01 };
        auto coupling = std::make_shared<FieldCoupling>(ElementCommonParameters{ reg.slug, reg.outSize }, fcp);
        sim.addElement(coupling);

        auto stim = std::make_shared<GaussStimulus>(
            ElementCommonParameters{ "probe", reg.inSize },
            GaussStimulusParameters{ reg.sigma, reg.amplitude, reg.position, reg.circular, false });
        sim.addElement(stim);
        sim.createInteraction("probe", "output", reg.slug);

        sim.init(); // learning disabled here, weights reset to zero

        // Overwrite with a deterministic synthetic weight matrix (learning stays off).
        const auto weights = syntheticWeights(reg.inSize, reg.outSize);
        *coupling->getComponentPtr("weights") = weights;

        sim.step();

        const auto probe = r::gaussStimulus1D(reg.inSize, reg.sigma, reg.position, reg.amplitude,
                                              reg.circular, false);
        const auto refOutput = r::fieldCouplingOutput(probe, weights, reg.outSize, reg.scalar);

        g::checkAgainstReference(reg.slug + "_output", coupling->getComponent("output"), refOutput);
    }
}

// ============================================================================
// GaussFieldCoupling — weights from Gaussian point-couplings, output = W * input
// ============================================================================

TEST(GoldenGaussFieldCoupling, WeightsAndOutputAcrossRegimes)
{
    struct Regime
    {
        std::string slug;
        int inSize, outSize;
        double d_x_in;
        bool normalized, circular;
        std::vector<r::CouplingSpec> couplings;
        double probeSigma, probePosition, probeAmplitude; bool probeCircular;
    };

    const std::vector<Regime> regimes = {
        {
            "gauss_field_coupling_single_normalized", 40, 40, 1.0, true, false,
            { { 20.0, 20.0, 5.0, 3.0 } },
            4.0, 20.0, 10.0, true
        },
        {
            "gauss_field_coupling_multi_circular", 40, 30, 1.0, true, true,
            { { 10.0, 8.0, 4.0, 2.5 }, { 30.0, 22.0, 3.0, 4.0 } },
            5.0, 15.0, 12.0, true
        },
        {
            "gauss_field_coupling_unnormalized", 25, 25, 1.0, false, false,
            { { 12.0, 12.0, 6.0, 2.0 } },
            3.0, 12.0, 9.0, false
        },
        {
            "gauss_field_coupling_scaled_dx", 50, 25, 2.0, true, false,
            { { 50.0, 25.0, 5.0, 4.0 } },
            4.0, 50.0, 15.0, false
        },
    };

    for (const auto& reg : regimes)
    {
        Simulation sim("gfc");

        ElementDimensions inputDim{ static_cast<int>(reg.inSize * reg.d_x_in), reg.d_x_in };
        GaussFieldCouplingParameters gfcp{ inputDim, reg.normalized, reg.circular, {} };
        for (const auto& c : reg.couplings)
            gfcp.addCoupling(GaussCoupling{ c.x_i, c.x_j, c.amplitude, c.width });

        auto coupling = std::make_shared<GaussFieldCoupling>(ElementCommonParameters{ reg.slug, reg.outSize }, gfcp);
        sim.addElement(coupling);

        auto stim = std::make_shared<GaussStimulus>(
            ElementCommonParameters{ "probe", inputDim },
            GaussStimulusParameters{ reg.probeSigma, reg.probeAmplitude, reg.probePosition, reg.probeCircular, false });
        sim.addElement(stim);
        sim.createInteraction("probe", "output", reg.slug);

        sim.init();
        sim.step();

        const auto refWeights = r::gaussFieldCouplingWeights(reg.inSize, reg.outSize, reg.d_x_in, 1.0,
                                                              reg.normalized, reg.circular, reg.couplings);
        g::checkAgainstReference(reg.slug + "_weights", coupling->getComponent("weights"), refWeights);

        const auto probe = r::gaussStimulus1D(reg.inSize, reg.probeSigma, reg.probePosition / reg.d_x_in,
                                              reg.probeAmplitude, reg.probeCircular, false);
        const auto refOutput = r::gaussFieldCouplingOutput(probe, refWeights, reg.outSize);
        g::checkAgainstReference(reg.slug + "_output", coupling->getComponent("output"), refOutput);
    }
}
