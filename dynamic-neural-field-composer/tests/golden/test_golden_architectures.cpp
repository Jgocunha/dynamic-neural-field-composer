// ----------------------------------------------------------------------------
//  Golden test — composed architectures (characterization; no closed form)
//
//  These build real production simulations, run them deterministically (no
//  noise elements — determinism is required for a frozen golden CSV), and
//  freeze a multi-step trajectory with checkCharacterization(). Covers:
//    A. field + Gauss stimulus, sub- and supra-threshold
//    B. field + self-excitation Gauss kernel -> sustained ("stable") bump
//    C. field + Mexican-hat kernel -> selection between two competing stimuli
//    D. two coupled fields via GaussFieldCoupling
//    E. memory-trace formation from a field driven by a stimulus (onset/offset)
// ----------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/gauss_field_coupling.h"
#include "elements/memory_trace.h"
#include "simulation/simulation.h"
#include "../golden/golden_test_utils.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace g = dnf_composer::golden;

namespace
{
    // Capture several (elementName, component) streams from ONE simulation run,
    // concatenating each step's vectors into a single CSV row (order matches the
    // `targets` argument). Used where a single trajectory must show two coupled
    // elements evolving together (captureTrajectory only tracks one).
    g::Grid captureMulti(Simulation& sim,
                         const std::vector<std::pair<std::string, std::string>>& targets,
                         int steps)
    {
        g::Grid traj;
        traj.reserve(static_cast<std::size_t>(steps));
        for (int i = 0; i < steps; ++i)
        {
            sim.step();
            g::Row row;
            for (const auto& [name, comp] : targets)
            {
                const auto v = sim.getElement(name)->getComponent(comp);
                row.insert(row.end(), v.begin(), v.end());
            }
            traj.push_back(std::move(row));
        }
        return traj;
    }
}

// ---------------------------------------------------------------------------
// A. field + Gauss stimulus, sub- and supra-threshold
// ---------------------------------------------------------------------------
TEST(GoldenArchitectures, FieldPlusGaussStimulusSubAndSupraThreshold)
{
    struct Regime { std::string slug; double amplitude; };
    const std::vector<Regime> regimes = {
        { "arch_field_stimulus_subthreshold",   2.0 },
        { "arch_field_stimulus_suprathreshold", 12.0 },
    };

    for (const auto& r : regimes)
    {
        auto sim = createSimulation(r.slug, 1.0);

        const ElementCommonParameters fieldCp(r.slug + "_field", 100);
        const SigmoidFunction sf(0.0, 10.0);
        const NeuralFieldParameters nfp(25.0, -5.0, sf);
        const auto field = std::make_shared<NeuralField>(fieldCp, nfp);
        sim->addElement(field);

        const ElementCommonParameters stimCp(r.slug + "_stim", 100);
        const GaussStimulusParameters gsp(5.0, r.amplitude, 50.0, true, false);
        const auto stim = std::make_shared<GaussStimulus>(stimCp, gsp);
        sim->addElement(stim);
        sim->createInteraction(stim->getUniqueName(), "output", field->getUniqueName());

        sim->init();
        const g::Grid traj = g::captureTrajectory(*sim, field->getUniqueName(), "activation", 60);
        g::checkCharacterization(r.slug, traj);
    }
}

// ---------------------------------------------------------------------------
// B. field + self-excitation Gauss kernel -> sustained bump (Amari)
// ---------------------------------------------------------------------------
TEST(GoldenArchitectures, FieldSelfExcitationStableBump)
{
    const std::string slug = "arch_field_self_excitation_stable_bump";
    auto sim = createSimulation(slug, 1.0);

    const ElementCommonParameters fieldCp(slug + "_field", 100);
    const SigmoidFunction sf(0.0, 10.0);
    const NeuralFieldParameters nfp(25.0, -5.0, sf);
    const auto field = std::make_shared<NeuralField>(fieldCp, nfp);
    sim->addElement(field);

    // Brief supra-threshold kick to seed the bump.
    const ElementCommonParameters stimCp(slug + "_stim", 100);
    const GaussStimulusParameters gsp(5.0, 10.0, 50.0, true, false);
    const auto stim = std::make_shared<GaussStimulus>(stimCp, gsp);
    sim->addElement(stim);
    sim->createInteraction(stim->getUniqueName(), "output", field->getUniqueName());

    // Self-excitatory loop: field.output -> kernel -> field.input.
    const ElementCommonParameters kernelCp(slug + "_kernel", 100);
    const GaussKernelParameters gkp(5.0, 8.0, -0.005, true, false);
    const auto kernel = std::make_shared<GaussKernel>(kernelCp, gkp);
    sim->addElement(kernel);
    sim->createInteraction(field->getUniqueName(), "output", kernel->getUniqueName());
    sim->createInteraction(kernel->getUniqueName(), "output", field->getUniqueName());

    sim->init();
    const g::Grid traj = g::captureTrajectory(*sim, field->getUniqueName(), "activation", 150);
    g::checkCharacterization(slug, traj);
}

// ---------------------------------------------------------------------------
// C. field + Mexican-hat kernel -> selection between two competing stimuli
// ---------------------------------------------------------------------------
TEST(GoldenArchitectures, FieldMexicanHatSelection)
{
    const std::string slug = "arch_field_mexican_hat_selection";
    auto sim = createSimulation(slug, 1.0);

    const ElementCommonParameters fieldCp(slug + "_field", 100);
    const SigmoidFunction sf(0.0, 10.0);
    const NeuralFieldParameters nfp(25.0, -5.0, sf);
    const auto field = std::make_shared<NeuralField>(fieldCp, nfp);
    sim->addElement(field);

    // Two competing stimuli of different strength at different locations.
    const ElementCommonParameters stimACp(slug + "_stimA", 100);
    const GaussStimulusParameters gspA(5.0, 10.0, 30.0, true, false);
    const auto stimA = std::make_shared<GaussStimulus>(stimACp, gspA);
    sim->addElement(stimA);
    sim->createInteraction(stimA->getUniqueName(), "output", field->getUniqueName());

    const ElementCommonParameters stimBCp(slug + "_stimB", 100);
    const GaussStimulusParameters gspB(5.0, 6.0, 70.0, true, false);
    const auto stimB = std::make_shared<GaussStimulus>(stimBCp, gspB);
    sim->addElement(stimB);
    sim->createInteraction(stimB->getUniqueName(), "output", field->getUniqueName());

    // Mexican-hat self-interaction: local excitation, broad inhibition -> WTA.
    const ElementCommonParameters kernelCp(slug + "_kernel", 100);
    const MexicanHatKernelParameters mhkp(3.0, 10.0, 8.0, 6.0, -0.02, true, false);
    const auto kernel = std::make_shared<MexicanHatKernel>(kernelCp, mhkp);
    sim->addElement(kernel);
    sim->createInteraction(field->getUniqueName(), "output", kernel->getUniqueName());
    sim->createInteraction(kernel->getUniqueName(), "output", field->getUniqueName());

    sim->init();
    const g::Grid traj = g::captureTrajectory(*sim, field->getUniqueName(), "activation", 200);
    g::checkCharacterization(slug, traj);
}

// ---------------------------------------------------------------------------
// D. two coupled fields via GaussFieldCoupling
// ---------------------------------------------------------------------------
TEST(GoldenArchitectures, TwoFieldsCoupledViaGaussFieldCoupling)
{
    const std::string slug = "arch_two_fields_gauss_coupling";
    auto sim = createSimulation(slug, 1.0);

    const ElementCommonParameters fieldACp(slug + "_fieldA", 100);
    const SigmoidFunction sf(0.0, 10.0);
    const NeuralFieldParameters nfpA(25.0, -5.0, sf);
    const auto fieldA = std::make_shared<NeuralField>(fieldACp, nfpA);
    sim->addElement(fieldA);

    const ElementCommonParameters stimCp(slug + "_stim", 100);
    const GaussStimulusParameters gsp(5.0, 10.0, 30.0, true, false);
    const auto stim = std::make_shared<GaussStimulus>(stimCp, gsp);
    sim->addElement(stim);
    sim->createInteraction(stim->getUniqueName(), "output", fieldA->getUniqueName());

    const ElementCommonParameters fieldBCp(slug + "_fieldB", 100);
    const NeuralFieldParameters nfpB(25.0, -5.0, sf);
    const auto fieldB = std::make_shared<NeuralField>(fieldBCp, nfpB);
    sim->addElement(fieldB);

    // Fixed relay projection: several point-to-point Gaussian couplings from
    // fieldA's locations onto the same locations in fieldB (a "relay" map),
    // so a bump forming in A gets projected onto B one step later.
    ElementDimensions inputDims(100, 1.0);
    GaussFieldCouplingParameters gfcp(inputDims, false, true, {});
    gfcp.addCoupling(GaussCoupling(30.0, 30.0, 6.0, 4.0));
    gfcp.addCoupling(GaussCoupling(50.0, 50.0, 6.0, 4.0));
    gfcp.addCoupling(GaussCoupling(70.0, 70.0, 6.0, 4.0));
    const ElementCommonParameters couplingCp(slug + "_coupling", 100);
    const auto coupling = std::make_shared<GaussFieldCoupling>(couplingCp, gfcp);
    sim->addElement(coupling);
    sim->createInteraction(fieldA->getUniqueName(), "output", coupling->getUniqueName());
    sim->createInteraction(coupling->getUniqueName(), "output", fieldB->getUniqueName());

    sim->init();
    const g::Grid traj = captureMulti(*sim,
        { { fieldA->getUniqueName(), "activation" }, { fieldB->getUniqueName(), "activation" } },
        80);
    g::checkCharacterization(slug, traj);
}

// ---------------------------------------------------------------------------
// E. memory-trace formation from a field driven by a stimulus (onset/offset)
// ---------------------------------------------------------------------------
TEST(GoldenArchitectures, FieldDrivenMemoryTraceFormation)
{
    const std::string slug = "arch_field_memory_trace_formation";
    auto sim = createSimulation(slug, 1.0);

    const ElementCommonParameters fieldCp(slug + "_field", 100);
    const SigmoidFunction sf(0.0, 10.0);
    const NeuralFieldParameters nfp(25.0, -5.0, sf);
    const auto field = std::make_shared<NeuralField>(fieldCp, nfp);
    sim->addElement(field);

    const ElementCommonParameters stimCp(slug + "_stim", 100);
    const GaussStimulusParameters gspOn(5.0, 10.0, 50.0, true, false);
    const auto stim = std::make_shared<GaussStimulus>(stimCp, gspOn);
    sim->addElement(stim);
    sim->createInteraction(stim->getUniqueName(), "output", field->getUniqueName());

    const ElementCommonParameters traceCp(slug + "_trace", 100);
    const MemoryTraceParameters mtp(50.0, 400.0, 0.5);
    const auto trace = std::make_shared<MemoryTrace>(traceCp, mtp);
    sim->addElement(trace);
    sim->createInteraction(field->getUniqueName(), "output", trace->getUniqueName());

    sim->init();

    g::Grid traj = captureMulti(*sim,
        { { field->getUniqueName(), "activation" }, { trace->getUniqueName(), "output" } },
        40);

    // Remove the stimulus (onset/offset) and continue: trace should persist
    // even as the field's own activation relaxes back toward resting level.
    const GaussStimulusParameters gspOff(5.0, 0.0, 50.0, true, false);
    stim->setParameters(gspOff);

    const g::Grid trajOff = captureMulti(*sim,
        { { field->getUniqueName(), "activation" }, { trace->getUniqueName(), "output" } },
        60);
    traj.insert(traj.end(), trajOff.begin(), trajOff.end());

    g::checkCharacterization(slug, traj);
}
