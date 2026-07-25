// ----------------------------------------------------------------------------
//  Golden tests — Stimuli family (Agent A)
//    GaussStimulus2D, BoostStimulus, BoostStimulus2D,
//    TimedGaussStimulus, TimedGaussStimulus2D
//
//  All ANALYTIC / deterministic — production output vs an independent
//  first-principles reference (tests/golden/reference/ref_stimuli.h), frozen
//  to CSV via golden::checkAgainstReference(). Sweeps sigma/width, position
//  (incl. boundary), circular vs non-circular, normalized vs not, and field
//  size, matching the exemplar's recipe (tests/golden/test_golden_gauss_stimulus.cpp).
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "elements/gauss_stimulus_2d.h"
#include "elements/boost_stimulus.h"
#include "elements/boost_stimulus_2d.h"
#include "elements/timed_gauss_stimulus.h"
#include "elements/timed_gauss_stimulus_2d.h"
#include "../golden/golden_test_utils.h"
#include "../golden/reference/ref_stimuli.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;

// ============================================================================
//  GaussStimulus2D
// ============================================================================
namespace
{
    struct GS2DRegime
    {
        std::string slug;
        int size_x, size_y;
        double width;
        double position_x, position_y;
        double amplitude;
        bool circular, normalized;
    };

    std::vector<GS2DRegime> gs2dRegimes()
    {
        return {
            { "gauss_stimulus_2d_circular_s5_p25_25",   50, 50, 5.0, 25.0, 25.0, 15.0, true,  false },
            { "gauss_stimulus_2d_noncirc_s5_p25_25",    50, 50, 5.0, 25.0, 25.0, 15.0, false, false },
            { "gauss_stimulus_2d_circular_edge_p0_0",   50, 50, 4.0,  0.0,  0.0, 10.0, true,  false },
            { "gauss_stimulus_2d_noncirc_edge_p49_49",  50, 50, 3.0, 49.0, 49.0, 12.0, false, false },
            { "gauss_stimulus_2d_normalized_s8",        50, 50, 8.0, 30.0, 20.0, 25.0, true,  true  },
            { "gauss_stimulus_2d_rect_sz60x40",         60, 40, 6.0, 40.0, 15.0, 18.0, true,  false },
        };
    }
}

TEST(GoldenGaussStimulus2D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : gs2dRegimes())
    {
        ElementCommonParameters cp{ r.slug, ElementDimensions(r.size_x, r.size_y, 1.0, 1.0) };
        GaussStimulus2DParameters gsp{ r.width, r.amplitude, r.position_x, r.position_y, r.circular, r.normalized };
        auto stim = std::make_shared<GaussStimulus2D>(cp, gsp);
        stim->init();

        const g::Row production = stim->getComponent("output");
        const g::Row reference = g::ref::gaussStimulus2DOutput(
            r.size_x, r.size_y, 1.0, 1.0,
            static_cast<double>(r.size_x), static_cast<double>(r.size_y),
            r.width, r.position_x, r.position_y, r.amplitude, r.circular, r.normalized);

        g::checkAgainstReference(r.slug, production, reference);
    }
}

// ============================================================================
//  BoostStimulus (1D)
// ============================================================================
namespace
{
    struct BoostRegime
    {
        std::string slug;
        int size;
        double amplitude;
        bool isActive;
    };

    std::vector<BoostRegime> boostRegimes()
    {
        return {
            { "boost_stimulus_1d_active_a5_sz100",    100, 5.0,  true  },
            { "boost_stimulus_1d_inactive_a5_sz100",  100, 5.0,  false },
            { "boost_stimulus_1d_active_neg_a3_sz50",  50, -3.0, true  },
            { "boost_stimulus_1d_active_a20_sz10",     10, 20.0, true  },
        };
    }
}

TEST(GoldenBoostStimulus1D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : boostRegimes())
    {
        ElementCommonParameters cp{ r.slug, r.size };
        BoostStimulusParameters bp{ r.amplitude, r.isActive };
        auto stim = std::make_shared<BoostStimulus>(cp, bp);
        stim->init();

        const g::Row production = stim->getComponent("output");
        const g::Row reference = g::ref::boostStimulusOutput(r.size, r.amplitude, r.isActive);

        g::checkAgainstReference(r.slug, production, reference);
    }
}

TEST(GoldenBoostStimulus1D, StepMatchesInit)
{
    // step() recomputes the same homogeneous fill as init(); verify the
    // algebraic identity holds after an explicit step() call too.
    ElementCommonParameters cp{ "boost_stimulus_1d_step_check", 30 };
    BoostStimulusParameters bp{ 7.5, true };
    auto stim = std::make_shared<BoostStimulus>(cp, bp);
    stim->init();
    stim->step(0.0, 1.0);

    const g::Row production = stim->getComponent("output");
    const g::Row reference = g::ref::boostStimulusOutput(30, 7.5, true);
    g::checkAgainstReference("boost_stimulus_1d_step_check", production, reference);
}

// ============================================================================
//  BoostStimulus2D
// ============================================================================
namespace
{
    struct Boost2DRegime
    {
        std::string slug;
        int size_x, size_y;
        double amplitude;
        bool isActive;
    };

    std::vector<Boost2DRegime> boost2dRegimes()
    {
        return {
            { "boost_stimulus_2d_active_a5_sz40x40",   40, 40, 5.0,  true  },
            { "boost_stimulus_2d_inactive_a5_sz40x40", 40, 40, 5.0,  false },
            { "boost_stimulus_2d_active_a12_sz30x20",  30, 20, 12.0, true  },
        };
    }
}

TEST(GoldenBoostStimulus2D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : boost2dRegimes())
    {
        ElementCommonParameters cp{ r.slug, ElementDimensions(r.size_x, r.size_y, 1.0, 1.0) };
        BoostStimulus2DParameters bp{ r.amplitude, r.isActive };
        auto stim = std::make_shared<BoostStimulus2D>(cp, bp);
        stim->init();

        const g::Row production = stim->getComponent("output");
        const g::Row reference = g::ref::boostStimulusOutput(r.size_x * r.size_y, r.amplitude, r.isActive);

        g::checkAgainstReference(r.slug, production, reference);
    }
}

// ============================================================================
//  TimedGaussStimulus (1D) — sample the output trajectory at several t values
//  spanning before / at-boundary / inside / after the configured on-windows.
// ============================================================================
namespace
{
    struct TimedRegime
    {
        std::string slug;
        int size;
        double width, position, amplitude;
        std::vector<std::pair<double, double>> onTimes;
        bool circular, normalized;
        std::vector<double> sampleTimes;
    };

    std::vector<TimedRegime> timedRegimes()
    {
        return {
            { "timed_gauss_stimulus_1d_circular_s5_p50", 100, 5.0, 50.0, 15.0,
              { {2.0, 5.0} }, true, false, { 0.0, 1.999, 2.0, 3.5, 5.0, 5.001, 8.0 } },
            { "timed_gauss_stimulus_1d_noncirc_edge_p1", 100, 4.0, 1.0, 12.0,
              { {1.0, 3.0}, {6.0, 7.0} }, false, false, { 0.5, 1.0, 2.0, 3.0, 4.5, 6.0, 6.5, 7.0, 9.0 } },
            { "timed_gauss_stimulus_1d_normalized_s8_p50", 100, 8.0, 50.0, 30.0,
              { {0.0, 10.0} }, true, true, { 0.0, 5.0, 10.0, 10.001 } },
        };
    }
}

TEST(GoldenTimedGaussStimulus1D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : timedRegimes())
    {
        ElementCommonParameters cp{ r.slug, r.size };
        TimedGaussStimulusParameters tp{ r.width, r.amplitude, r.position, r.onTimes, r.circular, r.normalized };
        auto stim = std::make_shared<TimedGaussStimulus>(cp, tp);
        stim->init();

        const g::Row pattern = g::ref::timedGaussStimulusPattern(
            r.size, r.width, r.position /* d_x == 1 */, r.amplitude, r.circular, r.normalized);

        g::Grid production, reference;
        for (double t : r.sampleTimes)
        {
            stim->step(t, 1.0);
            production.push_back(stim->getComponent("output"));
            reference.push_back(g::ref::timedGaussStimulusOutputAt(t, r.onTimes, pattern));
        }

        g::checkAgainstReference(r.slug, production, reference);
    }
}

// ============================================================================
//  TimedGaussStimulus2D
// ============================================================================
namespace
{
    struct Timed2DRegime
    {
        std::string slug;
        int size_x, size_y;
        double width, position_x, position_y, amplitude;
        std::vector<std::pair<double, double>> onTimes;
        bool circular, normalized;
        std::vector<double> sampleTimes;
    };

    std::vector<Timed2DRegime> timed2dRegimes()
    {
        return {
            { "timed_gauss_stimulus_2d_circular_s5_p25_25", 50, 50, 5.0, 25.0, 25.0, 15.0,
              { {1.0, 4.0} }, true, false, { 0.0, 0.999, 1.0, 2.5, 4.0, 4.001 } },
            { "timed_gauss_stimulus_2d_noncirc_edge_p0_0", 50, 50, 3.0, 0.0, 0.0, 10.0,
              { {0.0, 2.0}, {5.0, 6.0} }, false, false, { 0.0, 1.0, 2.0, 3.5, 5.0, 6.0, 7.0 } },
        };
    }
}

TEST(GoldenTimedGaussStimulus2D, AlgebraicEquivalenceAcrossRegimes)
{
    for (const auto& r : timed2dRegimes())
    {
        ElementCommonParameters cp{ r.slug, ElementDimensions(r.size_x, r.size_y, 1.0, 1.0) };
        TimedGaussStimulus2DParameters tp{ r.width, r.amplitude, r.position_x, r.position_y,
                                           r.onTimes, r.circular, r.normalized };
        auto stim = std::make_shared<TimedGaussStimulus2D>(cp, tp);
        stim->init();

        const g::Row pattern = g::ref::timedGaussStimulus2DPattern(
            r.size_x, r.size_y, 1.0, 1.0, static_cast<double>(r.size_x), static_cast<double>(r.size_y),
            r.width, r.position_x, r.position_y, r.amplitude, r.circular, r.normalized);

        g::Grid production, reference;
        for (double t : r.sampleTimes)
        {
            stim->step(t, 1.0);
            production.push_back(stim->getComponent("output"));
            reference.push_back(g::ref::timedGaussStimulusOutputAt(t, r.onTimes, pattern));
        }

        g::checkAgainstReference(r.slug, production, reference);
    }
}
