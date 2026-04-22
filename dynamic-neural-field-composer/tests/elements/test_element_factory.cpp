#include <gtest/gtest.h>
#include <memory>

#include "elements/element_factory.h"
#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/gauss_field_coupling.h"
#include "elements/field_coupling.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/activation_function.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// createElement(type, commonParams, specificParams) — explicit parameters
// ---------------------------------------------------------------------------

TEST(ElementFactoryTest, CreateNeuralFieldWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "nf", 100 };
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    const auto el = factory.createElement(ElementLabel::NEURAL_FIELD, cp, nfp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::NEURAL_FIELD);
    EXPECT_EQ(el->getUniqueName(), "nf");
}

TEST(ElementFactoryTest, CreateGaussStimulusWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "gs", 100 };
    GaussStimulusParameters gsp{ 5.0, 15.0, 50.0, true, false };
    const auto el = factory.createElement(ElementLabel::GAUSS_STIMULUS, cp, gsp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::GAUSS_STIMULUS);
    EXPECT_EQ(el->getUniqueName(), "gs");
}

TEST(ElementFactoryTest, CreateGaussKernelWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "gk", 100 };
    GaussKernelParameters gkp{ 5.0, 10.0, 0.0, true, true };
    const auto el = factory.createElement(ElementLabel::GAUSS_KERNEL, cp, gkp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::GAUSS_KERNEL);
}

TEST(ElementFactoryTest, CreateMexicanHatKernelWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "mhk", 100 };
    MexicanHatKernelParameters mhkp{ 3.0, 10.0, 6.0, 8.0, -0.05, true, true };
    const auto el = factory.createElement(ElementLabel::MEXICAN_HAT_KERNEL, cp, mhkp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::MEXICAN_HAT_KERNEL);
}

TEST(ElementFactoryTest, CreateNormalNoiseWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "nn", 100 };
    NormalNoiseParameters nnp{ 0.01 };
    const auto el = factory.createElement(ElementLabel::NORMAL_NOISE, cp, nnp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::NORMAL_NOISE);
}

TEST(ElementFactoryTest, CreateGaussFieldCouplingWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "gfc", 100 };
    GaussFieldCouplingParameters gfcp{};
    const auto el = factory.createElement(ElementLabel::GAUSS_FIELD_COUPLING, cp, gfcp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::GAUSS_FIELD_COUPLING);
}

TEST(ElementFactoryTest, CreateFieldCouplingWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "fc", 100 };
    FieldCouplingParameters fcp{};
    const auto el = factory.createElement(ElementLabel::FIELD_COUPLING, cp, fcp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::FIELD_COUPLING);
}

TEST(ElementFactoryTest, CreateOscillatoryKernelWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "ok", 100 };
    OscillatoryKernelParameters okp{};
    const auto el = factory.createElement(ElementLabel::OSCILLATORY_KERNEL, cp, okp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::OSCILLATORY_KERNEL);
}

TEST(ElementFactoryTest, CreateAsymmetricGaussKernelWithParams)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "agk", 100 };
    AsymmetricGaussKernelParameters agkp{};
    const auto el = factory.createElement(ElementLabel::ASYMMETRIC_GAUSS_KERNEL, cp, agkp);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::ASYMMETRIC_GAUSS_KERNEL);
}

TEST(ElementFactoryTest, CreateUninitializedWithParamsReturnsNullptr)
{
    ElementFactory factory;
    ElementCommonParameters cp{ "x", 100 };
    NeuralFieldParameters nfp{};
    const auto el = factory.createElement(ElementLabel::UNINITIALIZED, cp, nfp);
    EXPECT_EQ(el, nullptr);
}

// ---------------------------------------------------------------------------
// createElement(type) — default construction
// ---------------------------------------------------------------------------

TEST(ElementFactoryTest, CreateNeuralFieldDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::NEURAL_FIELD);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::NEURAL_FIELD);
}

TEST(ElementFactoryTest, CreateGaussStimulusDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::GAUSS_STIMULUS);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::GAUSS_STIMULUS);
}

TEST(ElementFactoryTest, CreateGaussKernelDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::GAUSS_KERNEL);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::GAUSS_KERNEL);
}

TEST(ElementFactoryTest, CreateMexicanHatKernelDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::MEXICAN_HAT_KERNEL);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::MEXICAN_HAT_KERNEL);
}

TEST(ElementFactoryTest, CreateNormalNoiseDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::NORMAL_NOISE);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::NORMAL_NOISE);
}

TEST(ElementFactoryTest, CreateGaussFieldCouplingDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::GAUSS_FIELD_COUPLING);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::GAUSS_FIELD_COUPLING);
}

TEST(ElementFactoryTest, CreateFieldCouplingDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::FIELD_COUPLING);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::FIELD_COUPLING);
}

TEST(ElementFactoryTest, CreateOscillatoryKernelDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::OSCILLATORY_KERNEL);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::OSCILLATORY_KERNEL);
}

TEST(ElementFactoryTest, CreateAsymmetricGaussKernelDefault)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::ASYMMETRIC_GAUSS_KERNEL);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->getLabel(), ElementLabel::ASYMMETRIC_GAUSS_KERNEL);
}

TEST(ElementFactoryTest, CreateUninitializedDefaultReturnsNullptr)
{
    ElementFactory factory;
    const auto el = factory.createElement(ElementLabel::UNINITIALIZED);
    EXPECT_EQ(el, nullptr);
}

// ---------------------------------------------------------------------------
// Each call produces an independent object
// ---------------------------------------------------------------------------

TEST(ElementFactoryTest, TwoCreatedElementsHaveDistinctIdentifiers)
{
    ElementFactory factory;
    const auto el1 = factory.createElement(ElementLabel::NEURAL_FIELD);
    const auto el2 = factory.createElement(ElementLabel::NEURAL_FIELD);
    ASSERT_NE(el1, nullptr);
    ASSERT_NE(el2, nullptr);
    EXPECT_NE(el1.get(), el2.get());
    EXPECT_NE(el1->getUniqueIdentifier(), el2->getUniqueIdentifier());
}
