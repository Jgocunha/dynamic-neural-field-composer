// ----------------------------------------------------------------------------
//  Golden test — MemoryTrace / MemoryTrace2D (leaky-integrator build/decay)
//
//  Analytic-equivalence: drive a production MemoryTrace directly from a
//  time-invariant GaussStimulus (spatially varying input naturally exercises
//  both the above-threshold "build" branch near the peak and the
//  below-threshold "decay" branch away from it), and additionally exercise a
//  genuine onset/offset (build-then-decay) run by toggling the stimulus
//  amplitude mid-trajectory. Compared against an independent re-derivation in
//  reference/ref_fields.h::memoryTraceTrajectory.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "elements/memory_trace.h"
#include "elements/memory_trace_2d.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"
#include "simulation/simulation.h"
#include "../golden/golden_test_utils.h"
#include "../golden/reference/ref_fields.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;

namespace
{
    struct Regime1D
    {
        std::string slug;
        int    size;
        double tauBuild, tauDecay, threshold, dt;
        int    steps;
        double stimAmplitude, stimSigma, stimPosition;
    };

    std::vector<Regime1D> regimes1D()
    {
        return {
            { "memory_trace_1d_default",         100, 100.0, 1000.0, 0.5, 1.0, 30, 10.0, 5.0, 50.0 },
            { "memory_trace_1d_fast_build",       100,  20.0, 1000.0, 0.5, 1.0, 30, 10.0, 5.0, 50.0 },
            { "memory_trace_1d_fast_decay",       100, 100.0,  100.0, 0.5, 1.0, 30, 10.0, 5.0, 50.0 },
            { "memory_trace_1d_high_threshold",   100, 100.0, 1000.0, 5.0, 1.0, 30, 10.0, 5.0, 50.0 },
            { "memory_trace_1d_low_threshold",    100, 100.0, 1000.0, 0.05, 1.0, 30,  3.0, 8.0, 50.0 },
            { "memory_trace_1d_dt_small",         100, 100.0, 1000.0, 0.5, 0.5, 40, 10.0, 5.0, 50.0 },
            { "memory_trace_1d_small_size",        30, 100.0, 1000.0, 0.5, 1.0, 25,  8.0, 3.0, 15.0 },
        };
    }
}

TEST(GoldenMemoryTrace, BuildAndDecayFromConstantStimulus1D)
{
    for (const auto& r : regimes1D())
    {
        auto sim = createSimulation(r.slug, r.dt);

        const ElementCommonParameters stimCp(r.slug + "_stim", r.size);
        const GaussStimulusParameters gsp(r.stimSigma, r.stimAmplitude, r.stimPosition, true, false);
        const auto stim = std::make_shared<GaussStimulus>(stimCp, gsp);
        sim->addElement(stim);

        const ElementCommonParameters traceCp(r.slug + "_trace", r.size);
        const MemoryTraceParameters mtp(r.tauBuild, r.tauDecay, r.threshold);
        const auto trace = std::make_shared<MemoryTrace>(traceCp, mtp);
        sim->addElement(trace);

        sim->createInteraction(stim->getUniqueName(), "output", trace->getUniqueName());
        sim->init();

        const std::vector<double> input = stim->getComponent("output"); // constant every step

        const g::Grid prodTraj = g::captureTrajectory(*sim, trace->getUniqueName(), "output", r.steps);

        const auto refTraj = g::ref::memoryTraceTrajectory(
            r.size, r.tauBuild, r.tauDecay, r.threshold, r.dt, r.steps,
            [&input](int) { return input; });

        g::checkAgainstReference(r.slug, prodTraj, refTraj);
    }
}

TEST(GoldenMemoryTrace, OnsetOffsetBuildThenDecay1D)
{
    // Genuine build-then-decay: stimulus present for the first half (trace
    // grows toward its input near the peak), then removed (trace decays with
    // tauDecay everywhere, since input drops below threshold).
    const std::string slug = "memory_trace_1d_onset_offset";
    constexpr int size = 100;
    constexpr double tauBuild = 50.0, tauDecay = 400.0, threshold = 0.5, dt = 1.0;
    constexpr int stepsOn = 15, stepsOff = 20;

    auto sim = createSimulation(slug, dt);
    const ElementCommonParameters stimCp(slug + "_stim", size);
    const GaussStimulusParameters gspOn(5.0, 12.0, 50.0, true, false);
    const auto stim = std::make_shared<GaussStimulus>(stimCp, gspOn);
    sim->addElement(stim);

    const ElementCommonParameters traceCp(slug + "_trace", size);
    const MemoryTraceParameters mtp(tauBuild, tauDecay, threshold);
    const auto trace = std::make_shared<MemoryTrace>(traceCp, mtp);
    sim->addElement(trace);

    sim->createInteraction(stim->getUniqueName(), "output", trace->getUniqueName());
    sim->init();

    const std::vector<double> inputOn = stim->getComponent("output");

    g::Grid prodTraj = g::captureTrajectory(*sim, trace->getUniqueName(), "output", stepsOn);

    const GaussStimulusParameters gspOff(5.0, 0.0, 50.0, true, false);
    stim->setParameters(gspOff);
    const std::vector<double> inputOff = stim->getComponent("output");

    const g::Grid prodTrajOff = g::captureTrajectory(*sim, trace->getUniqueName(), "output", stepsOff);
    prodTraj.insert(prodTraj.end(), prodTrajOff.begin(), prodTrajOff.end());

    const auto refTraj = g::ref::memoryTraceTrajectory(
        size, tauBuild, tauDecay, threshold, dt, stepsOn + stepsOff,
        [&](int s) { return (s < stepsOn) ? inputOn : inputOff; });

    g::checkAgainstReference(slug, prodTraj, refTraj);
}

TEST(GoldenMemoryTrace, BuildAndDecayFromConstantStimulus2D)
{
    struct Regime2D
    {
        std::string slug;
        int xMax, yMax;
        double tauBuild, tauDecay, threshold, dt;
        int steps;
        double stimAmplitude, stimSigma, posX, posY;
    };
    const std::vector<Regime2D> regimes = {
        { "memory_trace_2d_default",   30, 30, 100.0, 1000.0, 0.5, 1.0, 20, 10.0, 4.0, 15.0, 15.0 },
        { "memory_trace_2d_fast_build", 30, 30,  20.0, 1000.0, 0.5, 1.0, 20, 10.0, 4.0, 15.0, 15.0 },
    };

    for (const auto& r : regimes)
    {
        const ElementDimensions dims(r.xMax, r.yMax, 1.0, 1.0);

        auto sim = createSimulation(r.slug, r.dt);
        const ElementCommonParameters stimCp(r.slug + "_stim", dims);
        const GaussStimulus2DParameters gsp(r.stimSigma, r.stimAmplitude, r.posX, r.posY, true, false);
        const auto stim = std::make_shared<GaussStimulus2D>(stimCp, gsp);
        sim->addElement(stim);

        const ElementCommonParameters traceCp(r.slug + "_trace", dims);
        const MemoryTrace2DParameters mtp(r.tauBuild, r.tauDecay, r.threshold);
        const auto trace = std::make_shared<MemoryTrace2D>(traceCp, mtp);
        sim->addElement(trace);

        sim->createInteraction(stim->getUniqueName(), "output", trace->getUniqueName());
        sim->init();

        const std::vector<double> input = stim->getComponent("output");

        const g::Grid prodTraj = g::captureTrajectory(*sim, trace->getUniqueName(), "output", r.steps);

        const auto refTraj = g::ref::memoryTraceTrajectory(
            dims.size, r.tauBuild, r.tauDecay, r.threshold, r.dt, r.steps,
            [&input](int) { return input; });

        g::checkAgainstReference(r.slug, prodTraj, refTraj);
    }
}
