#include <gtest/gtest.h>
#include <memory>

#include "elements/gauss_stimulus.h"
#include "elements/boost_stimulus.h"
#include "elements/normal_noise.h"
#include "elements/neural_field.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/field_coupling.h"
#include "elements/gauss_field_coupling.h"
#include "elements/memory_trace.h"
#include "elements/activation_function.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static ElementDimensions dim(const int size) { return ElementDimensions{ size }; }

static std::shared_ptr<GaussStimulus> makeGaussStimulus(int size = 100)
{
    ElementCommonParameters cp{ "gs", size };
    return std::make_shared<GaussStimulus>(cp, GaussStimulusParameters{ 5.0, 15.0, 50.0, true, false });
}

static std::shared_ptr<BoostStimulus> makeBoostStimulus(int size = 100)
{
    ElementCommonParameters cp{ "bs", size };
    return std::make_shared<BoostStimulus>(cp, BoostStimulusParameters{ 5.0, true });
}

static std::shared_ptr<NormalNoise> makeNormalNoise(int size = 100)
{
    ElementCommonParameters cp{ "nn", size };
    return std::make_shared<NormalNoise>(cp, NormalNoiseParameters{ 0.2 });
}

static std::shared_ptr<NeuralField> makeNeuralField(int size = 100)
{
    ElementCommonParameters cp{ "nf", size };
    return std::make_shared<NeuralField>(cp, NeuralFieldParameters{ 25.0, -5.0, SigmoidFunction{ 0.0, 10.0 } });
}

static std::shared_ptr<MemoryTrace> makeMemoryTrace(int size = 100)
{
    ElementCommonParameters cp{ "mt", size };
    return std::make_shared<MemoryTrace>(cp, MemoryTraceParameters{ 100.0, 1000.0, 0.5 });
}

static std::shared_ptr<GaussKernel> makeGaussKernel(int size = 100)
{
    ElementCommonParameters cp{ "gk", size };
    return std::make_shared<GaussKernel>(cp, GaussKernelParameters{ 5.0, 3.0, -0.01, true, true });
}

static std::shared_ptr<MexicanHatKernel> makeMexicanHatKernel(int size = 100)
{
    ElementCommonParameters cp{ "mhk", size };
    return std::make_shared<MexicanHatKernel>(cp, MexicanHatKernelParameters{});
}

static std::shared_ptr<OscillatoryKernel> makeOscillatoryKernel(int size = 100)
{
    ElementCommonParameters cp{ "ok", size };
    return std::make_shared<OscillatoryKernel>(cp, OscillatoryKernelParameters{});
}

static std::shared_ptr<AsymmetricGaussKernel> makeAsymmetricGaussKernel(int size = 100)
{
    ElementCommonParameters cp{ "agk", size };
    return std::make_shared<AsymmetricGaussKernel>(cp, AsymmetricGaussKernelParameters{});
}

static std::shared_ptr<FieldCoupling> makeFieldCoupling(int inSize = 100, int outSize = 100)
{
    ElementCommonParameters cp{ "fc", outSize };
    FieldCouplingParameters fcp{ ElementDimensions{ inSize }, LearningRule::HEBB, 1.0, 0.01 };
    return std::make_shared<FieldCoupling>(cp, fcp);
}

static std::shared_ptr<GaussFieldCoupling> makeGaussFieldCoupling(int inSize = 100, int outSize = 100)
{
    ElementCommonParameters cp{ "gfc", outSize };
    GaussFieldCouplingParameters gfcp{ ElementDimensions{ inSize } };
    return std::make_shared<GaussFieldCoupling>(cp, gfcp);
}

// ---------------------------------------------------------------------------
// Macro: verifies output and input components resize and no throw
// ---------------------------------------------------------------------------

#define EXPECT_CHANGE_DIMENSIONS(elem, newSize)               \
    do {                                                       \
        (elem)->init();                                        \
        EXPECT_NO_THROW((elem)->changeDimensions(dim(newSize))); \
        EXPECT_EQ((elem)->getSize(), (newSize));               \
        EXPECT_EQ((elem)->getComponent("output").size(),       \
                  static_cast<size_t>(newSize));               \
    } while (false)

// ---------------------------------------------------------------------------
// GaussStimulus
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, GaussStimulus)
{
    const auto el = makeGaussStimulus(100);
    EXPECT_CHANGE_DIMENSIONS(el, 60);
}

// ---------------------------------------------------------------------------
// BoostStimulus
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, BoostStimulus)
{
    const auto el = makeBoostStimulus(100);
    EXPECT_CHANGE_DIMENSIONS(el, 80);
}

// ---------------------------------------------------------------------------
// NormalNoise
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, NormalNoise)
{
    const auto el = makeNormalNoise(100);
    EXPECT_CHANGE_DIMENSIONS(el, 50);
}

// ---------------------------------------------------------------------------
// NeuralField
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, NeuralField)
{
    const auto el = makeNeuralField(100);
    EXPECT_CHANGE_DIMENSIONS(el, 75);
}

// ---------------------------------------------------------------------------
// MemoryTrace
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, MemoryTrace)
{
    const auto el = makeMemoryTrace(100);
    EXPECT_CHANGE_DIMENSIONS(el, 40);
}

// ---------------------------------------------------------------------------
// GaussKernel — kernel component must be sized correctly by init()
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, GaussKernel_OutputResizes)
{
    const auto el = makeGaussKernel(100);
    EXPECT_CHANGE_DIMENSIONS(el, 60);
}

TEST(ChangeDimensions, GaussKernel_KernelComponentIsNonEmpty)
{
    const auto el = makeGaussKernel(100);
    el->init();
    el->changeDimensions(dim(60));
    const auto kernel = el->getComponent("kernel");
    EXPECT_FALSE(kernel.empty());
}

// ---------------------------------------------------------------------------
// MexicanHatKernel
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, MexicanHatKernel_OutputResizes)
{
    const auto el = makeMexicanHatKernel(100);
    EXPECT_CHANGE_DIMENSIONS(el, 70);
}

TEST(ChangeDimensions, MexicanHatKernel_KernelComponentIsNonEmpty)
{
    const auto el = makeMexicanHatKernel(100);
    el->init();
    el->changeDimensions(dim(70));
    const auto kernel = el->getComponent("kernel");
    EXPECT_FALSE(kernel.empty());
}

// ---------------------------------------------------------------------------
// OscillatoryKernel
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, OscillatoryKernel_OutputResizes)
{
    const auto el = makeOscillatoryKernel(100);
    EXPECT_CHANGE_DIMENSIONS(el, 80);
}

TEST(ChangeDimensions, OscillatoryKernel_KernelComponentIsNonEmpty)
{
    const auto el = makeOscillatoryKernel(100);
    el->init();
    el->changeDimensions(dim(80));
    const auto kernel = el->getComponent("kernel");
    EXPECT_FALSE(kernel.empty());
}

// ---------------------------------------------------------------------------
// AsymmetricGaussKernel
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, AsymmetricGaussKernel_OutputResizes)
{
    const auto el = makeAsymmetricGaussKernel(100);
    EXPECT_CHANGE_DIMENSIONS(el, 50);
}

TEST(ChangeDimensions, AsymmetricGaussKernel_KernelComponentIsNonEmpty)
{
    const auto el = makeAsymmetricGaussKernel(100);
    el->init();
    el->changeDimensions(dim(50));
    const auto kernel = el->getComponent("kernel");
    EXPECT_FALSE(kernel.empty());
}

// ---------------------------------------------------------------------------
// FieldCoupling
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, FieldCoupling_OutputResizes)
{
    const auto el = makeFieldCoupling(100, 100);
    EXPECT_CHANGE_DIMENSIONS(el, 60);
}

// ---------------------------------------------------------------------------
// GaussFieldCoupling
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, GaussFieldCoupling_OutputResizes)
{
    const auto el = makeGaussFieldCoupling(100, 100);
    EXPECT_CHANGE_DIMENSIONS(el, 80);
}

// ---------------------------------------------------------------------------
// Repeated resize stays consistent
// ---------------------------------------------------------------------------

TEST(ChangeDimensions, RepeatedResizeIsIdempotent)
{
    const auto el = makeNeuralField(100);
    el->init();
    el->changeDimensions(dim(50));
    el->changeDimensions(dim(50));
    EXPECT_EQ(el->getSize(), 50);
    EXPECT_EQ(el->getComponent("output").size(), 50u);
}

TEST(ChangeDimensions, ResizeUpThenDown)
{
    const auto el = makeGaussKernel(100);
    el->init();
    el->changeDimensions(dim(200));
    EXPECT_EQ(el->getSize(), 200);
    el->changeDimensions(dim(50));
    EXPECT_EQ(el->getSize(), 50);
    EXPECT_EQ(el->getComponent("output").size(), 50u);
}
