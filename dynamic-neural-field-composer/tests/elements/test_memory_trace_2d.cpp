#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <cmath>

#include "elements/memory_trace_2d.h"
#include "elements/boost_stimulus_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x_max = 10, int y_max = 10)
{
    return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
}

static MemoryTrace2DParameters makeMTP(double tauBuild = 100.0,
                                       double tauDecay = 1000.0,
                                       double threshold = 0.5)
{
    return MemoryTrace2DParameters{ tauBuild, tauDecay, threshold };
}

TEST(MemoryTrace2DConstruction, LabelIsMemoryTrace2D)
{
    MemoryTrace2D mt(makeCP("mt"), makeMTP());
    EXPECT_EQ(mt.getLabel(), ElementLabel::MEMORY_TRACE_2D);
}

TEST(MemoryTrace2DConstruction, SizeIsProductOfDimensions)
{
    MemoryTrace2D mt(makeCP("mt", 8, 6), makeMTP());
    EXPECT_EQ(mt.getSize(), 48);
}

TEST(MemoryTrace2DInit, OutputIsAllZerosAfterInit)
{
    MemoryTrace2D mt(makeCP("mt"), makeMTP());
    mt.init();
    for (const double v : mt.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(MemoryTrace2DInit, InputIsAllZerosAfterInit)
{
    MemoryTrace2D mt(makeCP("mt"), makeMTP());
    mt.init();
    for (const double v : mt.getComponent("input"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(MemoryTrace2DStep, ZeroInputDecaysTraceTowardZero)
{
    MemoryTrace2D mt(makeCP("mt", 5, 4), makeMTP(100.0, 1000.0, 0.5));
    mt.init();

    auto* out = mt.getComponentPtr("output");
    std::fill(out->begin(), out->end(), 1.0);

    mt.step(0.0, 1.0);

    for (const double v : mt.getComponent("output"))
        EXPECT_NEAR(v, 0.999, 1e-9);
}

TEST(MemoryTrace2DStep, AboveThresholdInputBuildsTrace)
{
    auto bs = std::make_shared<BoostStimulus2D>(makeCP("bs"), BoostStimulus2DParameters{1.0, true});
    bs->init();
    bs->step(0.0, 1.0);

    auto mt = std::make_shared<MemoryTrace2D>(makeCP("mt"), makeMTP(100.0, 1000.0, 0.5));
    mt->addInput(bs);
    mt->init();
    mt->step(0.0, 1.0);

    for (const double v : mt->getComponent("output"))
        EXPECT_NEAR(v, 0.01, 1e-9);
}

TEST(MemoryTrace2DStep, TraceConvergesToInputAmplitude)
{
    auto bs = std::make_shared<BoostStimulus2D>(makeCP("bs", 5, 4), BoostStimulus2DParameters{0.8, true});
    bs->init();
    bs->step(0.0, 1.0);

    auto mt = std::make_shared<MemoryTrace2D>(makeCP("mt", 5, 4), makeMTP(10.0, 1000.0, 0.5));
    mt->addInput(bs);
    mt->init();

    for (int i = 0; i < 500; ++i)
        mt->step(static_cast<double>(i), 1.0);

    for (const double v : mt->getComponent("output"))
        EXPECT_NEAR(v, 0.8, 0.01);
}

TEST(MemoryTrace2DParameters, GetParametersRoundtrip)
{
    const auto mtp = makeMTP(200.0, 5000.0, 0.3);
    const MemoryTrace2D mt(makeCP("mt"), mtp);
    EXPECT_EQ(mt.getParameters(), mtp);
}

TEST(MemoryTrace2DParameters, SetParametersUpdatesParams)
{
    MemoryTrace2D mt(makeCP("mt"), makeMTP());
    const auto newParams = makeMTP(50.0, 500.0, 0.25);
    mt.setParameters(newParams);
    EXPECT_EQ(mt.getParameters(), newParams);
}

TEST(MemoryTrace2DClone, CloneHasSameParameters)
{
    const auto mtp = makeMTP(150.0, 2000.0, 0.4);
    MemoryTrace2D mt(makeCP("mt"), mtp);
    mt.init();
    const auto cloned = std::dynamic_pointer_cast<MemoryTrace2D>(mt.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), mt.getParameters());
}

TEST(MemoryTrace2DToString, NonEmpty)
{
    const MemoryTrace2D mt(makeCP("mt"), makeMTP());
    EXPECT_FALSE(mt.toString().empty());
}

TEST(MemoryTrace2DElement, LabelStringIsMemoryTrace2D)
{
    const MemoryTrace2D mt(makeCP("mt"), makeMTP());
    const auto labelStr = ElementLabelToString.at(mt.getLabel());
    EXPECT_EQ(labelStr, "memory trace 2d");
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(MemoryTrace2DEdgeCases, OutputRemainsFiniteAfterManySteps)
{
    auto bs = std::make_shared<BoostStimulus2D>(makeCP("bs", 5, 4), BoostStimulus2DParameters{0.8, true});
    bs->init();
    bs->step(0.0, 1.0);

    auto mt = std::make_shared<MemoryTrace2D>(makeCP("mt", 5, 4), makeMTP(10.0, 1000.0, 0.5));
    mt->addInput(bs);
    mt->init();
    for (int i = 0; i < 500; ++i)
        mt->step(static_cast<double>(i), 1.0);

    for (double v : mt->getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}

TEST(MemoryTrace2DEdgeCases, ZeroThresholdAlwaysBuilds)
{
    auto bs = std::make_shared<BoostStimulus2D>(makeCP("bs"), BoostStimulus2DParameters{0.1, true});
    bs->init();
    bs->step(0.0, 1.0);

    auto mt = std::make_shared<MemoryTrace2D>(makeCP("mt"), makeMTP(100.0, 1000.0, 0.0));
    mt->addInput(bs);
    mt->init();
    mt->step(0.0, 1.0);

    const auto out = mt->getComponent("output");
    for (double v : out)
        EXPECT_GT(v, 0.0);
}
