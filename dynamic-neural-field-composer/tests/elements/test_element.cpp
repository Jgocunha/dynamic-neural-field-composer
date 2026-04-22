#include <gtest/gtest.h>
#include <memory>

#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "elements/normal_noise.h"
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

static std::shared_ptr<NeuralField> makeField(const std::string& name, int size = 100)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<GaussKernel> makeKernel(const std::string& name, int size = 100)
{
    ElementCommonParameters cp{ name, size };
    return std::make_shared<GaussKernel>(cp, GaussKernelParameters{});
}

// ---------------------------------------------------------------------------
// Identity / metadata
// ---------------------------------------------------------------------------

TEST(ElementIdentity, UniqueIdentifiersAreDifferentForDistinctObjects)
{
    const auto a = makeStimulus("a");
    const auto b = makeStimulus("b");
    EXPECT_NE(a->getUniqueIdentifier(), b->getUniqueIdentifier());
}

TEST(ElementIdentity, UniqueNameMatchesConstruction)
{
    const auto el = makeStimulus("hello-stim");
    EXPECT_EQ(el->getUniqueName(), "hello-stim");
}

TEST(ElementIdentity, GetLabelMatchesType)
{
    const auto stim   = makeStimulus("s");
    const auto field  = makeField("f");
    const auto kernel = makeKernel("k");
    EXPECT_EQ(stim->getLabel(),   ElementLabel::GAUSS_STIMULUS);
    EXPECT_EQ(field->getLabel(),  ElementLabel::NEURAL_FIELD);
    EXPECT_EQ(kernel->getLabel(), ElementLabel::GAUSS_KERNEL);
}

TEST(ElementIdentity, GetSizeMatchesDimensions)
{
    const auto el = makeStimulus("s", 75);
    EXPECT_EQ(el->getSize(), 75);
}

TEST(ElementIdentity, GetMaxSpatialDimension)
{
    const auto el = makeStimulus("s", 80);
    EXPECT_EQ(el->getMaxSpatialDimension(), 80);
}

TEST(ElementIdentity, GetStepSizeIsOne)
{
    const auto el = makeStimulus("s", 100);  // default d_x = 1.0
    EXPECT_DOUBLE_EQ(el->getStepSize(), 1.0);
}

// ---------------------------------------------------------------------------
// Input management
// ---------------------------------------------------------------------------

TEST(ElementInputs, HasNoInputsInitially)
{
    const auto el = makeField("f");
    EXPECT_FALSE(el->hasInput());
}

TEST(ElementInputs, HasInputAfterAddInput)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    EXPECT_TRUE(field->hasInput());
}

TEST(ElementInputs, AddNullInputIsIgnored)
{
    const auto field = makeField("f");

    EXPECT_NO_THROW(field->addInput(nullptr, "output"));
    EXPECT_FALSE(field->hasInput());
}

TEST(ElementInputs, AddDuplicateInputIsIgnored)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    // Adding the same element again should not crash (behavior: warning + skip)
    EXPECT_NO_THROW(field->addInput(stim, "output"));
    const auto inputs = field->getInputs();
    EXPECT_EQ(inputs.size(), 1u);
}

TEST(ElementInputs, AddSizeMismatchedInputDoesNotAddConnection)
{
    const auto smallStim = makeStimulus("small", 100);
    const auto largeField = makeField("large", 200);

    EXPECT_NO_THROW(largeField->addInput(smallStim, "output"));

    EXPECT_FALSE(largeField->hasInput(smallStim->getUniqueName(), "output"));
    EXPECT_FALSE(smallStim->hasOutput(largeField->getUniqueName(), "output"));
}

TEST(ElementInputs, RemoveInputByName)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    EXPECT_TRUE(field->hasInput());
    field->removeInput("stim");
    EXPECT_FALSE(field->hasInput());
}

TEST(ElementInputs, RemoveInputByUniqueId)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    const int id = stim->getUniqueIdentifier();
    field->removeInput(id);
    EXPECT_FALSE(field->hasInput());
}

TEST(ElementInputs, RemoveInputs)
{
    const auto stim1 = makeStimulus("s1");
    const auto stim2 = makeStimulus("s2");
    const auto field = makeField("f");
    field->addInput(stim1, "output");
    field->addInput(stim2, "output");
    field->removeInputs();
    EXPECT_FALSE(field->hasInput());
}

TEST(ElementInputs, HasInputByNameAndComponent)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    EXPECT_TRUE(field->hasInput("stim", "output"));
    EXPECT_FALSE(field->hasInput("stim", "wrong-component"));
    EXPECT_FALSE(field->hasInput("wrong-name", "output"));
}

TEST(ElementInputs, GetInputsReturnsConnectedElements)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    const auto inputs = field->getInputs();
    ASSERT_EQ(inputs.size(), 1u);
    EXPECT_EQ(inputs[0]->getUniqueName(), "stim");
}

TEST(ElementInputs, GetInputsAndComponentsReturnsMap)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    auto map = field->getInputsAndComponents();
    ASSERT_EQ(map.size(), 1u);
    // Find entry for stim
    bool found = false;
    for (const auto& [el, comp] : map)
        if (el->getUniqueName() == "stim" && comp == "output")
            found = true;
    EXPECT_TRUE(found);
}

// ---------------------------------------------------------------------------
// Output management
// ---------------------------------------------------------------------------

TEST(ElementOutputs, HasNoOutputsInitially)
{
    const auto el = makeStimulus("s");
    EXPECT_FALSE(el->hasOutput());
}

TEST(ElementOutputs, HasOutputAfterInputAdded)
{
    // When B adds A as an input, A gets B as an output
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    EXPECT_TRUE(stim->hasOutput());
}

TEST(ElementOutputs, HasOutputByNameAndComponent)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    EXPECT_TRUE(stim->hasOutput("field", "output"));
    EXPECT_FALSE(stim->hasOutput("other", "output"));
}

TEST(ElementOutputs, RemoveOutputByName)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    stim->removeOutput("field");
    EXPECT_FALSE(stim->hasOutput());
}

TEST(ElementOutputs, RemoveOutputs)
{
    const auto stim   = makeStimulus("stim");
    const auto field1 = makeField("f1");
    const auto field2 = makeField("f2");
    field1->addInput(stim, "output");
    field2->addInput(stim, "output");
    stim->removeOutputs();
    EXPECT_FALSE(stim->hasOutput());
}

TEST(ElementOutputs, GetOutputsReturnsConnectedElements)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    const auto outputs = stim->getOutputs();
    ASSERT_EQ(outputs.size(), 1u);
    EXPECT_EQ(outputs[0]->getUniqueName(), "field");
}

// ---------------------------------------------------------------------------
// Component access
// ---------------------------------------------------------------------------

TEST(ElementComponents, GetComponentListIsNonEmpty)
{
    const auto stim = makeStimulus("s");
    stim->init();
    const auto list = stim->getComponentList();
    EXPECT_FALSE(list.empty());
}

TEST(ElementComponents, GetComponentListContainsOutput)
{
    const auto stim = makeStimulus("s");
    stim->init();
    auto list = stim->getComponentList();
    const bool hasOutput = std::ranges::find(list, "output") != list.end();
    EXPECT_TRUE(hasOutput);
}

TEST(ElementComponents, GetComponentByNameReturnsCorrectSize)
{
    const auto stim = makeStimulus("s", 70);
    stim->init();
    const auto c = stim->getComponent("output");
    EXPECT_EQ(static_cast<int>(c.size()), 70);
}

TEST(ElementComponents, GetComponentPtrIsNonNull)
{
    const auto stim = makeStimulus("s");
    stim->init();
    auto* ptr = stim->getComponentPtr("output");
    EXPECT_NE(ptr, nullptr);
}

TEST(ElementComponents, GetComponentPtrModifiesOriginal)
{
    const auto stim = makeStimulus("s", 100);
    stim->init();
    auto* ptr = stim->getComponentPtr("output");
    (*ptr)[0] = 999.0;
    const auto c = stim->getComponent("output");
    EXPECT_DOUBLE_EQ(c[0], 999.0);
}

TEST(ElementComponents, GetComponentsMapIsNonNull)
{
    const auto stim = makeStimulus("s");
    stim->init();
    EXPECT_NE(stim->getComponents(), nullptr);
}

// ---------------------------------------------------------------------------
// ElementCommonParameters getter
// ---------------------------------------------------------------------------

TEST(ElementCommonParameters, GetElementCommonParametersReturnsCorrectName)
{
    const auto stim = makeStimulus("my-stim", 80);
    const auto cp = stim->getElementCommonParameters();
    EXPECT_EQ(cp.identifiers.uniqueName, "my-stim");
    EXPECT_EQ(cp.dimensionParameters.x_max, 80);
}

// ---------------------------------------------------------------------------
// close()
// ---------------------------------------------------------------------------

TEST(ElementClose, CloseDoesNotThrow)
{
    const auto stim = makeStimulus("s");
    stim->init();
    EXPECT_NO_THROW(stim->close());
}
