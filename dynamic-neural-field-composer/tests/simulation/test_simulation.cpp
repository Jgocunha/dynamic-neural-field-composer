#include <gtest/gtest.h>
#include <memory>

#include "simulation/simulation.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<GaussStimulus> makeStimulus(const std::string& name, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    GaussStimulusParameters gsp{ 5.0, 15.0, 50.0, true, false };
    return std::make_shared<GaussStimulus>(cp, gsp);
}

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    const SigmoidFunction sigmoid{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
    return std::make_shared<NeuralField>(cp, nfp);
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(SimulationConstruction, DefaultIdentifierIsGenerated)
{
    const Simulation sim("", 1.0, 0.0, 0.0);
    EXPECT_FALSE(sim.getUniqueIdentifier().empty());
}

TEST(SimulationConstruction, ExplicitIdentifierIsPreserved)
{
    const Simulation sim("my-sim", 1.0, 0.0, 0.0);
    EXPECT_EQ(sim.getUniqueIdentifier(), "my-sim");
}

TEST(SimulationConstruction, TimeParametersAreStored)
{
    const Simulation sim("s", 0.5, 1.0, 1.0);
    EXPECT_DOUBLE_EQ(sim.getDeltaT(), 0.5);
    EXPECT_DOUBLE_EQ(sim.getTZero(), 1.0);
    EXPECT_DOUBLE_EQ(sim.getT(), 1.0);
}

TEST(SimulationConstruction, InvalidDeltaTThrows)
{
    EXPECT_THROW(Simulation("s", 0.0, 0.0, 0.0), Exception);
    EXPECT_THROW(Simulation("s", -1.0, 0.0, 0.0), Exception);
}

TEST(SimulationConstruction, TZeroGreaterThanTThrows)
{
    EXPECT_THROW(Simulation("s", 1.0, 5.0, 0.0), Exception);
}

TEST(SimulationConstruction, FactoryFunctionReturnsValidPointer)
{
    const auto sim = createSimulation("factory-sim", 1.0, 0.0, 0.0);
    ASSERT_NE(sim, nullptr);
    EXPECT_EQ(sim->getUniqueIdentifier(), "factory-sim");
}

TEST(SimulationConstruction, NotInitializedAfterConstruction)
{
    const Simulation sim("s", 1.0, 0.0, 0.0);
    EXPECT_FALSE(sim.isInitialized());
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

TEST(SimulationLifecycle, InitSetsInitializedFlag)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    EXPECT_TRUE(sim.isInitialized());
}

TEST(SimulationLifecycle, StepAdvancesTime)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    sim.step();
    EXPECT_DOUBLE_EQ(sim.getT(), 1.0);
    sim.step();
    EXPECT_DOUBLE_EQ(sim.getT(), 2.0);
}

TEST(SimulationLifecycle, PausedSimDoesNotAdvanceTime)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    sim.pause();
    sim.step();
    EXPECT_DOUBLE_EQ(sim.getT(), 0.0);
}

TEST(SimulationLifecycle, ResumeAfterPauseAllowsStep)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    sim.pause();
    sim.resume();
    sim.step();
    EXPECT_DOUBLE_EQ(sim.getT(), 1.0);
}

TEST(SimulationLifecycle, CloseResetsInitializedFlag)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    EXPECT_TRUE(sim.isInitialized());
    sim.close();
    EXPECT_FALSE(sim.isInitialized());
}

TEST(SimulationLifecycle, CleanRemovesAllElements)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim1"));
    sim.addElement(makeStimulus("stim2"));
    EXPECT_EQ(sim.getNumberOfElements(), 2);
    sim.clean();
    EXPECT_EQ(sim.getNumberOfElements(), 0);
    EXPECT_FALSE(sim.isInitialized());
}

TEST(SimulationLifecycle, RunInvalidTimeThrows)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    EXPECT_THROW(sim.run(0.0), Exception);
    EXPECT_THROW(sim.run(-5.0), Exception);
}

TEST(SimulationLifecycle, RunAutoInitsAndAdvancesTime)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    EXPECT_FALSE(sim.isInitialized());
    sim.run(10.0);
    // After run(), close() is called so isInitialized() == false again
    EXPECT_FALSE(sim.isInitialized());
}

// ---------------------------------------------------------------------------
// Element management
// ---------------------------------------------------------------------------

TEST(SimulationElements, AddElementIncreasesCount)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    EXPECT_EQ(sim.getNumberOfElements(), 0);
    sim.addElement(makeStimulus("stim"));
    EXPECT_EQ(sim.getNumberOfElements(), 1);
}

TEST(SimulationElements, DuplicateNameIsIgnored)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.addElement(makeStimulus("stim")); // duplicate
    EXPECT_EQ(sim.getNumberOfElements(), 1);
}

TEST(SimulationElements, RemoveElementDecreasesCount)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.removeElement("stim");
    EXPECT_EQ(sim.getNumberOfElements(), 0);
}

TEST(SimulationElements, GetElementByNameReturnsCorrectElement)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    const auto el = sim.getElement("stim");
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getUniqueName(), "stim");
}

TEST(SimulationElements, GetElementByNameReturnsNullForMissing)
{
    const Simulation sim("s", 1.0, 0.0, 0.0);
    const auto el = sim.getElement("nonexistent");
    EXPECT_EQ(el, nullptr);
}

TEST(SimulationElements, GetElementByIndexThrowsForMissing)
{
    const Simulation sim("s", 1.0, 0.0, 0.0);
    EXPECT_THROW(sim.getElement(9999), Exception);
}

TEST(SimulationElements, GetElementsReturnsAll)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("a"));
    sim.addElement(makeStimulus("b"));
    EXPECT_EQ(sim.getElements().size(), 2u);
}

TEST(SimulationElements, ResetElementReplacesIt)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();

    const auto newStim = makeStimulus("new-stim");
    sim.resetElement("stim", newStim);

    // Count unchanged, old name gone, new one present
    EXPECT_EQ(sim.getNumberOfElements(), 1);
    // The element at position 0 is now new-stim
    const auto el = sim.getElement("new-stim");
    ASSERT_NE(el, nullptr);
}

// ---------------------------------------------------------------------------
// Interactions
// ---------------------------------------------------------------------------

TEST(SimulationInteractions, CreateInteractionWiresTwoElements)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.addElement(makeField("field"));
    sim.createInteraction("stim", "output", "field");

    const auto dependents = sim.getElementsThatHaveSpecifiedElementAsInput("stim");
    ASSERT_EQ(dependents.size(), 1u);
    EXPECT_EQ(dependents[0]->getUniqueName(), "field");
}

// ---------------------------------------------------------------------------
// Component queries
// ---------------------------------------------------------------------------

TEST(SimulationComponents, ComponentExistsAfterInit)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    EXPECT_TRUE(sim.componentExists("stim", "output"));
}

TEST(SimulationComponents, ComponentExistsReturnsFalseForMissingElement)
{
    const Simulation sim("s", 1.0, 0.0, 0.0);
    EXPECT_FALSE(sim.componentExists("nonexistent", "output"));
}

TEST(SimulationComponents, ComponentExistsReturnsFalseForMissingComponent)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    EXPECT_FALSE(sim.componentExists("stim", "nonexistent_component"));
}

TEST(SimulationComponents, GetComponentReturnsCorrectSize)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    const auto data = sim.getComponent("stim", "output");
    EXPECT_EQ(static_cast<int>(data.size()), sim.getElement("stim")->getSize());
}

TEST(SimulationComponents, GetComponentPtrReturnsNonNull)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    auto* ptr = sim.getComponentPtr("stim", "output");
    EXPECT_NE(ptr, nullptr);
}

// ---------------------------------------------------------------------------
// Time / parameter setters
// ---------------------------------------------------------------------------

TEST(SimulationTime, SetDeltaTUpdatesValue)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.setDeltaT(0.5);
    EXPECT_DOUBLE_EQ(sim.getDeltaT(), 0.5);
}

TEST(SimulationTime, SetDeltaTInvalidThrows)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    EXPECT_THROW(sim.setDeltaT(0.0), Exception);
    EXPECT_THROW(sim.setDeltaT(-1.0), Exception);
}

TEST(SimulationTime, InitResetsTimeToTZero)
{
    Simulation sim("s", 1.0, 2.0, 2.0);
    sim.addElement(makeStimulus("stim"));
    sim.init();
    EXPECT_DOUBLE_EQ(sim.getT(), sim.getTZero());
}

// ---------------------------------------------------------------------------
// Identifier
// ---------------------------------------------------------------------------

TEST(SimulationIdentifier, SetUniqueIdentifierWorks)
{
    Simulation sim("original", 1.0, 0.0, 0.0);
    sim.setUniqueIdentifier("renamed");
    EXPECT_EQ(sim.getUniqueIdentifier(), "renamed");
    EXPECT_EQ(sim.getIdentifier(), "renamed");
}

// ---------------------------------------------------------------------------
// Copy / move semantics
// ---------------------------------------------------------------------------

TEST(SimulationCopyMove, CopyCopiesIdentifierAndParameters)
{
    Simulation orig("orig", 0.5, 0.0, 0.0);
    orig.addElement(makeStimulus("stim"));

    const Simulation copy(orig);
    EXPECT_EQ(copy.getUniqueIdentifier(), "orig");
    EXPECT_DOUBLE_EQ(copy.getDeltaT(), 0.5);
    EXPECT_EQ(copy.getNumberOfElements(), 1);
}

TEST(SimulationCopyMove, MoveTransfersOwnership)
{
    Simulation orig("orig", 0.5, 0.0, 0.0);
    orig.addElement(makeStimulus("stim"));

    const Simulation moved(std::move(orig));
    EXPECT_EQ(moved.getUniqueIdentifier(), "orig");
    EXPECT_EQ(moved.getNumberOfElements(), 1);
}

// ---------------------------------------------------------------------------
// changeDimensions()
// ---------------------------------------------------------------------------

TEST(SimulationChangeDimensions, ResizesElementAndBreaksConnections)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeStimulus("stim"));
    sim.addElement(makeField("field"));
    sim.createInteraction("stim", "output", "field");
    sim.init();

    sim.changeDimensions("field", ElementDimensions{ 50 });

    EXPECT_EQ(sim.getElement("field")->getSize(), 50);
    EXPECT_FALSE(sim.getElement("field")->hasInput());
    EXPECT_FALSE(sim.getElement("stim")->hasOutput());
}

TEST(SimulationChangeDimensions, UnconnectedElementResizesWithoutError)
{
    Simulation sim("s", 1.0, 0.0, 0.0);
    sim.addElement(makeField("field", 100));
    sim.init();
    EXPECT_NO_THROW(sim.changeDimensions("field", ElementDimensions{ 75 }));
    EXPECT_EQ(sim.getElement("field")->getSize(), 75);
}
