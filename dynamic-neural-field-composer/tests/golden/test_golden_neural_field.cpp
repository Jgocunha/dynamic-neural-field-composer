// ----------------------------------------------------------------------------
//  Golden test — NeuralField / NeuralField2D (Amari update, multi-step trajectory)
//
//  Analytic-equivalence: build a production NeuralField (optionally driven by a
//  constant GaussStimulus, whose output is time-invariant — GaussStimulus::step()
//  is a no-op), capture a multi-step trajectory of "activation" and "output" via
//  captureTrajectory(), and compare against an independently re-derived Euler
//  integration of the Amari equation in reference/ref_fields.h.
//
//  Sweeps: tau, resting level, dt, activation function (Sigmoid/AbsSigmoid/
//  Heaviside), field size, presence/amplitude of external drive, stimulus
//  onset+offset (decay), both 1D and 2D.
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
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
    // ---- 1D ----------------------------------------------------------------
    struct Regime1D
    {
        std::string slug;
        int    size;
        double tau;
        double restingLevel;
        double dt;
        int    steps;
        // activation function selector
        enum class Act { Sigmoid, AbsSigmoid, Heaviside } act;
        double actP1, actP2; // (x_shift, steepness/beta) — actP2 unused for Heaviside
        // external drive (constant GaussStimulus); amplitude == 0 => no stimulus at all
        double stimAmplitude;
        double stimSigma;
        double stimPosition;
        bool   stimCircular;
    };

    std::vector<Regime1D> regimes1D()
    {
        return {
            { "neural_field_1d_sigmoid_suprathreshold", 100, 25.0, -5.0, 1.0, 30, Regime1D::Act::Sigmoid, 0.0, 10.0,  8.0, 5.0, 50.0, true },
            { "neural_field_1d_sigmoid_subthreshold",   100, 25.0, -5.0, 1.0, 30, Regime1D::Act::Sigmoid, 0.0, 10.0,  2.0, 5.0, 50.0, true },
            { "neural_field_1d_tau_fast",               100, 10.0, -5.0, 1.0, 30, Regime1D::Act::Sigmoid, 0.0, 10.0,  8.0, 5.0, 50.0, true },
            { "neural_field_1d_tau_slow",                100, 60.0, -5.0, 1.0, 40, Regime1D::Act::Sigmoid, 0.0, 10.0,  8.0, 5.0, 50.0, true },
            { "neural_field_1d_dt_small",                100, 25.0, -5.0, 0.5, 40, Regime1D::Act::Sigmoid, 0.0, 10.0,  8.0, 5.0, 50.0, true },
            { "neural_field_1d_dt_large",                100, 25.0, -5.0, 2.0, 20, Regime1D::Act::Sigmoid, 0.0, 10.0,  8.0, 5.0, 50.0, true },
            { "neural_field_1d_resting_positive",        100, 25.0,  2.0, 1.0, 30, Regime1D::Act::Sigmoid, 0.0, 10.0,  5.0, 5.0, 50.0, true },
            { "neural_field_1d_resting_very_negative",   100, 25.0,-10.0, 1.0, 30, Regime1D::Act::Sigmoid, 0.0, 10.0, 10.0, 5.0, 50.0, true },
            { "neural_field_1d_abssigmoid",              100, 25.0, -5.0, 1.0, 30, Regime1D::Act::AbsSigmoid, 0.0, 20.0, 8.0, 5.0, 50.0, true },
            { "neural_field_1d_heaviside",                100, 25.0, -5.0, 1.0, 30, Regime1D::Act::Heaviside, 0.0, 0.0,  8.0, 5.0, 50.0, true },
            { "neural_field_1d_small_size",                20, 25.0, -5.0, 1.0, 25, Regime1D::Act::Sigmoid, 0.0, 10.0,  6.0, 3.0, 10.0, true },
            { "neural_field_1d_large_size",               200, 25.0, -5.0, 1.0, 25, Regime1D::Act::Sigmoid, 0.0, 10.0,  8.0, 5.0, 100.0, true },
            { "neural_field_1d_noncircular_edge",         100, 25.0, -5.0, 1.0, 25, Regime1D::Act::Sigmoid, 0.0, 10.0,  8.0, 5.0, 5.0, false },
            { "neural_field_1d_no_external_input",        100, 25.0, -5.0, 1.0, 15, Regime1D::Act::Sigmoid, 0.0, 10.0,  0.0, 5.0, 50.0, true },
        };
    }

    g::ref::ActivationFn makeRefActivation(const Regime1D& r)
    {
        switch (r.act)
        {
        case Regime1D::Act::Sigmoid:    return g::ref::makeSigmoidFn(r.actP1, r.actP2);
        case Regime1D::Act::AbsSigmoid: return g::ref::makeAbsSigmoidFn(r.actP1, r.actP2);
        case Regime1D::Act::Heaviside:  return g::ref::makeHeavisideFn(r.actP1);
        }
        return g::ref::makeSigmoidFn(0.0, 10.0);
    }

    std::unique_ptr<ActivationFunction> makeProdActivation(const Regime1D& r)
    {
        switch (r.act)
        {
        case Regime1D::Act::Sigmoid:    return std::make_unique<SigmoidFunction>(r.actP1, r.actP2);
        case Regime1D::Act::AbsSigmoid: return std::make_unique<AbsSigmoidFunction>(r.actP1, r.actP2);
        case Regime1D::Act::Heaviside:  return std::make_unique<HeavisideFunction>(r.actP1);
        }
        return std::make_unique<SigmoidFunction>(0.0, 10.0);
    }

    // Capture "activation" AND "output" of one element from a single simulation
    // run (captureTrajectory only tracks one component and advances the sim, so
    // two calls on the same sim would desynchronise the two component streams).
    std::pair<g::Grid, g::Grid> captureActivationAndOutput(Simulation& sim, const std::string& elementName, int steps)
    {
        g::Grid act, out;
        act.reserve(static_cast<std::size_t>(steps));
        out.reserve(static_cast<std::size_t>(steps));
        for (int i = 0; i < steps; ++i)
        {
            sim.step();
            const auto elem = sim.getElement(elementName);
            act.push_back(elem->getComponent("activation"));
            out.push_back(elem->getComponent("output"));
        }
        return { act, out };
    }
}

TEST(GoldenNeuralField, AmariTrajectory1DAcrossRegimes)
{
    for (const auto& r : regimes1D())
    {
        auto sim = createSimulation(r.slug, r.dt);

        const ElementCommonParameters fieldCp(r.slug + "_field", r.size);
        const auto prodAct = makeProdActivation(r);
        const NeuralFieldParameters nfp(r.tau, r.restingLevel, *prodAct);
        const auto field = std::make_shared<NeuralField>(fieldCp, nfp);
        sim->addElement(field);

        std::vector<double> externalInput(static_cast<std::size_t>(r.size), 0.0);
        if (r.stimAmplitude != 0.0)
        {
            const ElementCommonParameters stimCp(r.slug + "_stim", r.size);
            const GaussStimulusParameters gsp(r.stimSigma, r.stimAmplitude, r.stimPosition, r.stimCircular, false);
            const auto stim = std::make_shared<GaussStimulus>(stimCp, gsp);
            sim->addElement(stim);
            sim->createInteraction(stim->getUniqueName(), "output", field->getUniqueName());
            externalInput = stim->getComponent("output"); // constant for all steps (step() is a no-op)
        }

        sim->init();

        const auto [prodAct_, prodOut_] = captureActivationAndOutput(*sim, field->getUniqueName(), r.steps);

        const g::ref::ActivationFn refAct = makeRefActivation(r);
        const auto refTraj = g::ref::amariFieldTrajectory(
            r.size, r.tau, r.restingLevel, r.dt, refAct, r.steps,
            [&externalInput](int) { return externalInput; });

        g::checkAgainstReference(r.slug + "_activation", prodAct_, refTraj.activation);
        g::checkAgainstReference(r.slug + "_output", prodOut_, refTraj.output);
    }
}

TEST(GoldenNeuralField, AmariTrajectory1DStimulusOnsetOffset)
{
    // Non-trivial decay test: drive the field for the first half of the run,
    // then remove the stimulus (setParameters(amplitude=0) -> re-init) and
    // watch activation relax back toward the resting level.
    const std::string slug = "neural_field_1d_onset_offset";
    constexpr int size = 100;
    constexpr double tau = 25.0, h = -5.0, dt = 1.0;
    constexpr int stepsOn = 15, stepsOff = 15;

    auto sim = createSimulation(slug, dt);
    const ElementCommonParameters fieldCp(slug + "_field", size);
    const SigmoidFunction sf(0.0, 10.0);
    const NeuralFieldParameters nfp(tau, h, sf);
    const auto field = std::make_shared<NeuralField>(fieldCp, nfp);
    sim->addElement(field);

    const ElementCommonParameters stimCp(slug + "_stim", size);
    const GaussStimulusParameters gspOn(5.0, 9.0, 50.0, true, false);
    const auto stim = std::make_shared<GaussStimulus>(stimCp, gspOn);
    sim->addElement(stim);
    sim->createInteraction(stim->getUniqueName(), "output", field->getUniqueName());
    sim->init();

    const std::vector<double> inputOn = stim->getComponent("output");

    g::Grid prodTraj = g::captureTrajectory(*sim, field->getUniqueName(), "activation", stepsOn);

    const GaussStimulusParameters gspOff(5.0, 0.0, 50.0, true, false);
    stim->setParameters(gspOff); // amplitude -> 0, output re-initialised to all-zero
    const std::vector<double> inputOff = stim->getComponent("output");

    const g::Grid prodTrajOff = g::captureTrajectory(*sim, field->getUniqueName(), "activation", stepsOff);
    prodTraj.insert(prodTraj.end(), prodTrajOff.begin(), prodTrajOff.end());

    const g::ref::ActivationFn refAct = g::ref::makeSigmoidFn(0.0, 10.0);
    const auto refTraj = g::ref::amariFieldTrajectory(
        size, tau, h, dt, refAct, stepsOn + stepsOff,
        [&](int s) { return (s < stepsOn) ? inputOn : inputOff; });

    g::checkAgainstReference(slug, prodTraj, refTraj.activation);
}

namespace
{
    // ---- 2D ------------------------------------------------------------
    struct Regime2D
    {
        std::string slug;
        int    xMax, yMax;
        double tau, restingLevel, dt;
        int    steps;
        bool   absSigmoid;
        double stimAmplitude, stimSigma, stimPosX, stimPosY;
        bool   stimCircular;
    };

    std::vector<Regime2D> regimes2D()
    {
        return {
            { "neural_field_2d_sigmoid_suprathreshold", 30, 30, 25.0, -5.0, 1.0, 20, false, 8.0, 4.0, 15.0, 15.0, true },
            { "neural_field_2d_sigmoid_subthreshold",   30, 30, 25.0, -5.0, 1.0, 20, false, 2.0, 4.0, 15.0, 15.0, true },
            { "neural_field_2d_tau_fast",               30, 30, 12.0, -5.0, 1.0, 20, false, 8.0, 4.0, 15.0, 15.0, true },
            { "neural_field_2d_abssigmoid",             30, 30, 25.0, -5.0, 1.0, 20, true,  8.0, 4.0, 15.0, 15.0, true },
            { "neural_field_2d_noncircular_edge",       30, 30, 25.0, -5.0, 1.0, 15, false, 8.0, 4.0, 3.0, 3.0, false },
        };
    }
}

TEST(GoldenNeuralField, AmariTrajectory2DAcrossRegimes)
{
    for (const auto& r : regimes2D())
    {
        const ElementDimensions dims(r.xMax, r.yMax, 1.0, 1.0);

        auto sim = createSimulation(r.slug, r.dt);
        const ElementCommonParameters fieldCp(r.slug + "_field", dims);
        const SigmoidFunction sigmoidFn(0.0, 10.0);
        const AbsSigmoidFunction absSigmoidFn(0.0, 20.0);
        const NeuralField2DParameters nfp = r.absSigmoid
            ? NeuralField2DParameters(r.tau, r.restingLevel, absSigmoidFn)
            : NeuralField2DParameters(r.tau, r.restingLevel, sigmoidFn);
        const auto field = std::make_shared<NeuralField2D>(fieldCp, nfp);
        sim->addElement(field);

        const ElementCommonParameters stimCp(r.slug + "_stim", dims);
        const GaussStimulus2DParameters gsp(r.stimSigma, r.stimAmplitude, r.stimPosX, r.stimPosY, r.stimCircular, false);
        const auto stim = std::make_shared<GaussStimulus2D>(stimCp, gsp);
        sim->addElement(stim);
        sim->createInteraction(stim->getUniqueName(), "output", field->getUniqueName());
        sim->init();

        const std::vector<double> externalInput = stim->getComponent("output");

        const g::Grid prodActTraj = g::captureTrajectory(*sim, field->getUniqueName(), "activation", r.steps);

        const int size = dims.size;
        const g::ref::ActivationFn refAct = r.absSigmoid
            ? g::ref::makeAbsSigmoidFn(0.0, 20.0)
            : g::ref::makeSigmoidFn(0.0, 10.0);
        const auto refTraj = g::ref::amariFieldTrajectory(
            size, r.tau, r.restingLevel, r.dt, refAct, r.steps,
            [&externalInput](int) { return externalInput; });

        g::checkAgainstReference(r.slug + "_activation", prodActTraj, refTraj.activation);
    }
}
