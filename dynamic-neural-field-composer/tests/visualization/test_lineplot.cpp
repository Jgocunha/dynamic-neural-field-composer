#include <gtest/gtest.h>
#include <vector>
#include <cmath>

#include "visualization/lineplot.h"

using namespace dnf_composer;

// LinePlot construction tests
TEST(LinePlot, DefaultConstruction)
{
	const LinePlot plot;
	EXPECT_EQ(plot.getType(), PlotType::LINE_PLOT);
	EXPECT_EQ(plot.getLineThickness(), 2.0);
	EXPECT_EQ(plot.getAutoFit(), 1.0);
}

TEST(LinePlot, WithCustomCommonParameters)
{
	const PlotCommonParameters common(PlotType::LINE_PLOT);
	const LinePlot plot(common);
	EXPECT_EQ(plot.getType(), PlotType::LINE_PLOT);
}

TEST(LinePlot, WithCustomDimensions)
{
	const PlotDimensions dims(0.0, 100.0, -20.0, 20.0, 1.0, 1.0);
	const PlotCommonParameters common(PlotType::LINE_PLOT, dims, PlotAnnotations());
	const LinePlot plot(common);
	const auto d = plot.getDimensions();
	EXPECT_EQ(d.xMax, 100.0);
}

TEST(LinePlot, WithCustomAnnotations)
{
	const PlotAnnotations ann("Neural Activity", "Position", "Activation");
	const PlotCommonParameters common(PlotType::LINE_PLOT, PlotDimensions(), ann);
	const LinePlot plot(common);
	EXPECT_EQ(plot.getAnnotations().title, "Neural Activity");
}

TEST(LinePlot, WithCustomParameters)
{
	const PlotCommonParameters common(PlotType::LINE_PLOT);
	const LinePlotParameters specific(5.0, false);
	const LinePlot plot(common, specific);
	EXPECT_EQ(plot.getLineThickness(), 5.0);
	EXPECT_EQ(plot.getAutoFit(), 0.0);
}

// Guard against a later "symmetry" refactor silently changing LinePlot's
// throw into a normalize-and-log, which would be a breaking API change for
// LinePlot's existing callers. See heatmap.h for why Heatmap does not throw
// on a mismatched type (#143).
TEST(LinePlot, WithWrongTypeThrows)
{
	const PlotCommonParameters common(PlotType::HEATMAP);
	EXPECT_THROW(LinePlot plot(common), std::invalid_argument);
}

TEST(LinePlot, WithNegativeDimensions)
{
	const PlotDimensions dims(-100.0, 100.0, -100.0, 100.0, 10.0, 10.0);
	const PlotCommonParameters common(PlotType::LINE_PLOT, dims, PlotAnnotations());
	const LinePlot plot(common);
	EXPECT_EQ(plot.getDimensions(), dims);
}

TEST(LinePlot, WithLargeDimensions)
{
	const PlotDimensions dims(0.0, 1000000.0, -1000000.0, 1000000.0, 1000.0, 1000.0);
	const PlotCommonParameters common(PlotType::LINE_PLOT, dims, PlotAnnotations());
	const LinePlot plot(common);
	EXPECT_TRUE(plot.getDimensions().isLegal());
}

TEST(LinePlot, WithSmallDimensions)
{
	const PlotDimensions dims(0.0, 0.000001, 0.0, 0.000001, 0.00000001, 0.00000001);
	const PlotCommonParameters common(PlotType::LINE_PLOT, dims, PlotAnnotations());
	const LinePlot plot(common);
	EXPECT_TRUE(plot.getDimensions().isLegal());
}

// Line thickness tests
TEST(LinePlot, SetLineThickness)
{
	LinePlot plot;
	plot.setLineThickness(7.5);
	EXPECT_EQ(plot.getLineThickness(), 7.5);
}

TEST(LinePlot, SetLineThicknessZero)
{
	LinePlot plot;
	plot.setLineThickness(0.0);
	EXPECT_EQ(plot.getLineThickness(), 0.0);
}

TEST(LinePlot, SetLineThicknessLarge)
{
	LinePlot plot;
	plot.setLineThickness(1000.0);
	EXPECT_EQ(plot.getLineThickness(), 1000.0);
}

TEST(LinePlot, SetLineThicknessSmall)
{
	LinePlot plot;
	plot.setLineThickness(0.0000000001);
	EXPECT_EQ(plot.getLineThickness(), 0.0000000001);
}

TEST(LinePlot, SetLineThicknessMultipleTimes)
{
	LinePlot plot;
	plot.setLineThickness(1.0);
	EXPECT_EQ(plot.getLineThickness(), 1.0);

	plot.setLineThickness(5.0);
	EXPECT_EQ(plot.getLineThickness(), 5.0);

	plot.setLineThickness(2.5);
	EXPECT_EQ(plot.getLineThickness(), 2.5);
}

// AutoFit tests
TEST(LinePlot, SetAutoFitTrue)
{
	LinePlot plot;
	plot.setAutoFit(true);
	EXPECT_EQ(plot.getAutoFit(), 1.0);
}

TEST(LinePlot, SetAutoFitFalse)
{
	LinePlot plot;
	plot.setAutoFit(false);
	EXPECT_EQ(plot.getAutoFit(), 0.0);
}

TEST(LinePlot, SetAutoFitMultipleTimes)
{
	LinePlot plot;
	plot.setAutoFit(false);
	EXPECT_EQ(plot.getAutoFit(), 0.0);

	plot.setAutoFit(true);
	EXPECT_EQ(plot.getAutoFit(), 1.0);

	plot.setAutoFit(false);
	EXPECT_EQ(plot.getAutoFit(), 0.0);
}

TEST(LinePlot, SetThicknessAndAutoFitRoundTrip)
{
	LinePlot plot;
	plot.setLineThickness(7.5);
	plot.setAutoFit(false);
	EXPECT_EQ(plot.getLineThickness(), 7.5);
	EXPECT_EQ(plot.getAutoFit(), 0.0);
}

// Plot base class tests
TEST(LinePlot, GetType)
{
	const LinePlot plot;
	EXPECT_EQ(plot.getType(), PlotType::LINE_PLOT);
}

TEST(LinePlot, GetUniqueIdentifier)
{
	const LinePlot p1;
	const LinePlot p2;
	const LinePlot p3;

	EXPECT_NE(p1.getUniqueIdentifier(), p2.getUniqueIdentifier());
	EXPECT_NE(p2.getUniqueIdentifier(), p3.getUniqueIdentifier());
	EXPECT_LT(p1.getUniqueIdentifier(), p2.getUniqueIdentifier());
	EXPECT_LT(p2.getUniqueIdentifier(), p3.getUniqueIdentifier());
}

TEST(LinePlot, SetAndGetDimensions)
{
	LinePlot plot;
	const PlotDimensions newDims(0.0, 200.0, -50.0, 50.0, 2.0, 2.0);
	plot.setDimensions(newDims);
	EXPECT_EQ(plot.getDimensions(), newDims);
}

TEST(LinePlot, SetAndGetAnnotations)
{
	LinePlot plot;
	const PlotAnnotations newAnn("Custom Plot", "Time", "Voltage");
	plot.setAnnotations(newAnn);
	EXPECT_EQ(plot.getAnnotations(), newAnn);
}

TEST(LinePlot, ToStringNotEmpty)
{
	const LinePlot plot;
	EXPECT_FALSE(plot.toString().empty());
}

TEST(LinePlot, ToStringContainsIdentifier)
{
	const LinePlot plot;
	const auto str = plot.toString();
	const auto id_str = std::to_string(plot.getUniqueIdentifier());
	EXPECT_NE(str.find(id_str), std::string::npos);
}

TEST(LinePlot, ToStringContainsThickness)
{
	LinePlot plot;
	plot.setLineThickness(3.5);
	const auto str = plot.toString();
	EXPECT_NE(str.find("3.5"), std::string::npos);
}

TEST(LinePlot, ThicknessAndAutoFitIndependent)
{
	LinePlot plot;

	plot.setLineThickness(5.0);
	EXPECT_EQ(plot.getAutoFit(), 1.0);

	plot.setAutoFit(false);
	EXPECT_EQ(plot.getLineThickness(), 5.0);

	plot.setLineThickness(3.0);
	EXPECT_EQ(plot.getAutoFit(), 0.0);
}

TEST(LinePlot, DimensionsAndParametersIndependent)
{
	LinePlot plot;

	const PlotDimensions dims(0.0, 50.0, -25.0, 25.0, 1.0, 1.0);
	plot.setDimensions(dims);

	EXPECT_EQ(plot.getLineThickness(), 2.0);
	EXPECT_EQ(plot.getAutoFit(), 1.0);

	plot.setLineThickness(7.0);
	EXPECT_EQ(plot.getDimensions(), dims);
}

TEST(LinePlot, MultipleInstancesIndependent)
{
	LinePlot p1;
	LinePlot p2;
	LinePlot p3;

	p1.setLineThickness(1.0);
	p2.setLineThickness(5.0);
	p3.setLineThickness(10.0);

	EXPECT_EQ(p1.getLineThickness(), 1.0);
	EXPECT_EQ(p2.getLineThickness(), 5.0);
	EXPECT_EQ(p3.getLineThickness(), 10.0);
}

TEST(LinePlot, MultipleInstancesAutoFitIndependent)
{
	LinePlot p1;
	LinePlot p2;
	LinePlot p3;

	p1.setAutoFit(true);
	p2.setAutoFit(false);
	p3.setAutoFit(true);

	EXPECT_EQ(p1.getAutoFit(), 1.0);
	EXPECT_EQ(p2.getAutoFit(), 0.0);
	EXPECT_EQ(p3.getAutoFit(), 1.0);
}

// LinePlotParameters tests
TEST(LinePlotParameters, DefaultEquality)
{
	const LinePlotParameters a;
	const LinePlotParameters b;
	EXPECT_TRUE(a == b);
}

TEST(LinePlotParameters, CustomEquality)
{
	const LinePlotParameters a(2.0, true);
	const LinePlotParameters b(2.0, true);
	EXPECT_TRUE(a == b);
}



TEST(LinePlotParameters, ToStringNotEmpty)
{
	const LinePlotParameters params;
	EXPECT_FALSE(params.toString().empty());
}


TEST(LinePlotParameters, ToStringContainsAutoFit)
{
	const LinePlotParameters params(2.0, true);
	const auto str = params.toString();
	EXPECT_NE(str.find("true"), std::string::npos);
}
