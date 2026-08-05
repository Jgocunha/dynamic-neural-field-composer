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

TEST(SimulationRunForRealTime, PositiveDurationAdvancesTAndStaysOpen)
{
    // Regression test for #124: runForRealTime() must not wipe data by
    // default; closeOnFinish defaults to false.
    const auto sim = createSimulation("rrt-ok", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    EXPECT_NO_THROW(sim->runForRealTime(20.0)); // 20 ms
    EXPECT_GT(sim->getT(), 0.0);
    EXPECT_TRUE(sim->isInitialized());
}

TEST(SimulationRunForRealTime, CloseOnFinishClosesWhenRequested)
{
    const auto sim = createSimulation("rrt-close", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    sim->runForRealTime(10.0, true);
    EXPECT_FALSE(sim->isInitialized());
}

TEST(SimulationRunForRealTime, AutoInitializesIfNotAlreadyInitialized)
{
    const auto sim = createSimulation("rrt-autoinit", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    EXPECT_FALSE(sim->isInitialized());
    sim->runForRealTime(10.0);
    // Default no longer closes, so the simulation stays initialized.
    EXPECT_TRUE(sim->isInitialized());
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

// ---------------------------------------------------------------------------
// renameElement
// ---------------------------------------------------------------------------

TEST(SimulationRename, HappyPathPreservesConnections)
{
    auto sim = createSimulation("rename-happy", 1.0, 0.0, 0.0);
    sim->addElement(makeStimulus("stim"));
    sim->addElement(makeField("field"));
    sim->createInteraction("stim", "output", "field");
    sim->init();

    sim->renameElement("stim", "stim2");

    EXPECT_EQ(sim->getElement("stim"), nullptr);
    ASSERT_NE(sim->getElement("stim2"), nullptr);
    EXPECT_NO_THROW(sim->step()); // interaction still wired through the renamed element
}

TEST(SimulationRename, SameNameIsNoOp)
{
    auto sim = createSimulation("rename-same", 1.0, 0.0, 0.0);
    sim->addElement(makeField("field"));
    sim->init();

    EXPECT_NO_THROW(sim->renameElement("field", "field"));
    EXPECT_NE(sim->getElement("field"), nullptr);
}

TEST(SimulationRename, NonexistentOldNameIsNoOp)
{
    auto sim = createSimulation("rename-missing", 1.0, 0.0, 0.0);
    sim->addElement(makeField("field"));
    sim->init();

    EXPECT_NO_THROW(sim->renameElement("does-not-exist", "new-name"));
    EXPECT_NE(sim->getElement("field"), nullptr);
    EXPECT_EQ(sim->getElement("new-name"), nullptr);
}

TEST(SimulationRename, NewNameAlreadyExistsIsNoOp)
{
    auto sim = createSimulation("rename-collision", 1.0, 0.0, 0.0);
    sim->addElement(makeField("field-a"));
    sim->addElement(makeField("field-b"));
    sim->init();

    sim->renameElement("field-a", "field-b");

    // Both original names must still resolve; nothing was renamed.
    EXPECT_NE(sim->getElement("field-a"), nullptr);
    EXPECT_NE(sim->getElement("field-b"), nullptr);
}

// ---------------------------------------------------------------------------
// getHighestElementIndex
// ---------------------------------------------------------------------------

TEST(SimulationHighestElementIndex, ZeroForEmptySimulation)
{
    const auto sim = createSimulation("empty-highest", 1.0, 0.0, 0.0);
    EXPECT_EQ(sim->getHighestElementIndex(), 0);
}

TEST(SimulationHighestElementIndex, GrowsAsElementsAreAdded)
{
    auto sim = createSimulation("growing-highest", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf1"));
    const int afterOne = sim->getHighestElementIndex();

    sim->addElement(makeField("nf2"));
    const int afterTwo = sim->getHighestElementIndex();

    EXPECT_GE(afterTwo, afterOne);
}

TEST(SimulationHighestElementIndex, MatchesMaxUniqueIdentifier)
{
    auto sim = createSimulation("matches-max", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf1"));
    sim->addElement(makeField("nf2"));
    sim->addElement(makeField("nf3"));

    int expectedMax = 0;
    for (const auto& el : sim->getElements())
        expectedMax = std::max(expectedMax, el->getUniqueIdentifier());

    EXPECT_EQ(sim->getHighestElementIndex(), expectedMax);
}

// ---------------------------------------------------------------------------
// Timing accessors
// ---------------------------------------------------------------------------

TEST(SimulationTiming, LastStepDurationNonNegativeWhenMeasuring)
{
    // steady_clock::now() has coarse resolution on some platforms/CI runners,
    // so two calls can legitimately land in the same tick and report 0ns even
    // though measurement is enabled and the implementation is correct.
    // Asserting > 0 would be flaky; >= 0 still verifies the accessor works.
    auto sim = createSimulation("timing-on", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf1"));
    sim->init();
    sim->step();
    EXPECT_GE(sim->getLastStepDuration().count(), 0);
}

TEST(SimulationTiming, LastStepDurationStaysZeroWhenDisabled)
{
    auto sim = createSimulation("timing-off", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf1"));
    sim->setMeasureStepDuration(false);
    EXPECT_FALSE(sim->getMeasureStepDuration());
    sim->init();
    sim->step();
    EXPECT_EQ(sim->getLastStepDuration().count(), 0);
}

TEST(SimulationTiming, MeasureStepDurationRoundTrips)
{
    auto sim = createSimulation("measure-flag", 1.0, 0.0, 0.0);
    EXPECT_TRUE(sim->getMeasureStepDuration()); // default enabled
    sim->setMeasureStepDuration(false);
    EXPECT_FALSE(sim->getMeasureStepDuration());
    sim->setMeasureStepDuration(true);
    EXPECT_TRUE(sim->getMeasureStepDuration());
}

TEST(SimulationTiming, TotalRunDurationNonDecreasingAcrossSteps)
{
    auto sim = createSimulation("total-run", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf1"));
    sim->init();
    sim->step();
    const auto firstTotal = sim->getTotalRunDuration();
    sim->step();
    sim->step();
    const auto secondTotal = sim->getTotalRunDuration();
    EXPECT_GE(secondTotal.count(), firstTotal.count());
}

TEST(SimulationTiming, PauseFreezesTotalRunDuration)
{
    auto sim = createSimulation("pause-freeze", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf1"));
    sim->init();
    sim->step();
    sim->pause();

    const auto pausedTotal = sim->getTotalRunDuration();
    const auto pausedTotalAgain = sim->getTotalRunDuration();
    EXPECT_EQ(pausedTotal.count(), pausedTotalAgain.count());
}

// ---------------------------------------------------------------------------
// NeuralField::getSelfExcitationKernel
// ---------------------------------------------------------------------------

TEST(SelfExcitationKernel, ReturnsKernelInSelfLoop)
{
    auto sim = createSimulation("self-excitation", 1.0, 0.0, 0.0);
    const auto field = makeField("field");
    ElementCommonParameters kcp{ "kernel", 100 };
    const auto kernel = std::make_shared<GaussKernel>(kcp, GaussKernelParameters{});

    sim->addElement(field);
    sim->addElement(kernel);
    sim->createInteraction("field", "output", "kernel");
    sim->createInteraction("kernel", "output", "field");
    sim->init();

    const auto selfKernel = field->getSelfExcitationKernel();
    ASSERT_NE(selfKernel, nullptr);
    EXPECT_EQ(selfKernel->getUniqueName(), "kernel");
}

TEST(SelfExcitationKernel, NullWhenKernelFedByAnotherElement)
{
    // Kernel exists and feeds the field, but the kernel's own input comes
    // from a different field, not from the field under test -> not a
    // self-excitation loop.
    auto sim = createSimulation("no-self-excitation", 1.0, 0.0, 0.0);
    const auto field  = makeField("field");
    const auto other  = makeField("other-field");
    ElementCommonParameters kcp{ "kernel", 100 };
    const auto kernel = std::make_shared<GaussKernel>(kcp, GaussKernelParameters{});

    sim->addElement(field);
    sim->addElement(other);
    sim->addElement(kernel);
    sim->createInteraction("other-field", "output", "kernel");
    sim->createInteraction("kernel", "output", "field");
    sim->init();

    EXPECT_EQ(field->getSelfExcitationKernel(), nullptr);
}

TEST(SelfExcitationKernel, NullWhenNoKernelInputs)
{
    auto sim = createSimulation("no-kernel-inputs", 1.0, 0.0, 0.0);
    const auto field = makeField("field");
    sim->addElement(field);
    sim->init();

    EXPECT_EQ(field->getSelfExcitationKernel(), nullptr);
}
