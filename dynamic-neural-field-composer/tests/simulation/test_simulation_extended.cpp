#include <gtest/gtest.h>
#include <memory>
#include <chrono>

#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/activation_function.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 100)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<GaussStimulus> makeStimulus(const std::string& name, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    GaussStimulusParameters gsp{ 5.0, 15.0, 50.0, true, false };
    return std::make_shared<GaussStimulus>(cp, gsp);
}

// ---------------------------------------------------------------------------
// pause / resume
// ---------------------------------------------------------------------------

TEST(SimulationPauseResume, PausedSimDoesNotAdvanceT)
{
    const auto sim = createSimulation("pause-test", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    sim->init();

    for (int i = 0; i < 5; ++i)
        sim->step();
    const double tBeforePause = sim->getT();

    sim->pause();
    for (int i = 0; i < 10; ++i)
        sim->step();

    EXPECT_DOUBLE_EQ(sim->getT(), tBeforePause);
}

TEST(SimulationPauseResume, ResumedSimAdvancesT)
{
    const auto sim = createSimulation("resume-test", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    sim->init();
    sim->pause();

    for (int i = 0; i < 5; ++i)
        sim->step();
    const double tAfterPause = sim->getT();

    sim->resume();
    for (int i = 0; i < 5; ++i)
        sim->step();

    EXPECT_GT(sim->getT(), tAfterPause);
}

TEST(SimulationPauseResume, PauseThenResumeMultipleCycles)
{
    const auto sim = createSimulation("multi-pause", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    sim->init();

    for (int cycle = 0; cycle < 3; ++cycle)
    {
        const double tBefore = sim->getT();
        sim->pause();
        for (int i = 0; i < 5; ++i) sim->step();
        EXPECT_DOUBLE_EQ(sim->getT(), tBefore);

        sim->resume();
        for (int i = 0; i < 5; ++i) sim->step();
        EXPECT_GT(sim->getT(), tBefore);
    }
}

// ---------------------------------------------------------------------------
// runForRealTime
// ---------------------------------------------------------------------------

TEST(SimulationRunForRealTime, NegativeDurationThrows)
{
    const auto sim = createSimulation("rrt-neg", 1.0, 0.0, 0.0);
    EXPECT_THROW(sim->runForRealTime(-1.0), Exception);
}

TEST(SimulationRunForRealTime, ZeroDurationThrows)
{
    const auto sim = createSimulation("rrt-zero", 1.0, 0.0, 0.0);
    EXPECT_THROW(sim->runForRealTime(0.0), Exception);
}

TEST(SimulationRunForRealTime, PositiveDurationAdvancesTAndCloses)
{
    const auto sim = createSimulation("rrt-ok", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    EXPECT_NO_THROW(sim->runForRealTime(20.0)); // 20 ms
    EXPECT_GT(sim->getT(), 0.0);
    EXPECT_FALSE(sim->isInitialized());
}

TEST(SimulationRunForRealTime, AutoInitializesIfNotAlreadyInitialized)
{
    const auto sim = createSimulation("rrt-autoinit", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    EXPECT_FALSE(sim->isInitialized());
    sim->runForRealTime(10.0);
    // runForRealTime calls close() at the end, so initialized is false again
    EXPECT_FALSE(sim->isInitialized());
    EXPECT_GT(sim->getT(), 0.0);
}

// ---------------------------------------------------------------------------
// Copy constructor / copy assignment
// ---------------------------------------------------------------------------

TEST(SimulationCopy, CopyConstructorPreservesElementCount)
{
    const auto simA = createSimulation("copy-src", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));
    simA->addElement(makeStimulus("gs 1"));

    const Simulation simB(*simA);
    EXPECT_EQ(simB.getNumberOfElements(), 2);
}

TEST(SimulationCopy, CopyConstructorPreservesIdentifier)
{
    const auto simA = createSimulation("my-sim", 1.0, 0.0, 0.0);
    const Simulation simB(*simA);
    EXPECT_EQ(simB.getUniqueIdentifier(), "my-sim");
}

TEST(SimulationCopy, CopyConstructorPreservesDeltaT)
{
    const auto simA = createSimulation("dt-sim", 2.5, 0.0, 0.0);
    const Simulation simB(*simA);
    EXPECT_DOUBLE_EQ(simB.getDeltaT(), 2.5);
}

TEST(SimulationCopy, CopyConstructorProducesDeepCopy)
{
    const auto simA = createSimulation("deep", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));

    const Simulation simB(*simA);

    // The element pointer in B must be a different object from A's
    const auto elA = simA->getElement("nf 1");
    const auto elB = simB.getElement("nf 1");
    ASSERT_NE(elA, nullptr);
    ASSERT_NE(elB, nullptr);
    EXPECT_NE(elA.get(), elB.get());
}

TEST(SimulationCopy, CopyAssignmentPreservesElementCount)
{
    const auto simA = createSimulation("assign-src", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));
    simA->addElement(makeField("nf 2"));

    auto simB = createSimulation("assign-dst", 1.0, 0.0, 0.0);
    *simB = *simA;

    EXPECT_EQ(simB->getNumberOfElements(), 2);
    EXPECT_EQ(simB->getUniqueIdentifier(), "assign-src");
}

TEST(SimulationCopy, SelfAssignmentIsSafe)
{
    auto sim = createSimulation("self", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    EXPECT_NO_THROW(*sim = *sim);
    EXPECT_EQ(sim->getNumberOfElements(), 1);
}
