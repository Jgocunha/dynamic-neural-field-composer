#include <gtest/gtest.h>

#include <cstddef>

#include "visualization/heatmap.h"

using namespace dnf_composer;

// ---------------------------------------------------------------------------
// resolveManualHeatmapDimensions - issue #121
//
// Heatmap::render() (an ImGui/ImPlot render function, not headlessly
// unit-testable per .claude/tests/05-gui-headless.md) computes manual-mode
// rows/cols from user-editable axis extents/steps with no relationship to the
// actual flattened-data buffer size, then hands rows*cols straight to
// ImPlot::PlotHeatmap - a mismatched manual setting reads past the buffer.
// This free function is the extracted, pure, testable core of that
// computation (see heatmap.h for the extraction rationale).
// ---------------------------------------------------------------------------

TEST(ResolveManualHeatmapDimensions, MatchingSettingsAreUnclamped)
{
    // 10x10 grid, step 1 => rows=10, cols=10 = 100 elements, matches dataSize.
    const auto dims = resolveManualHeatmapDimensions(10, 10, 1.0F, 1.0F, 100);
    EXPECT_EQ(dims.rows, 10);
    EXPECT_EQ(dims.cols, 10);
    EXPECT_FALSE(dims.clamped);
}

TEST(ResolveManualHeatmapDimensions, MismatchedSettingsAreClampedToDataSize)
{
    // x_max/y_max imply a 100x100 grid (10000 elements) but only 100 elements
    // are actually available (e.g. a small connected element with stale/
    // user-edited axis maxes) - #121's out-of-bounds read.
    const auto dims = resolveManualHeatmapDimensions(100, 100, 1.0F, 1.0F, 100);
    EXPECT_LE(static_cast<std::size_t>(dims.rows) * static_cast<std::size_t>(dims.cols), 100u);
    // rows is preserved (so the configured aspect ratio survives); cols is
    // the one clamped down.
    EXPECT_EQ(dims.rows, 100);
    EXPECT_EQ(dims.cols, 1); // 100 / 100 = 1
    EXPECT_TRUE(dims.clamped);
}

TEST(ResolveManualHeatmapDimensions, ZeroDataSizeYieldsEmptyGrid)
{
    const auto dims = resolveManualHeatmapDimensions(10, 10, 1.0F, 1.0F, 0);
    EXPECT_EQ(dims.rows, 0);
    EXPECT_EQ(dims.cols, 0);
    EXPECT_FALSE(dims.clamped);
}

TEST(ResolveManualHeatmapDimensions, NonPositiveStepYieldsZeroForThatAxis)
{
    const auto dims = resolveManualHeatmapDimensions(10, 10, 0.0F, 1.0F, 100);
    EXPECT_EQ(dims.cols, 0); // x_step <= 0 => degenerate, not out of bounds
    EXPECT_EQ(dims.rows, 10);
    EXPECT_FALSE(dims.clamped);
}

TEST(ResolveManualHeatmapDimensions, ClampNeverExceedsDataSizeForLargeExtents)
{
    // Pathological extents: rows*cols would overflow if computed naively.
    const auto dims = resolveManualHeatmapDimensions(1'000'000, 1'000'000, 1.0F, 1.0F, 50);
    EXPECT_TRUE(dims.clamped);
    EXPECT_LE(static_cast<std::size_t>(dims.rows) * static_cast<std::size_t>(dims.cols), 50u);
}
