#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <numeric>

#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/gauss_stimulus.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "simulation/simulation.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Shared helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 100)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<GaussStimulus> makeStimulus(const std::string& name,
                                                    const double pos = 50.0, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    GaussStimulusParameters gsp{ 5.0, 15.0, pos, true, false };
    return std::make_shared<GaussStimulus>(cp, gsp);
}

// ============================================================================
// GaussKernel
// ============================================================================

TEST(GaussKernelConstruction, LabelIsGaussKernel)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    const GaussKernelParameters gkp;
    const GaussKernel k(cp, gkp);
    EXPECT_EQ(k.getLabel(), ElementLabel::GAUSS_KERNEL);
}

TEST(GaussKernelConstruction, KernelComponentExistsAfterConstruction)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{});
    EXPECT_NO_THROW(k.getComponent("kernel"));
}

TEST(GaussKernelInit, KernelComponentHasNonZeroValues)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 5.0, 3.0, -0.01, true, true });
    k.init();
    auto kernel = k.getComponent("kernel");
    const double sum = std::accumulate(kernel.begin(), kernel.end(), 0.0,
                                  [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(sum, 0.0);
}

TEST(GaussKernelInit, KernelRangeIsPositive)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 5.0, 3.0, -0.01, true, true });
    k.init();
    const auto range = k.getKernelRange();
    EXPECT_GE(range[0], 0);
    EXPECT_GE(range[1], 0);
}

TEST(GaussKernelInit, ExtIndexIsNonEmptyForCircular)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 5.0, 3.0, -0.01, true, true });
    k.init();
    EXPECT_FALSE(k.getExtIndex().empty());
}

TEST(GaussKernelParameters, GetParametersRoundtrip)
{
    const GaussKernelParameters gkp{ 4.0, 2.5, -0.05, false, false };
    const ElementCommonParameters cp{ std::string("k"), 100 };
    const GaussKernel k(cp, gkp);
    EXPECT_EQ(k.getParameters(), gkp);
}

TEST(GaussKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, GaussKernelParameters{ 3.0, 2.0, -0.01, true, true });
    k.init();
    const auto before = k.getComponent("kernel");

    const GaussKernelParameters newP{ 8.0, 5.0, -0.02, true, true };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");

    EXPECT_NE(before.size(), 0u);
    EXPECT_NE(after.size(), 0u);
    // Different widths should produce different kernel vectors
    EXPECT_NE(before, after);
}

TEST(GaussKernelClone, CloneHasSameParameters)
{
    GaussKernelParameters gkp{ 5.0, 3.0, -0.01, true, true };
    ElementCommonParameters cp{ std::string("k"), 100 };
    GaussKernel k(cp, gkp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<GaussKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), gkp);
}

TEST(GaussKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("k"), 100 };
    const GaussKernel k(cp, GaussKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}

// Integration: a kernel step produces non-trivial output when driven by a neural field
TEST(GaussKernelIntegration, StepProducesOutputWhenDrivenByField)
{
    Simulation sim("gk-test", 1.0, 0.0, 0.0);
    const auto stim  = makeStimulus("stim", 50.0);
    const auto field = makeField("field");
    ElementCommonParameters kcp{ std::string("kernel"), 100 };
    const auto kernel = std::make_shared<GaussKernel>(kcp, GaussKernelParameters{ 5.0, 3.0, -0.01,
        true, true });

    sim.addElement(stim);
    sim.addElement(field);
    sim.addElement(kernel);
    sim.createInteraction("stim",  "output", "field");
    sim.createInteraction("field", "output", "kernel");
    sim.createInteraction("kernel","output", "field");
    sim.init();

    for (int i = 0; i < 10; ++i)
        sim.step();

    auto out = sim.getComponent("kernel", "output");
    const double absSum = std::accumulate(out.begin(), out.end(), 0.0,
                                     [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(absSum, 0.0);
}

// ============================================================================
// MexicanHatKernel
// ============================================================================

TEST(MexicanHatKernelConstruction, LabelIsMexicanHatKernel)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernel k(cp, MexicanHatKernelParameters{});
    EXPECT_EQ(k.getLabel(), ElementLabel::MEXICAN_HAT_KERNEL);
}

TEST(MexicanHatKernelInit, KernelHasPositiveAndNegativeValues)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernelParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1,
        true, true };
    MexicanHatKernel k(cp, mhkp);
    k.init();
    auto kernel = k.getComponent("kernel");
    const bool hasPositive = std::ranges::any_of(kernel, [](const double v){ return v > 0.0; });
    const bool hasNegative = std::ranges::any_of(kernel, [](const double v){ return v < 0.0; });
    EXPECT_TRUE(hasPositive);
    EXPECT_TRUE(hasNegative);
}

TEST(MexicanHatKernelParameters, GetParametersRoundtrip)
{
    const MexicanHatKernelParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1,
        true, true };
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernel k(cp, mhkp);
    EXPECT_EQ(k.getParameters(), mhkp);
}

TEST(MexicanHatKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    MexicanHatKernel k(cp, MexicanHatKernelParameters{});
    k.init();
    const auto before = k.getComponent("kernel");

    const MexicanHatKernelParameters newP{ 5.0, 20.0, 10.0, 25.0, -0.2,
        true, true };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");
    EXPECT_NE(before, after);
}

TEST(MexicanHatKernelClone, CloneHasSameParameters)
{
    const MexicanHatKernelParameters mhkp{ 2.5, 11.0, 5.0, 15.0, -0.1,
        true, true };
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    MexicanHatKernel k(cp, mhkp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<MexicanHatKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), mhkp);
}

TEST(MexicanHatKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("mhk"), 100 };
    const MexicanHatKernel k(cp, MexicanHatKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}

// ============================================================================
// OscillatoryKernel
// ============================================================================

TEST(OscillatoryKernelParameters, ZeroCrossingsClampedToZeroWhenNegative)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, -0.5, -0.01,
        true, false };
    EXPECT_DOUBLE_EQ(okp.zeroCrossings, 0.0);
}

TEST(OscillatoryKernelParameters, ZeroCrossingsClampedToOneWhenAboveOne)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, 1.5, -0.01,
        true, false };
    EXPECT_DOUBLE_EQ(okp.zeroCrossings, 1.0);
}

TEST(OscillatoryKernelParameters, DecayClampedToPositiveWhenZeroOrNegative)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.0, 0.3, -0.01,
        true, false };
    EXPECT_GT(okp.decay, 0.0);

    const OscillatoryKernelParameters okp2{ 1.0, -1.0, 0.3, -0.01,
        true, false };
    EXPECT_GT(okp2.decay, 0.0);
}

TEST(OscillatoryKernelConstruction, LabelIsOscillatoryKernel)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    const OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    EXPECT_EQ(k.getLabel(), ElementLabel::OSCILLATORY_KERNEL);
}

TEST(OscillatoryKernelInit, KernelHasNonZeroValues)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    k.init();
    auto kernel = k.getComponent("kernel");
    const double absSum = std::accumulate(kernel.begin(), kernel.end(), 0.0,
                                     [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(absSum, 0.0);
}

TEST(OscillatoryKernelParameters, GetParametersRoundtrip)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, 0.3, -0.01,
        true, false };
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    const OscillatoryKernel k(cp, okp);
    EXPECT_EQ(k.getParameters(), okp);
}

TEST(OscillatoryKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    k.init();
    const auto before = k.getComponent("kernel");

    const OscillatoryKernelParameters newP{ 2.0, 0.05, 0.5, -0.02,
        true, false };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");
    EXPECT_NE(before, after);
}

TEST(OscillatoryKernelClone, CloneHasSameParameters)
{
    const OscillatoryKernelParameters okp{ 1.0, 0.08, 0.3, -0.01,
        true, false };
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    OscillatoryKernel k(cp, okp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<OscillatoryKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), okp);
}

TEST(OscillatoryKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("ok"), 100 };
    const OscillatoryKernel k(cp, OscillatoryKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}

// ============================================================================
// AsymmetricGaussKernel
// ============================================================================

TEST(AsymmetricGaussKernelConstruction, LabelIsAsymmetricGaussKernel)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{});
    EXPECT_EQ(k.getLabel(), ElementLabel::ASYMMETRIC_GAUSS_KERNEL);
}

TEST(AsymmetricGaussKernelInit, KernelHasNonZeroValues)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernelParameters agkp{ 3.0, 3.0, 0.0, 0.0,
        true, true };
    AsymmetricGaussKernel k(cp, agkp);
    k.init();
    auto kernel = k.getComponent("kernel");
    const double absSum = std::accumulate(kernel.begin(), kernel.end(), 0.0,
                                     [](const double s, const double v){ return s + std::abs(v); });
    EXPECT_GT(absSum, 0.0);
}

TEST(AsymmetricGaussKernelParameters, GetParametersRoundtrip)
{
    const AsymmetricGaussKernelParameters agkp{ 3.0, 3.0, 0.0, 1.0,
        true, true };
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernel k(cp, agkp);
    EXPECT_EQ(k.getParameters(), agkp);
}

TEST(AsymmetricGaussKernelParameters, SetParametersUpdatesKernel)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{});
    k.init();
    const auto before = k.getComponent("kernel");

    const AsymmetricGaussKernelParameters newP{ 6.0, 5.0, 0.01, 2.0,
        true, true };
    k.setParameters(newP);
    const auto after = k.getComponent("kernel");
    EXPECT_NE(before, after);
}

TEST(AsymmetricGaussKernelClone, CloneHasSameParameters)
{
    const AsymmetricGaussKernelParameters agkp{ 3.0, 3.0, 0.0, 1.0,
        true, true };
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    AsymmetricGaussKernel k(cp, agkp);
    k.init();
    const auto cloned = k.clone();
    auto* ck = dynamic_cast<AsymmetricGaussKernel*>(cloned.get());
    ASSERT_NE(ck, nullptr);
    EXPECT_EQ(ck->getParameters(), agkp);
}

TEST(AsymmetricGaussKernelToString, NonEmpty)
{
    const ElementCommonParameters cp{ std::string("agk"), 100 };
    const AsymmetricGaussKernel k(cp, AsymmetricGaussKernelParameters{});
    EXPECT_FALSE(k.toString().empty());
}
