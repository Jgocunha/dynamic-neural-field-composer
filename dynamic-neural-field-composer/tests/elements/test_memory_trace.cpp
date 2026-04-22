#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <cmath>

#include "elements/memory_trace.h"
#include "elements/boost_stimulus.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static ElementCommonParameters makeCP(const std::string& name, const int size = 100)
{
    return ElementCommonParameters{ name, size };
}

static MemoryTraceParameters makeMTP(const double tauBuild = 100.0,
                                     const double tauDecay = 1000.0,
                                     const double threshold = 0.5)
{
    return MemoryTraceParameters{ tauBuild, tauDecay, threshold };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(MemoryTraceConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(MemoryTrace(makeCP("mt"), makeMTP()));
}

TEST(MemoryTraceConstruction, LabelIsMemoryTrace)
{
    MemoryTrace mt(makeCP("mt"), makeMTP());
    EXPECT_EQ(mt.getLabel(), ElementLabel::MEMORY_TRACE);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(MemoryTraceInit, OutputComponentHasCorrectSize)
{
    MemoryTrace mt(makeCP("mt", 80), makeMTP());
    mt.init();
    const auto out = mt.getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 80);
}

TEST(MemoryTraceInit, OutputIsAllZerosAfterInit)
{
    MemoryTrace mt(makeCP("mt", 100), makeMTP());
    mt.init();
    const auto out = mt.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(MemoryTraceInit, InputIsAllZerosAfterInit)
{
    MemoryTrace mt(makeCP("mt", 100), makeMTP());
    mt.init();
    const auto in = mt.getComponent("input");
    for (const double v : in)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// step() — no input (decay)
// ---------------------------------------------------------------------------

TEST(MemoryTraceStep, ZeroInputDecaysTraceTowardZero)
{
    MemoryTrace mt(makeCP("mt", 10), makeMTP(100.0, 1000.0, 0.5));
    mt.init();

    // Pre-fill output with a non-zero trace
    auto* out = mt.getComponentPtr("output");
    std::fill(out->begin(), out->end(), 1.0);

    // step with no input (input remains zeros, below threshold)
    mt.step(0.0, 1.0);

    const auto result = mt.getComponent("output");
    for (const double v : result)
    {
        // With deltaT=1, tauDecay=1000: output += (1/1000)*(-1.0) = -0.001 => 0.999
        EXPECT_NEAR(v, 0.999, 1e-9);
    }
}

TEST(MemoryTraceStep, ZeroInputFullyDecaysOverTime)
{
    MemoryTrace mt(makeCP("mt", 5), makeMTP(100.0, 10.0, 0.5));
    mt.init();

    auto* out = mt.getComponentPtr("output");
    std::fill(out->begin(), out->end(), 1.0);

    // Run many steps; trace should approach 0
    for (int i = 0; i < 200; ++i)
        mt.step(static_cast<double>(i), 1.0);

    const auto result = mt.getComponent("output");
    for (const double v : result)
        EXPECT_NEAR(v, 0.0, 0.01);
}

// ---------------------------------------------------------------------------
// step() — above-threshold input (build-up)
// ---------------------------------------------------------------------------

TEST(MemoryTraceStep, AboveThresholdInputBuildsTrace)
{
    // BoostStimulus provides a uniform above-threshold input via the normal input pipeline
    auto bs = std::make_shared<BoostStimulus>(makeCP("bs", 10), BoostStimulusParameters{1.0, true});
    bs->init();
    bs->step(0.0, 1.0);

    auto mt = std::make_shared<MemoryTrace>(makeCP("mt", 10), makeMTP(100.0, 1000.0, 0.5));
    mt->addInput(bs);
    mt->init();
    mt->step(0.0, 1.0);

    const auto result = mt->getComponent("output");
    for (const double v : result)
    {
        // With deltaT=1, tauBuild=100, output starts at 0, input=1:
        // output += (1/100)*(-0 + 1) = 0.01
        EXPECT_NEAR(v, 0.01, 1e-9);
    }
}

TEST(MemoryTraceStep, TraceConvergesToInputAmplitude)
{
    auto bs = std::make_shared<BoostStimulus>(makeCP("bs", 5), BoostStimulusParameters{0.8, true});
    bs->init();
    bs->step(0.0, 1.0);

    auto mt = std::make_shared<MemoryTrace>(makeCP("mt", 5), makeMTP(10.0, 1000.0, 0.5));
    mt->addInput(bs);
    mt->init();

    for (int i = 0; i < 500; ++i)
        mt->step(static_cast<double>(i), 1.0);

    const auto result = mt->getComponent("output");
    for (const double v : result)
        EXPECT_NEAR(v, 0.8, 0.01);
}

TEST(MemoryTraceStep, BelowThresholdInputDecaysNotBuilds)
{
    // BoostStimulus at 0.3 — below the 0.5 threshold — so decay branch is taken
    auto bs = std::make_shared<BoostStimulus>(makeCP("bs", 10), BoostStimulusParameters{0.3, true});
    bs->init();
    bs->step(0.0, 1.0);

    auto mt = std::make_shared<MemoryTrace>(makeCP("mt", 10), makeMTP(100.0, 1000.0, 0.5));
    mt->addInput(bs);
    mt->init();

    auto* out = mt->getComponentPtr("output");
    std::fill(out->begin(), out->end(), 1.0);

    mt->step(0.0, 1.0);

    const auto result = mt->getComponent("output");
    for (const double v : result)
        EXPECT_LT(v, 1.0);   // decay branch taken: trace decreases
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(MemoryTraceParameters, GetParametersRoundtrip)
{
    const auto mtp = makeMTP(200.0, 5000.0, 0.3);
    const MemoryTrace mt(makeCP("mt"), mtp);
    EXPECT_EQ(mt.getParameters(), mtp);
}

TEST(MemoryTraceParameters, SetParametersUpdatesParams)
{
    MemoryTrace mt(makeCP("mt"), makeMTP());
    const auto newParams = makeMTP(50.0, 500.0, 0.25);
    mt.setParameters(newParams);
    EXPECT_EQ(mt.getParameters(), newParams);
}

TEST(MemoryTraceParameters, SetParametersDoesNotResetTrace)
{
    MemoryTrace mt(makeCP("mt", 5), makeMTP(100.0, 1000.0, 0.5));
    mt.init();

    auto* out = mt.getComponentPtr("output");
    std::fill(out->begin(), out->end(), 0.5);

    // Changing tauBuild should not wipe the accumulated trace
    mt.setParameters(makeMTP(200.0, 1000.0, 0.5));

    const auto result = mt.getComponent("output");
    for (const double v : result)
        EXPECT_DOUBLE_EQ(v, 0.5);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(MemoryTraceClone, CloneHasSameParameters)
{
    const auto mtp = makeMTP(150.0, 2000.0, 0.4);
    MemoryTrace mt(makeCP("mt"), mtp);
    mt.init();
    const auto cloned = mt.clone();
    const auto clonedMT = std::dynamic_pointer_cast<MemoryTrace>(cloned);
    ASSERT_NE(clonedMT, nullptr);
    EXPECT_EQ(clonedMT->getParameters(), mt.getParameters());
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(MemoryTraceToString, NonEmpty)
{
    const MemoryTrace mt(makeCP("mt"), makeMTP());
    EXPECT_FALSE(mt.toString().empty());
}

// ---------------------------------------------------------------------------
// Element base: name, size, label
// ---------------------------------------------------------------------------

TEST(MemoryTraceElement, UniqueNameMatchesConstruction)
{
    const MemoryTrace mt(makeCP("my-trace"), makeMTP());
    EXPECT_EQ(mt.getUniqueName(), "my-trace");
}

TEST(MemoryTraceElement, SizeMatchesDimension)
{
    const MemoryTrace mt(makeCP("mt", 60), makeMTP());
    EXPECT_EQ(mt.getSize(), 60);
}

TEST(MemoryTraceElement, LabelStringIsMemoryTrace)
{
    const MemoryTrace mt(makeCP("mt"), makeMTP());
    const auto labelStr = ElementLabelToString.at(mt.getLabel());
    EXPECT_EQ(labelStr, "memory trace");
}
