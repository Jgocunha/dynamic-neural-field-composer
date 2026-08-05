#include <gtest/gtest.h>
#include <memory>

#include "visualization/plot.h"
#include "visualization/lineplot.h"
#include "visualization/heatmap.h"

using namespace dnf_composer;

// ---------------------------------------------------------------------------
// Plot base class tests (via concrete subclasses)
// ---------------------------------------------------------------------------

TEST(Plot, GetUniqueIdentifierIsNonNegative)
{
	const LinePlot plot;
	EXPECT_GE(plot.getUniqueIdentifier(), 0);
}

TEST(Plot, UniqueIdentifiersAreStrictlyIncreasing)
{
	// Clear any previous plots by checking a sequence
	const LinePlot a;
	const LinePlot b;
	const LinePlot c;
	EXPECT_LT(a.getUniqueIdentifier(), b.getUniqueIdentifier());
	EXPECT_LT(b.getUniqueIdentifier(), c.getUniqueIdentifier());
}

TEST(Plot, ConstructWithDefaultCommonParameters)
{
	const LinePlot plot;
	EXPECT_EQ(plot.getType(), PlotType::LINE_PLOT);
	const auto dims = plot.getDimensions();
	EXPECT_TRUE(dims.isLegal());
	const auto annot = plot.getAnnotations();
	EXPECT_FALSE(annot.title.empty());
}

TEST(Plot, ConstructWithCustomCommonParameters)
{
	const PlotDimensions dims{ 0.0, 50.0, -5.0, 5.0, 0.5, 0.5 };
	const PlotAnnotations ann{ "Custom Title", "Custom X", "Custom Y" };
	const PlotCommonParameters params{ PlotType::LINE_PLOT, dims, ann };
	const LinePlot plot{ params };
	EXPECT_EQ(plot.getType(), PlotType::LINE_PLOT);
	const auto d = plot.getDimensions();
	EXPECT_DOUBLE_EQ(d.xMax, 50.0);
}

TEST(Plot, SetDimensions)
{
	LinePlot plot;
	const PlotDimensions newDims{ 10.0, 90.0, -50.0, 50.0, 2.0, 2.0 };
	plot.setDimensions(newDims);
	const auto dims = plot.getDimensions();
	EXPECT_EQ(dims, newDims);
}

TEST(Plot, SetAnnotations)
{
	LinePlot plot;
	const PlotAnnotations newAnn{ "New Title", "New X", "New Y" };
	plot.setAnnotations(newAnn);
	const auto ann = plot.getAnnotations();
	EXPECT_EQ(ann, newAnn);
}

TEST(Plot, GetTypeHonorsConstructorParameter)
{
	const PlotCommonParameters lineParams{ PlotType::LINE_PLOT };
	const LinePlot linePlot{ lineParams };
	EXPECT_EQ(linePlot.getType(), PlotType::LINE_PLOT);

	const PlotCommonParameters heatmapParams{ PlotType::HEATMAP };
	const Heatmap heatmap{ heatmapParams };
	EXPECT_EQ(heatmap.getType(), PlotType::HEATMAP);
}

TEST(Plot, DimensionsCanBeModifiedMultipleTimes)
{
	LinePlot plot;

	PlotDimensions dims1{ 0.0, 100.0, -10.0, 10.0, 1.0, 1.0 };
	plot.setDimensions(dims1);
	EXPECT_EQ(plot.getDimensions(), dims1);

	PlotDimensions dims2{ 0.0, 50.0, -5.0, 5.0, 0.5, 0.5 };
	plot.setDimensions(dims2);
	EXPECT_EQ(plot.getDimensions(), dims2);
}

TEST(Plot, AnnotationsCanBeModifiedMultipleTimes)
{
	LinePlot plot;

	PlotAnnotations ann1{ "Title1", "X1", "Y1" };
	plot.setAnnotations(ann1);
	EXPECT_EQ(plot.getAnnotations(), ann1);

	PlotAnnotations ann2{ "Title2", "X2", "Y2" };
	plot.setAnnotations(ann2);
	EXPECT_EQ(plot.getAnnotations(), ann2);
}

TEST(Plot, ToStringIsNotEmpty)
{
	const LinePlot plot;
	EXPECT_FALSE(plot.toString().empty());
}

TEST(Plot, HeatmapToStringIsNotEmpty)
{
	const Heatmap heatmap;
	EXPECT_FALSE(heatmap.toString().empty());
}

TEST(Plot, ToStringContainsUniqueIdentifier)
{
	const LinePlot plot;
	const auto str = plot.toString();
	const auto id_str = std::to_string(plot.getUniqueIdentifier());
	EXPECT_NE(str.find(id_str), std::string::npos);
}

// Test edge case: plot with minimal dimensions
TEST(Plot, PlotWithMinimalLegalDimensions)
{
	const PlotDimensions dims{ 0.0, 0.1, 0.0, 0.1, 0.01, 0.01 };
	const PlotCommonParameters params{ PlotType::LINE_PLOT, dims, PlotAnnotations{} };
	const LinePlot plot{ params };
	EXPECT_EQ(plot.getType(), PlotType::LINE_PLOT);
	const auto d = plot.getDimensions();
	EXPECT_TRUE(d.isLegal());
}

// Test edge case: plot with large dimensions
TEST(Plot, PlotWithLargeDimensions)
{
	const PlotDimensions dims{ 0.0, 1e6, -1e6, 1e6, 1e3, 1e3 };
	const PlotCommonParameters params{ PlotType::LINE_PLOT, dims, PlotAnnotations{} };
	const LinePlot plot{ params };
	EXPECT_EQ(plot.getType(), PlotType::LINE_PLOT);
	const auto d = plot.getDimensions();
	EXPECT_TRUE(d.isLegal());
}

// Test edge case: empty title and labels
TEST(Plot, PlotWithEmptyAnnotations)
{
	const PlotCommonParameters params{
		PlotType::LINE_PLOT,
		PlotDimensions{},
		PlotAnnotations{ "", "", "" }
	};
	const LinePlot plot{ params };
	const auto ann = plot.getAnnotations();
	EXPECT_EQ(ann.title, "");
	EXPECT_EQ(ann.x_label, "");
	EXPECT_EQ(ann.y_label, "");
}
