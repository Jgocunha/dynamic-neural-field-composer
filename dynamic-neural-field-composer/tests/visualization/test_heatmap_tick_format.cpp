#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>

#include "visualization/heatmap.h"

using namespace dnf_composer;

// ---------------------------------------------------------------------------
// selectHeatmapTickFormat
//
// Every ImPlot::ColormapScale(...) call site in this codebase omitted the
// format argument, so ImPlot's "%g" default rendered colorbar tick labels
// unreadably for narrow ranges: a FieldCoupling DELTA weight matrix spanning
// roughly [-0.0074, 0.0090] (correct -- see FieldCoupling::updateWeights())
// showed every tick as "0.00" in the standalone Plot window and as garbage
// multi-digit integers in the node-graph plot card. Heatmap::render() (an
// ImGui/ImPlot render function, not headlessly unit-testable per
// .claude/tests/05-gui-headless.md) is not exercised here; this free function
// is the extracted, pure, testable core of the format-selection decision
// (see heatmap.h for the extraction rationale, matching
// resolveManualHeatmapDimensions in test_heatmap_dimensions.cpp).
// ---------------------------------------------------------------------------

TEST(SelectHeatmapTickFormat, WeightMatrixRangeGetsFourDecimals)
{
    // The actual regression: a DELTA coupling's learned weight matrix.
    EXPECT_STREQ(selectHeatmapTickFormat(-0.00739912, 0.0089524), "%.4f");
}

TEST(SelectHeatmapTickFormat, OrdinaryFieldRangeGetsWholeNumbers)
{
    EXPECT_STREQ(selectHeatmapTickFormat(-20.0, 20.0), "%.0f");
}

TEST(SelectHeatmapTickFormat, UnitScaleRangeGetsTwoDecimals)
{
    EXPECT_STREQ(selectHeatmapTickFormat(0.0, 8.66), "%.2f");
}

TEST(SelectHeatmapTickFormat, VerySmallRangeGetsScientific)
{
    EXPECT_STREQ(selectHeatmapTickFormat(0.0, 1e-5), "%.1e");
}

TEST(SelectHeatmapTickFormat, NarrowRangeOffsetFromZeroGetsScientific)
{
    // span = 0.000101 alone would pick "%.5f" from its width, but that format
    // rounds the nonzero endpoint 0.000001 down to "0.00000" -- an unreadable
    // label for a value that is not actually zero. Must fall back to scientific.
    EXPECT_STREQ(selectHeatmapTickFormat(0.000001, 0.000102), "%.1e");
}

TEST(SelectHeatmapTickFormat, ArgumentOrderDoesNotMatter)
{
    EXPECT_STREQ(selectHeatmapTickFormat(-0.00739912, 0.0089524),
                 selectHeatmapTickFormat(0.0089524, -0.00739912));
}

TEST(SelectHeatmapTickFormat, DegenerateAndNonFiniteRangesReturnAUsableFormat)
{
    EXPECT_NE(selectHeatmapTickFormat(1.0, 1.0), nullptr);
    EXPECT_NE(selectHeatmapTickFormat(0.0, 0.0), nullptr);
    EXPECT_NE(selectHeatmapTickFormat(std::numeric_limits<double>::quiet_NaN(), 1.0), nullptr);
    EXPECT_NE(selectHeatmapTickFormat(0.0, std::numeric_limits<double>::infinity()), nullptr);
}

TEST(SelectHeatmapTickFormat, EveryReturnedFormatIsNonNullAndFormatsWithoutTruncation)
{
    // Sweep spans across ~12 decades and confirm the returned format actually
    // renders a representative nonzero value as something other than an
    // all-zeros string -- the property that encodes "the user can read the
    // number", not just "a format string came back".
    const double spans[] = { 1e6, 1e3, 50.0, 5.0, 1.0, 0.5, 0.05, 0.005, 5e-4, 5e-5, 5e-6, 5e-9 };
    for (const double span : spans)
    {
        const char* fmt = selectHeatmapTickFormat(0.0, span);
        ASSERT_NE(fmt, nullptr);

        char buff[64];
        const double sample = span * 0.5;
        std::snprintf(buff, sizeof(buff), fmt, sample);

        EXPECT_GT(std::strlen(buff), 0u) << "span=" << span;
        if (sample != 0.0)
        {
            const bool allZeroish = (std::strspn(buff, "-0.,e+ ") == std::strlen(buff));
            EXPECT_FALSE(allZeroish) << "span=" << span << " formatted as \"" << buff << "\"";
        }
    }
}
