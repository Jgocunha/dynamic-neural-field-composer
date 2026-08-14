#include <gtest/gtest.h>
#include <vector>
#include <cmath>

#include "visualization/heatmap.h"
#include "visualization/lineplot.h"

using namespace dnf_composer;

// Heatmap construction tests
TEST(Heatmap, DefaultConstruction)
{
	const Heatmap heatmap;
	EXPECT_EQ(heatmap.getType(), PlotType::HEATMAP);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, 0.0);
	EXPECT_EQ(max, 1.0);
}

TEST(Heatmap, WithCustomCommonParameters)
{
	const PlotCommonParameters common(PlotType::HEATMAP);
	const Heatmap heatmap(common);
	EXPECT_EQ(heatmap.getType(), PlotType::HEATMAP);
}

TEST(Heatmap, WithCustomDimensions)
{
	const PlotDimensions dims(0.0, 64.0, 0.0, 64.0, 1.0, 1.0);
	const PlotCommonParameters common(PlotType::HEATMAP, dims, PlotAnnotations());
	const Heatmap heatmap(common);
	const auto d = heatmap.getDimensions();
	EXPECT_EQ(d.xMax, 64.0);
	EXPECT_EQ(d.yMax, 64.0);
}

TEST(Heatmap, WithCustomAnnotations)
{
	const PlotAnnotations ann("Weight Matrix", "Output", "Input");
	const PlotCommonParameters common(PlotType::HEATMAP, PlotDimensions(), ann);
	const Heatmap heatmap(common);
	EXPECT_EQ(heatmap.getAnnotations().title, "Weight Matrix");
}

TEST(Heatmap, WithCustomParameters)
{
	const PlotCommonParameters common(PlotType::HEATMAP);
	const HeatmapParameters specific(-10.0, 10.0);
	const Heatmap heatmap(common, specific);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, -10.0);
	EXPECT_EQ(max, 10.0);
}

TEST(Heatmap, WithNegativeDimensions)
{
	const PlotDimensions dims(-100.0, 100.0, -100.0, 100.0, 10.0, 10.0);
	const PlotCommonParameters common(PlotType::HEATMAP, dims, PlotAnnotations());
	const Heatmap heatmap(common);
	EXPECT_EQ(heatmap.getDimensions(), dims);
}

TEST(Heatmap, WithLargeDimensions)
{
	const PlotDimensions dims(0.0, 1000000.0, 0.0, 1000000.0, 1e3, 1e3);
	const PlotCommonParameters common(PlotType::HEATMAP, dims, PlotAnnotations());
	const Heatmap heatmap(common);
	EXPECT_TRUE(heatmap.getDimensions().isLegal());
}

TEST(Heatmap, WithSmallDimensions)
{
	const PlotDimensions dims(0.0, 0.000001, 0.0, 0.000001, 0.00000001, 0.00000001);
	const PlotCommonParameters common(PlotType::HEATMAP, dims, PlotAnnotations());
	const Heatmap heatmap(common);
	EXPECT_TRUE(heatmap.getDimensions().isLegal());
}

// Scale getter/setter tests
TEST(Heatmap, SetScaleAndGet)
{
	Heatmap heatmap;
	heatmap.setScale(-5.0, 5.0);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, -5.0);
	EXPECT_EQ(max, 5.0);
}

TEST(Heatmap, SetScaleNegative)
{
	Heatmap heatmap;
	heatmap.setScale(-100.0, -50.0);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, -100.0);
	EXPECT_EQ(max, -50.0);
}

TEST(Heatmap, SetScaleZero)
{
	Heatmap heatmap;
	heatmap.setScale(0.0, 0.0);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, 0.0);
	EXPECT_EQ(max, 1.0);
}

TEST(Heatmap, SetScaleSmallRange)
{
	Heatmap heatmap;
	heatmap.setScale(0.0, 0.0000000001);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(max, 0.0000000001);
}

TEST(Heatmap, SetScaleLargeRange)
{
	Heatmap heatmap;
	heatmap.setScale(0.0, 10000000000.0);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(max, 10000000000.0);
}

TEST(Heatmap, SetScaleInvertedResetsToDefault)
{
	Heatmap heatmap;
	heatmap.setScale(10.0, 0.0);
	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, 0.0);
	EXPECT_EQ(max, 1.0);
}

TEST(Heatmap, SetScaleMultipleTimes)
{
	Heatmap heatmap;

	heatmap.setScale(0.0, 10.0);
	auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, 0.0);
	EXPECT_EQ(max, 10.0);

	heatmap.setScale(-5.0, 5.0);
	std::tie(min, max) = heatmap.getScale();
	EXPECT_EQ(min, -5.0);
	EXPECT_EQ(max, 5.0);
}

// Dimension hint tests
TEST(Heatmap, SetDimensionHint)
{
	Heatmap heatmap;
	heatmap.setDimensionHint(64, 64);
}

TEST(Heatmap, SetDimensionHintMultipleTimes)
{
	Heatmap heatmap;
	heatmap.setDimensionHint(0, 0);
	heatmap.setDimensionHint(1, 1);
	heatmap.setDimensionHint(1000, 1000);
}

// Plot base class tests
TEST(Heatmap, GetType)
{
	const Heatmap heatmap;
	EXPECT_EQ(heatmap.getType(), PlotType::HEATMAP);
}

TEST(Heatmap, GetUniqueIdentifier)
{
	const Heatmap h1;
	const Heatmap h2;
	EXPECT_NE(h1.getUniqueIdentifier(), h2.getUniqueIdentifier());
	EXPECT_LT(h1.getUniqueIdentifier(), h2.getUniqueIdentifier());
}

TEST(Heatmap, SetAndGetDimensions)
{
	Heatmap heatmap;
	const PlotDimensions newDims(0.0, 256.0, 0.0, 256.0, 2.0, 2.0);
	heatmap.setDimensions(newDims);
	EXPECT_EQ(heatmap.getDimensions(), newDims);
}

TEST(Heatmap, SetAndGetAnnotations)
{
	Heatmap heatmap;
	const PlotAnnotations newAnn("Custom Heatmap", "Output", "Input");
	heatmap.setAnnotations(newAnn);
	EXPECT_EQ(heatmap.getAnnotations(), newAnn);
}

TEST(Heatmap, ToStringNotEmpty)
{
	const Heatmap heatmap;
	EXPECT_FALSE(heatmap.toString().empty());
}

TEST(Heatmap, ToStringContainsIdentifier)
{
	const Heatmap heatmap;
	const auto str = heatmap.toString();
	const auto id_str = std::to_string(heatmap.getUniqueIdentifier());
	EXPECT_NE(str.find(id_str), std::string::npos);
}

TEST(Heatmap, DimensionsAndScaleAreIndependent)
{
	Heatmap heatmap;

	const PlotDimensions dims(0.0, 50.0, 0.0, 50.0, 1.0, 1.0);
	heatmap.setDimensions(dims);

	const auto [min, max] = heatmap.getScale();
	EXPECT_EQ(min, 0.0);
	EXPECT_EQ(max, 1.0);

	heatmap.setScale(-5.0, 5.0);
	EXPECT_EQ(heatmap.getDimensions(), dims);
}

TEST(Heatmap, MultipleInstancesIndependent)
{
	Heatmap h1;
	Heatmap h2;
	Heatmap h3;

	h1.setScale(0.0, 1.0);
	h2.setScale(-10.0, 10.0);
	h3.setScale(0.0, 100.0);

	const auto [min1, max1] = h1.getScale();
	const auto [min2, max2] = h2.getScale();
	const auto [min3, max3] = h3.getScale();

	EXPECT_EQ(min1, 0.0);
	EXPECT_EQ(max1, 1.0);
	EXPECT_EQ(min2, -10.0);
	EXPECT_EQ(max2, 10.0);
	EXPECT_EQ(min3, 0.0);
	EXPECT_EQ(max3, 100.0);
}

// HeatmapParameters tests
TEST(HeatmapParameters, DefaultEquality)
{
	const HeatmapParameters a;
	const HeatmapParameters b;
	EXPECT_TRUE(a == b);
}

TEST(HeatmapParameters, CustomEquality)
{
	const HeatmapParameters a(0.0, 1.0);
	const HeatmapParameters b(0.0, 1.0);
	EXPECT_TRUE(a == b);
}

TEST(HeatmapParameters, InequalityDifferentMin)
{
	const HeatmapParameters a(0.0, 1.0);
	const HeatmapParameters b(-1.0, 1.0);
	EXPECT_FALSE(a == b);
}

TEST(HeatmapParameters, InequalityDifferentMax)
{
	const HeatmapParameters a(0.0, 1.0);
	const HeatmapParameters b(0.0, 2.0);
	EXPECT_FALSE(a == b);
}

TEST(HeatmapParameters, ToStringNotEmpty)
{
	const HeatmapParameters params;
	EXPECT_FALSE(params.toString().empty());
}

TEST(HeatmapParameters, ToStringContainsValues)
{
	const HeatmapParameters params(-3.0, 3.0);
	const auto str = params.toString();
	EXPECT_NE(str.find("3"), std::string::npos);
}

// ---------------------------------------------------------------------------
// PlotType handling: Heatmap and LinePlot are NOT symmetric.
//
// LinePlot's constructor throws std::invalid_argument when
// commonParameters.type != PlotType::LINE_PLOT (see lineplot.cpp) -- a mismatch
// there is treated as a programmer error the caller must fix. Heatmap instead
// normalizes: a mismatched parameters.type is silently corrected to
// PlotType::HEATMAP (with a warning logged) rather than thrown. Symmetry with
// LinePlot was considered and rejected: throwing from a constructor that
// previously succeeded would turn a working caller's program into a crash on
// upgrade, which this codebase treats as a hard no (see PR description).
// Normalizing keeps existing callers running while making getType() honest --
// a Heatmap always reports PlotType::HEATMAP (#143).
// ---------------------------------------------------------------------------

TEST(Heatmap, MismatchedPlotTypeStillDoesNotThrow)
{
	const PlotCommonParameters common(PlotType::LINE_PLOT);
	EXPECT_NO_THROW({ const Heatmap heatmap(common); });
}

TEST(Heatmap, MismatchedPlotTypeIsNormalizedToHeatmap)
{
	const PlotCommonParameters common(PlotType::LINE_PLOT);
	const Heatmap heatmap(common);
	EXPECT_EQ(heatmap.getType(), PlotType::HEATMAP);
}

TEST(Heatmap, MismatchedPlotTypeLogsAWarning)
{
	const PlotCommonParameters common(PlotType::LINE_PLOT);
	::testing::internal::CaptureStdout();
	const Heatmap heatmap(common);
	const std::string out = ::testing::internal::GetCapturedStdout();
	EXPECT_NE(out.find("PlotType"), std::string::npos);
}

TEST(Heatmap, MatchingPlotTypeIsUnchangedAndLogsNoWarning)
{
	const PlotCommonParameters common(PlotType::HEATMAP);
	::testing::internal::CaptureStdout();
	const Heatmap heatmap(common);
	const std::string out = ::testing::internal::GetCapturedStdout();
	EXPECT_EQ(heatmap.getType(), PlotType::HEATMAP);
	EXPECT_TRUE(out.empty());
}

// Guard against a later "symmetry" refactor silently changing LinePlot's
// throw into a normalize-and-log, which would be a breaking API change for
// LinePlot's existing callers.
TEST(LinePlot, MismatchedPlotTypeStillThrows)
{
	const PlotCommonParameters common(PlotType::HEATMAP);
	EXPECT_THROW({ const LinePlot linePlot(common); }, std::invalid_argument);
}
