#include <gtest/gtest.h>
#include <memory>

#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
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

static std::shared_ptr<GaussStimulus2D> makeStimulus2D(const std::string& name,
    const int sizeX, const int sizeY)
{
    ElementCommonParameters cp{ name, ElementDimensions{ sizeX, sizeY, 1.0, 1.0 } };
    GaussStimulus2DParameters gsp{ 5.0, 15.0, 0.0, 0.0, true, false };
    return std::make_shared<GaussStimulus2D>(cp, gsp);
}

static std::shared_ptr<NeuralField2D> makeField2D(const std::string& name,
    const int sizeX, const int sizeY)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralField2DParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, ElementDimensions{ sizeX, sizeY, 1.0, 1.0 } };
    return std::make_shared<NeuralField2D>(cp, nfp);
}

// ---------------------------------------------------------------------------
// Construction with an invalid size (#118)
// ---------------------------------------------------------------------------
//
// ElementDimensions's own constructors already validate extent/spacing and can never
// themselves produce size <= 0 (see tests/element_parameters/test_element_parameters.cpp).
// The only way an Element subclass constructor can still observe an invalid size today is
// if an already-validated ElementDimensions is mutated afterward (its fields are public)
// before being wrapped in ElementCommonParameters - reproducing the scenario Element's own
// defense-in-depth check guards against.

TEST(ElementConstruction, InvalidZeroSizeThrows)
{
    ElementDimensions dims{ 100, 1.0 };
    dims.size = 0;
    const ElementCommonParameters cp{ "bad", dims };
    const SigmoidFunction sig{ 0.0, 10.0 };
    const NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    EXPECT_THROW(NeuralField(cp, nfp), dnf_composer::Exception);
}

TEST(ElementConstruction, InvalidNegativeSizeThrows)
{
    ElementDimensions dims{ 100, 1.0 };
    dims.size = -5;
    const ElementCommonParameters cp{ "bad", dims };
    EXPECT_THROW(GaussKernel(cp, GaussKernelParameters{}), dnf_composer::Exception);
}

TEST(ElementConstruction, InvalidSizeThrowsElemInvalidSizeWithElementName)
{
    ElementDimensions dims{ 100, 1.0 };
    dims.size = 0;
    const ElementCommonParameters cp{ "bad-element", dims };
    try
    {
        NeuralField nf(cp, NeuralFieldParameters{});
        FAIL() << "Expected Exception to be thrown";
    }
    catch (const dnf_composer::Exception& e)
    {
        EXPECT_EQ(e.getErrorCode(), ErrorCode::ELEM_INVALID_SIZE);
        EXPECT_NE(std::string(e.what()).find("bad-element"), std::string::npos);
    }
}

TEST(ElementConstruction, InvalidSizeNeverProducesAConstructedObject)
{
    // Acceptance criterion: "No code path can observe an Element without its
    // components." A throwing constructor guarantees this (no object is ever
    // produced/assigned), verified here through a factory-style call site.
    ElementDimensions dims{ 100, 1.0 };
    dims.size = 0;
    const ElementCommonParameters cp{ "bad", dims };
    std::shared_ptr<NeuralField> nf;
    EXPECT_THROW(nf = std::make_shared<NeuralField>(cp, NeuralFieldParameters{}), dnf_composer::Exception);
    EXPECT_EQ(nf, nullptr);
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

// Regression (#41): a 1D size-10 element and a 2D 5x2 element both flatten to
// 10 samples, but their spatial layouts are not interchangeable. The old
// flattened-size-only check let this connect silently; it must now be rejected.
TEST(ElementInputs, Add1DInputTo2DElementWithMatchingFlattenedSizeIsRejected)
{
    // position 0 keeps the Gaussian centre in range for this small 1D field.
    const GaussStimulusParameters gp{ 1.0, 1.0, 0.0 };
    const auto stim1D = std::make_shared<GaussStimulus>(
        ElementCommonParameters{ "stim-1d", ElementDimensions{ 10, 1.0 } }, gp);  // 1D, size 10
    const auto field2D = makeField2D("field-2d", 5, 2);     // 2D, 5x2 = 10

    EXPECT_NO_THROW(field2D->addInput(stim1D, "output"));

    EXPECT_FALSE(field2D->hasInput(stim1D->getUniqueName(), "output"));
    EXPECT_FALSE(stim1D->hasOutput(field2D->getUniqueName(), "output"));
    EXPECT_TRUE(field2D->getInputs().empty());
}

// Symmetric case: a 2D 5x2 source into a 1D size-10 element must also be rejected.
TEST(ElementInputs, Add2DInputTo1DElementWithMatchingFlattenedSizeIsRejected)
{
    const auto stim2D = makeStimulus2D("stim-2d", 5, 2);    // 2D, 5x2 = 10
    const auto field1D = makeField("field-1d", 10);         // 1D, size 10

    EXPECT_NO_THROW(field1D->addInput(stim2D, "output"));

    EXPECT_FALSE(field1D->hasInput(stim2D->getUniqueName(), "output"));
    EXPECT_TRUE(field1D->getInputs().empty());
}

// Same-shape 2D<->2D connections must still succeed after the stricter gate.
TEST(ElementInputs, AddSameShape2DInputConnects)
{
    const auto stim2D  = makeStimulus2D("stim-2d", 5, 2);
    const auto field2D = makeField2D("field-2d", 5, 2);

    EXPECT_NO_THROW(field2D->addInput(stim2D, "output"));

    EXPECT_TRUE(field2D->hasInput(stim2D->getUniqueName(), "output"));
    ASSERT_EQ(field2D->getInputs().size(), 1u);
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

// Regression test for #168: the singular removeInput() overloads must erase the
// matching entry from the *other* element's `outputs`, exactly like removeInputs()
// (plural) already does. Before the fix, `field->removeInput(id)` cleared only
// `field->inputs`, leaving `stim->outputs` with a dangling bookkeeping entry that
// still reported the (now disconnected) field as an output.
TEST(ElementInputs, RemoveInputByUniqueIdErasesOtherElementsOutputEntry)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    ASSERT_TRUE(stim->hasOutput());

    const int id = stim->getUniqueIdentifier();
    field->removeInput(id);

    EXPECT_FALSE(stim->hasOutput());
    EXPECT_TRUE(stim->getOutputs().empty());
}

// Same asymmetry as above, for the name-based overload.
TEST(ElementInputs, RemoveInputByNameErasesOtherElementsOutputEntry)
{
    const auto stim  = makeStimulus("stim");
    const auto field = makeField("field");
    field->addInput(stim, "output");
    ASSERT_TRUE(stim->hasOutput());

    field->removeInput("stim");

    EXPECT_FALSE(stim->hasOutput());
    EXPECT_TRUE(stim->getOutputs().empty());
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

// Regression test for #168: `Element::outputs` must be a non-owning (observing)
// reference to the downstream element, not a shared_ptr. Before the fix, addInput()
// wired a two-way shared_ptr cycle (field->inputs[stim], stim->outputs[field]), so
// the only external owner of `field` going out of scope did not free it -- it was
// kept alive by `stim`, which is itself still alive (held by this test). A weak_ptr
// observer registered before the scope ends must report the element as destroyed.
TEST(ElementOutputs, DownstreamElementIsDestroyedWhenLastExternalOwnerReleases)
{
    const auto stim = makeStimulus("stim");
    std::weak_ptr<Element> fieldWatcher;
    {
        const auto field = makeField("field");
        field->addInput(stim, "output");
        fieldWatcher = field;
        // `field`'s only external shared_ptr goes out of scope here. `stim->outputs`
        // must not keep it alive.
    }

    EXPECT_TRUE(fieldWatcher.expired());
}

// Regression test for #168: getOutputs() must silently skip an entry whose element
// has already been destroyed (a natural consequence of outputs being non-owning),
// rather than returning a null shared_ptr or crashing. fieldA is destroyed without
// ever calling removeInput()/removeOutputs() -- exactly the pattern the cycle bug
// used to paper over by keeping it alive forever.
TEST(ElementOutputs, GetOutputsSkipsExpiredEntryAfterDownstreamDestroyed)
{
    const auto stim = makeStimulus("stim");
    const auto fieldB = makeField("fieldB");
    fieldB->addInput(stim, "output");
    {
        const auto fieldA = makeField("fieldA");
        fieldA->addInput(stim, "output");
        // fieldA destroyed here; stim->outputs must not keep it alive, and
        // getOutputs() must not surface a stale/null entry for it afterward.
    }

    const auto outputs = stim->getOutputs();
    ASSERT_EQ(outputs.size(), 1u);
    ASSERT_NE(outputs[0], nullptr);
    EXPECT_EQ(outputs[0]->getUniqueName(), "fieldB");
}

// Regression test: removeOutputs() erases the receiver's `inputs` map entry
// but must also invalidate the receiver's input cache (inputPtr/cachedInputs).
// Without that, the receiver keeps a raw pointer into the removed element's
// components["output"] vector; once that element is destroyed, the next
// step() reads freed memory. Reproduced by overwriting the freed stimulus's
// former heap block with a new allocation carrying a recognizable value, then
// asserting the field's cached input does NOT pick it up.
TEST(ElementOutputs, RemoveOutputsInvalidatesReceiverInputCache)
{
    auto field = makeField("field");
    field->init();
    {
        const auto stim = makeStimulus("stim");
        stim->init();
        field->addInput(stim, "output");
        stim->step(0.0, 1.0);
        field->step(0.0, 1.0); // builds field's input cache from stim's output

        stim->removeOutputs();
        // stim goes out of scope here and is destroyed (last shared_ptr).
    }

    // Allocate a same-sized vector of a recognizable sentinel value; a good
    // chance it reuses the just-freed stimulus's heap block.
    std::vector<double> sentinel(100, 12345.0);

    field->step(0.0, 1.0); // must NOT read through the dangling cached pointer

    const auto input = field->getComponent("input");
    for (const double v : input)
    {
        EXPECT_NE(v, 12345.0);
    }
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

// ---------------------------------------------------------------------------
// changeDimensions()
// ---------------------------------------------------------------------------

TEST(ElementChangeDimensions, ResizesComponentsToNewSize)
{
    const auto field = makeField("f", 100);
    field->init();
    field->changeDimensions(ElementDimensions{ 50, 1.0 });
    EXPECT_EQ(field->getSize(), 50);
    EXPECT_EQ(field->getComponent("output").size(), 50u);
    EXPECT_EQ(field->getComponent("input").size(), 50u);
}

TEST(ElementChangeDimensions, KernelResizesCorrectlyViaInit)
{
    const auto kernel = makeKernel("k", 100);
    kernel->init();
    kernel->changeDimensions(ElementDimensions{ 60, 1.0 });
    EXPECT_EQ(kernel->getSize(), 60);
    EXPECT_EQ(kernel->getComponent("output").size(), 60u);
}

TEST(ElementChangeDimensions, ConnectionsAreNotAffected)
{
    const auto stim  = makeStimulus("stim", 100);
    const auto field = makeField("field", 100);
    field->addInput(stim, "output");
    field->changeDimensions(ElementDimensions{ 50, 1.0 });
    // changeDimensions alone does not remove connections (Simulation::changeDimensions does)
    EXPECT_TRUE(field->hasInput());
}
