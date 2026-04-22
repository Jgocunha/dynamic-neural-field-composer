#include <gtest/gtest.h>
#include <memory>
#include <numeric>

#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/oscillatory_kernel.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "simulation/simulation.h"
#include "tools/math.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField> makeField(const std::string& name, int size)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<GaussKernel> makeGaussKernel(const std::string& name, int inputSize,
    std::optional<ElementDimensions> outputDims = std::nullopt)
{
    GaussKernelParameters gkp{ 3.0, 3.0, -0.01, true, true, outputDims };
    ElementCommonParameters cp{ name, inputSize };
    return std::make_shared<GaussKernel>(cp, gkp);
}

static std::shared_ptr<MexicanHatKernel> makeMexKernel(const std::string& name, int inputSize,
    std::optional<ElementDimensions> outputDims = std::nullopt)
{
    MexicanHatKernelParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1, true, true, outputDims };
    ElementCommonParameters cp{ name, inputSize };
    return std::make_shared<MexicanHatKernel>(cp, mhkp);
}

static std::shared_ptr<OscillatoryKernel> makeOscKernel(const std::string& name, int inputSize,
    std::optional<ElementDimensions> outputDims = std::nullopt)
{
    OscillatoryKernelParameters okp{ 1.0, 0.08, 0.3, -0.01, true, false, outputDims };
    ElementCommonParameters cp{ name, inputSize };
    return std::make_shared<OscillatoryKernel>(cp, okp);
}

// ---------------------------------------------------------------------------
// tools::math::resample unit tests
// ---------------------------------------------------------------------------

TEST(ResampleTest, SameSizeReturnsIdentical)
{
    const std::vector<double> v{ 1.0, 2.0, 3.0, 4.0 };
    const auto result = tools::math::resample(v, 4);
    ASSERT_EQ(result.size(), 4u);
    for (int i = 0; i < 4; ++i)
        EXPECT_DOUBLE_EQ(result[i], v[i]);
}

TEST(ResampleTest, HalfSizePicksEndpoints)
{
    // 4 → 2: positions 0.0 and 1.0 → indices 0 and 3
    const std::vector<double> v{ 10.0, 20.0, 30.0, 40.0 };
    const auto result = tools::math::resample(v, 2);
    ASSERT_EQ(result.size(), 2u);
    EXPECT_DOUBLE_EQ(result[0], 10.0);
    EXPECT_DOUBLE_EQ(result[1], 40.0);
}

TEST(ResampleTest, DoubleSizeInterpolates)
{
    // 2 → 4: positions 0, 1/3, 2/3, 1 → 0, 0.333, 0.666, 1.0 of [0..1]
    const std::vector<double> v{ 0.0, 3.0 };
    const auto result = tools::math::resample(v, 4);
    ASSERT_EQ(result.size(), 4u);
    EXPECT_DOUBLE_EQ(result[0], 0.0);
    EXPECT_NEAR(result[1], 1.0, 1e-9);
    EXPECT_NEAR(result[2], 2.0, 1e-9);
    EXPECT_DOUBLE_EQ(result[3], 3.0);
}

TEST(ResampleTest, EmptyInputReturnsEmpty)
{
    const std::vector<double> v;
    EXPECT_TRUE(tools::math::resample(v, 5).empty());
}

TEST(ResampleTest, ZeroOutputSizeReturnsEmpty)
{
    const std::vector<double> v{ 1.0, 2.0 };
    EXPECT_TRUE(tools::math::resample(v, 0).empty());
}

TEST(ResampleTest, SingleOutputSizeReturnsCenterSample)
{
    // outputSize == 1 must not divide by (outputSize - 1) == 0
    const std::vector<double> v{ 10.0, 20.0, 30.0, 40.0 };
    const auto result = tools::math::resample(v, 1);
    ASSERT_EQ(result.size(), 1u);
    // center index: N/2 == 2 → v[2] == 30.0
    EXPECT_DOUBLE_EQ(result[0], v[v.size() / 2]);
}

// ---------------------------------------------------------------------------
// GaussKernel cross-dimension
// ---------------------------------------------------------------------------

TEST(GaussKernelCrossDim, OutputSizeMatchesTargetAfterConstruction)
{
    const auto gk = makeGaussKernel("gk", 100, ElementDimensions{ 50 });
    EXPECT_EQ(gk->getComponentPtr("output")->size(), 50u);
}

TEST(GaussKernelCrossDim, SameDimFallbackHasOriginalOutputSize)
{
    const auto gk = makeGaussKernel("gk", 100);
    EXPECT_EQ(gk->getComponentPtr("output")->size(), 100u);
}

TEST(GaussKernelCrossDim, ConnectionFromLargerToSmallerField)
{
    // Field 100 → GaussKernel(in=100, out=50) → Field 50
    const auto fieldSrc = makeField("src", 100);
    const auto gk = makeGaussKernel("gk", 100, ElementDimensions{ 50 });
    const auto fieldDst = makeField("dst", 50);

    EXPECT_NO_THROW(gk->addInput(fieldSrc));
    EXPECT_NO_THROW(fieldDst->addInput(gk));

    EXPECT_EQ(gk->getInputs().size(), 1u);
    EXPECT_EQ(fieldDst->getInputs().size(), 1u);
}

TEST(GaussKernelCrossDim, ConnectionFromSmallerToLargerField)
{
    // Field 50 → GaussKernel(in=50, out=100) → Field 100
    const auto fieldSrc = makeField("src", 50);
    const auto gk = makeGaussKernel("gk", 50, ElementDimensions{ 100 });
    const auto fieldDst = makeField("dst", 100);

    EXPECT_NO_THROW(gk->addInput(fieldSrc));
    EXPECT_NO_THROW(fieldDst->addInput(gk));
}

TEST(GaussKernelCrossDim, StepProducesCorrectOutputSize)
{
    const auto fieldSrc = makeField("src", 100);
    const auto gk = makeGaussKernel("gk", 100, ElementDimensions{ 50 });
    const auto fieldDst = makeField("dst", 50);

    gk->addInput(fieldSrc);
    fieldDst->addInput(gk);

    fieldSrc->init();
    gk->init();
    fieldDst->init();

    fieldSrc->step(0.0, 1.0);
    gk->step(0.0, 1.0);
    fieldDst->step(0.0, 1.0);

    EXPECT_EQ(gk->getComponentPtr("output")->size(), 50u);
}

TEST(GaussKernelCrossDim, SimulationRunsWithCrossDimensionKernel)
{
    const auto sim = createSimulation("cross-dim-gauss", 1.0, 0.0, 0.0);
    sim->addElement(makeField("src", 100));
    sim->addElement(makeGaussKernel("gk", 100, ElementDimensions{ 50 }));
    sim->addElement(makeField("dst", 50));

    sim->createInteraction("src", "output", "gk");
    sim->createInteraction("gk", "output", "dst");

    EXPECT_NO_THROW(sim->init());
    EXPECT_NO_THROW(sim->step());
    EXPECT_NO_THROW(sim->step());
}

TEST(GaussKernelCrossDim, ParametersRoundTrip)
{
    const GaussKernelParameters p{ 3.0, 3.0, -0.01, true, true, ElementDimensions{ 50 } };
    EXPECT_TRUE(p.outputFieldDimensions.has_value());
    EXPECT_EQ(p.outputFieldDimensions->size, 50);
    EXPECT_FALSE(p.toString().empty());
    EXPECT_TRUE(p.toString().find("Output size") != std::string::npos);
}

// ---------------------------------------------------------------------------
// MexicanHatKernel cross-dimension
// ---------------------------------------------------------------------------

TEST(MexicanHatKernelCrossDim, OutputSizeMatchesTargetAfterConstruction)
{
    const auto mhk = makeMexKernel("mhk", 100, ElementDimensions{ 200 });
    EXPECT_EQ(mhk->getComponentPtr("output")->size(), 200u);
}

TEST(MexicanHatKernelCrossDim, StepProducesCorrectOutputSize)
{
    const auto fieldSrc = makeField("src", 100);
    const auto mhk = makeMexKernel("mhk", 100, ElementDimensions{ 200 });
    const auto fieldDst = makeField("dst", 200);

    mhk->addInput(fieldSrc);
    fieldDst->addInput(mhk);

    fieldSrc->init();
    mhk->init();
    fieldDst->init();

    fieldSrc->step(0.0, 1.0);
    mhk->step(0.0, 1.0);
    fieldDst->step(0.0, 1.0);

    EXPECT_EQ(mhk->getComponentPtr("output")->size(), 200u);
}

TEST(MexicanHatKernelCrossDim, SimulationRunsWithCrossDimensionKernel)
{
    const auto sim = createSimulation("cross-dim-mhk", 1.0, 0.0, 0.0);
    sim->addElement(makeField("src", 100));
    sim->addElement(makeMexKernel("mhk", 100, ElementDimensions{ 200 }));
    sim->addElement(makeField("dst", 200));

    sim->createInteraction("src", "output", "mhk");
    sim->createInteraction("mhk", "output", "dst");

    EXPECT_NO_THROW(sim->init());
    EXPECT_NO_THROW(sim->step());
}

// ---------------------------------------------------------------------------
// OscillatoryKernel cross-dimension
// ---------------------------------------------------------------------------

TEST(OscillatoryKernelCrossDim, OutputSizeMatchesTargetAfterConstruction)
{
    const auto ok = makeOscKernel("ok", 100, ElementDimensions{ 75 });
    EXPECT_EQ(ok->getComponentPtr("output")->size(), 75u);
}

TEST(OscillatoryKernelCrossDim, StepProducesCorrectOutputSize)
{
    const auto fieldSrc = makeField("src", 100);
    const auto ok = makeOscKernel("ok", 100, ElementDimensions{ 75 });
    const auto fieldDst = makeField("dst", 75);

    ok->addInput(fieldSrc);
    fieldDst->addInput(ok);

    fieldSrc->init();
    ok->init();
    fieldDst->init();

    fieldSrc->step(0.0, 1.0);
    ok->step(0.0, 1.0);
    fieldDst->step(0.0, 1.0);

    EXPECT_EQ(ok->getComponentPtr("output")->size(), 75u);
}

TEST(OscillatoryKernelCrossDim, SimulationRunsWithCrossDimensionKernel)
{
    const auto sim = createSimulation("cross-dim-osc", 1.0, 0.0, 0.0);
    sim->addElement(makeField("src", 100));
    sim->addElement(makeOscKernel("ok", 100, ElementDimensions{ 75 }));
    sim->addElement(makeField("dst", 75));

    sim->createInteraction("src", "output", "ok");
    sim->createInteraction("ok", "output", "dst");

    EXPECT_NO_THROW(sim->init());
    EXPECT_NO_THROW(sim->step());
}
