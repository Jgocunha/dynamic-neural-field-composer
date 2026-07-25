// ----------------------------------------------------------------------------
//  Golden test — GaussStimulus (1D)   [EXEMPLAR — the pattern agents replicate]
//
//  Demonstrates the full golden-test recipe:
//    1. build the production element with an explicit, deterministic parameter set
//    2. compute the SAME output from the independent golden reference
//    3. checkAgainstReference() → asserts algebraic equivalence AND freezes the
//       committed golden CSV under tests/golden/data/
//
//  Sweep several parameter regimes (sigma, position incl. boundary, circular vs
//  not, normalized vs not, field size) so the net is broad, not a single point.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "elements/gauss_stimulus.h"
#include "../golden/golden_test_utils.h"
#include "../golden/reference/ref_gauss_stimulus.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;

namespace
{
    // One regime of the parameter sweep.
    struct Regime
    {
        std::string slug;
        int         size;
        double      sigma;
        double      position;  // spatial coordinate (d_x == 1 here → == samples)
        double      amplitude;
        bool        circular;
        bool        normalized;
    };

    std::vector<Regime> regimes()
    {
        return {
            { "gauss_stimulus_1d_circular_s5_p50",     100, 5.0,  50.0, 15.0, true,  false },
            { "gauss_stimulus_1d_noncirc_s5_p50",      100, 5.0,  50.0, 15.0, false, false },
            { "gauss_stimulus_1d_circular_s3_p10",     100, 3.0,  10.0, 10.0, true,  false },
            { "gauss_stimulus_1d_circular_edge_p99",   100, 5.0,  99.0, 20.0, true,  false },
            { "gauss_stimulus_1d_noncirc_edge_p1",     100, 4.0,   1.0, 12.0, false, false },
            { "gauss_stimulus_1d_normalized_s8_p50",   100, 8.0,  50.0, 30.0, true,  true  },
            { "gauss_stimulus_1d_wide_s20_sz200_p120", 200, 20.0, 120.0, 8.0, true,  false },
        };
    }
}

TEST(GoldenGaussStimulus1D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : regimes())
    {
        ElementCommonParameters cp{ r.slug, r.size };            // d_x defaults to 1.0
        GaussStimulusParameters gsp{ r.sigma, r.amplitude, r.position, r.circular, r.normalized };
        auto stim = std::make_shared<GaussStimulus>(cp, gsp);
        stim->init();

        const g::Row production = stim->getComponent("output");
        const g::Row reference  = g::ref::gaussStimulusOutput(
            r.size, r.sigma, r.position /* d_x == 1 */, r.amplitude, r.circular, r.normalized);

        g::checkAgainstReference(r.slug, production, reference);
    }
}
