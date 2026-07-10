#include <gtest/gtest.h>
#include <memory>

#include "visualization/visualization.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// All tests here stay strictly on the headless side of the API: they never
// call Visualization::render()/renderTile() or Plot::render(), which require
// an ImGui/OpenGL context. Construction, plot-management, and parameter
// accessors are pure logic and safe to exercise directly (see
// .claude/tests/05-gui-headless.md).

static std::shared_ptr<Simulation> makeSim(const std::string& id = "vis-test")
{
    return std::make_shared<Simulation>(id, 1.0, 0.0, 0.0);
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(VisualizationConstruction, NullSimulationThrows)
{
    const std::shared_ptr<Simulation> nullSim;
    EXPECT_THROW({ Visualization vis(nullSim); }, Exception);
}

TEST(VisualizationConstruction, NullSimulationThrowsWithCorrectErrorCode)
{
    const std::shared_ptr<Simulation> nullSim;
    try
    {
        Visualization vis(nullSim);
        FAIL() << "Expected an Exception to be thrown";
    }
    catch (const Exception& e)
    {
        EXPECT_EQ(e.getErrorCode(), ErrorCode::VIS_INVALID_SIM);
    }
}

TEST(VisualizationConstruction, ValidSimulationStoresSimAndStartsEmpty)
{
    const auto sim = makeSim();
    Visualization vis(sim);
    EXPECT_EQ(vis.getSimulation(), sim);
    EXPECT_TRUE(vis.getPlots().empty());
}

// ---------------------------------------------------------------------------
// plot() overloads — add plots / data sources
// ---------------------------------------------------------------------------

TEST(VisualizationPlot, AddsLinePlotByDefault)
{
    Visualization vis(makeSim());
    vis.plot();
    ASSERT_EQ(vis.getPlots().size(), 1u);
    EXPECT_EQ(vis.getPlots().begin()->first->getType(), PlotType::LINE_PLOT);
}

TEST(VisualizationPlot, AddsHeatmapWhenRequested)
{
    Visualization vis(makeSim());
    vis.plot(PlotType::HEATMAP);
    ASSERT_EQ(vis.getPlots().size(), 1u);
    EXPECT_EQ(vis.getPlots().begin()->first->getType(), PlotType::HEATMAP);
}

TEST(VisualizationPlot, PlotWithDataSourcesStoresThem)
{
    Visualization vis(makeSim());
    const std::vector<std::pair<std::string, std::string>> data{ { "nf", "activation" }, { "nf", "output" } };
    vis.plot(data);
    ASSERT_EQ(vis.getPlots().size(), 1u);
    EXPECT_EQ(vis.getPlots().begin()->second, data);
}

TEST(VisualizationPlot, PlotWithSingleNameComponentStoresOnePair)
{
    Visualization vis(makeSim());
    vis.plot("nf", "activation");
    const auto plots = vis.getPlots();
    ASSERT_EQ(plots.size(), 1u);
    const auto data = plots.begin()->second;
    ASSERT_EQ(data.size(), 1u);
    EXPECT_EQ(data.front(), (std::pair<std::string, std::string>{ "nf", "activation" }));
}

TEST(VisualizationPlot, LinePlotWithParametersHonoursThickness)
{
    Visualization vis(makeSim());
    const PlotCommonParameters common{ PlotType::LINE_PLOT };
    const LinePlotParameters specific{ 5.0, false };
    vis.plot(common, specific, "nf", "activation");

    ASSERT_EQ(vis.getPlots().size(), 1u);
    const auto plot = std::dynamic_pointer_cast<LinePlot>(vis.getPlots().begin()->first);
    ASSERT_NE(plot, nullptr);
    EXPECT_DOUBLE_EQ(plot->getLineThickness(), 5.0);
    EXPECT_DOUBLE_EQ(plot->getAutoFit(), 0.0); // getAutoFit returns double; false -> 0.0
}

TEST(VisualizationPlot, HeatmapWithParametersHonoursScale)
{
    Visualization vis(makeSim());
    const PlotCommonParameters common{ PlotType::HEATMAP };
    const HeatmapParameters specific{ -2.0, 2.0 };
    vis.plot(common, specific, "nf", "weights");

    ASSERT_EQ(vis.getPlots().size(), 1u);
    const auto plot = std::dynamic_pointer_cast<Heatmap>(vis.getPlots().begin()->first);
    ASSERT_NE(plot, nullptr);
    const auto [scaleMin, scaleMax] = plot->getScale();
    EXPECT_DOUBLE_EQ(scaleMin, -2.0);
    EXPECT_DOUBLE_EQ(scaleMax, 2.0);
}

TEST(VisualizationPlot, MismatchedSpecificParametersDoesNotAddPlot)
{
    // LINE_PLOT type but HeatmapParameters -> logged and skipped, not added.
    Visualization vis(makeSim());
    const PlotCommonParameters common{ PlotType::LINE_PLOT };
    const HeatmapParameters wrongSpecific{};
    vis.plot(common, wrongSpecific, "nf", "activation");
    EXPECT_TRUE(vis.getPlots().empty());
}

TEST(VisualizationPlot, AddDataToExistingPlotById)
{
    Visualization vis(makeSim());
    vis.plot(); // creates plot #0 (or whatever the running counter is)
    const int id = vis.getPlots().begin()->first->getUniqueIdentifier();

    vis.plot(id, "nf", "activation");
    EXPECT_EQ(vis.getPlots().begin()->second.size(), 1u);

    vis.plot(id, std::vector<std::pair<std::string, std::string>>{ { "nf", "output" } });
    EXPECT_EQ(vis.getPlots().begin()->second.size(), 2u);
}

TEST(VisualizationPlot, AddDataToUnknownIdIsNoOp)
{
    Visualization vis(makeSim());
    vis.plot();
    const auto sizeBefore = vis.getPlots().begin()->second.size();

    vis.plot(-999999, "nf", "activation");
    EXPECT_EQ(vis.getPlots().begin()->second.size(), sizeBefore);
}

// ---------------------------------------------------------------------------
// removePlot / removeAllPlots / removePlottingDataFromPlot
// ---------------------------------------------------------------------------

TEST(VisualizationRemove, RemovePlotById)
{
    Visualization vis(makeSim());
    vis.plot();
    const int id = vis.getPlots().begin()->first->getUniqueIdentifier();

    vis.removePlot(id);
    EXPECT_TRUE(vis.getPlots().empty());
}

TEST(VisualizationRemove, RemoveUnknownIdIsNoOp)
{
    Visualization vis(makeSim());
    vis.plot();
    EXPECT_EQ(vis.getPlots().size(), 1u);

    vis.removePlot(-999999);
    EXPECT_EQ(vis.getPlots().size(), 1u);
}

TEST(VisualizationRemove, RemoveAllPlotsEmptiesMap)
{
    Visualization vis(makeSim());
    vis.plot();
    vis.plot();
    vis.plot(PlotType::HEATMAP);
    ASSERT_EQ(vis.getPlots().size(), 3u);

    vis.removeAllPlots();
    EXPECT_TRUE(vis.getPlots().empty());
}

TEST(VisualizationRemove, RemovePlottingDataFromPlotRemovesExactPair)
{
    Visualization vis(makeSim());
    const std::vector<std::pair<std::string, std::string>> data{ { "nf", "activation" }, { "nf", "output" } };
    vis.plot(data);
    const int id = vis.getPlots().begin()->first->getUniqueIdentifier();

    vis.removePlottingDataFromPlot(id, { "nf", "activation" });
    const auto remaining = vis.getPlots().begin()->second;
    ASSERT_EQ(remaining.size(), 1u);
    EXPECT_EQ(remaining.front(), (std::pair<std::string, std::string>{ "nf", "output" }));
}

TEST(VisualizationRemove, RemovePlottingDataUnknownPairIsNoOp)
{
    Visualization vis(makeSim());
    const std::vector<std::pair<std::string, std::string>> data{ { "nf", "activation" } };
    vis.plot(data);
    const int id = vis.getPlots().begin()->first->getUniqueIdentifier();

    vis.removePlottingDataFromPlot(id, { "does-not-exist", "component" });
    EXPECT_EQ(vis.getPlots().begin()->second, data);
}

// ---------------------------------------------------------------------------
// Plot identity — unique, strictly increasing IDs
// ---------------------------------------------------------------------------

TEST(PlotIdentity, UniqueIdentifiersIncreaseAcrossConstructions)
{
    const LinePlot a;
    const LinePlot b;
    EXPECT_GT(b.getUniqueIdentifier(), a.getUniqueIdentifier());
}

// ---------------------------------------------------------------------------
// PlotDimensions / PlotAnnotations / PlotCommonParameters
// ---------------------------------------------------------------------------

TEST(PlotDimensions, DefaultIsLegal)
{
    const PlotDimensions dims;
    EXPECT_TRUE(dims.isLegal());
}

TEST(PlotDimensions, InvertedRangeResetsToDefaultAndIsLegal)
{
    // The constructor detects xMin >= xMax and resets to the default range
    // rather than keeping the illegal input.
    const PlotDimensions dims{ 100.0, 0.0, -10.0, 10.0, 1.0, 1.0 };
    EXPECT_TRUE(dims.isLegal());
    EXPECT_DOUBLE_EQ(dims.xMin, 0.0);
    EXPECT_DOUBLE_EQ(dims.xMax, 100.0);
}

TEST(PlotDimensions, ZeroStepResetsToOne)
{
    const PlotDimensions dims{ 0.0, 100.0, -10.0, 10.0, 0.0, 1.0 };
    EXPECT_DOUBLE_EQ(dims.xStep, 1.0);
    EXPECT_TRUE(dims.isLegal());
}

TEST(PlotDimensions, EqualityAndInequality)
{
    const PlotDimensions a{ 0.0, 100.0, -10.0, 10.0, 1.0, 1.0 };
    const PlotDimensions b{ 0.0, 100.0, -10.0, 10.0, 1.0, 1.0 };
    const PlotDimensions c{ 0.0, 50.0, -10.0, 10.0, 1.0, 1.0 };
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(PlotDimensions, ToStringContainsValues)
{
    const PlotDimensions dims{ 0.0, 100.0, -10.0, 10.0, 1.0, 1.0 };
    const auto str = dims.toString();
    EXPECT_NE(str.find("100"), std::string::npos);
}

TEST(PlotAnnotations, EqualityAndInequality)
{
    const PlotAnnotations a{ "Title", "X", "Y" };
    const PlotAnnotations b{ "Title", "X", "Y" };
    const PlotAnnotations c{ "Other", "X", "Y" };
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(PlotAnnotations, ToStringContainsTitle)
{
    const PlotAnnotations ann{ "MyTitle", "X", "Y" };
    EXPECT_NE(ann.toString().find("MyTitle"), std::string::npos);
}

TEST(PlotCommonParameters, EqualityIgnoresType)
{
    // operator== only compares dimensions and annotations, not type.
    const PlotCommonParameters a{ PlotType::LINE_PLOT };
    const PlotCommonParameters b{ PlotType::HEATMAP };
    EXPECT_TRUE(a == b);
}

TEST(PlotCommonParameters, ToStringContainsTypeName)
{
    const PlotCommonParameters common{ PlotType::HEATMAP };
    EXPECT_NE(common.toString().find("heatmap"), std::string::npos);
}

// ---------------------------------------------------------------------------
// LinePlotParameters / HeatmapParameters — equality, toString, get/set
// ---------------------------------------------------------------------------

TEST(LinePlotParameters, EqualityAndInequality)
{
    const LinePlotParameters a{ 2.0, true };
    const LinePlotParameters b{ 2.0, true };
    const LinePlotParameters c{ 3.0, true };
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(LinePlotParameters, ToStringContainsThickness)
{
    const LinePlotParameters params{ 4.5, true };
    EXPECT_NE(params.toString().find("4.5"), std::string::npos);
}

TEST(LinePlot, SetLineThicknessAndAutoFitRoundTrip)
{
    LinePlot plot;
    plot.setLineThickness(7.5);
    plot.setAutoFit(false);
    EXPECT_DOUBLE_EQ(plot.getLineThickness(), 7.5);
    EXPECT_DOUBLE_EQ(plot.getAutoFit(), 0.0);
}

TEST(HeatmapParameters, EqualityAndInequality)
{
    const HeatmapParameters a{ 0.0, 1.0 };
    const HeatmapParameters b{ 0.0, 1.0 };
    const HeatmapParameters c{ -1.0, 1.0 };
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(HeatmapParameters, ToStringContainsScaleValues)
{
    const HeatmapParameters params{ -3.0, 3.0 };
    const auto str = params.toString();
    EXPECT_NE(str.find("3.000000"), std::string::npos);
}

TEST(Heatmap, SetScaleAndGetScaleRoundTrip)
{
    Heatmap heatmap;
    heatmap.setScale(-5.0, 5.0);
    const auto [scaleMin, scaleMax] = heatmap.getScale();
    EXPECT_DOUBLE_EQ(scaleMin, -5.0);
    EXPECT_DOUBLE_EQ(scaleMax, 5.0);
}

TEST(Heatmap, SetScaleInvertedRangeResetsToDefault)
{
    Heatmap heatmap;
    heatmap.setScale(10.0, 0.0); // min >= max -> reset to [0,1]
    const auto [scaleMin, scaleMax] = heatmap.getScale();
    EXPECT_DOUBLE_EQ(scaleMin, 0.0);
    EXPECT_DOUBLE_EQ(scaleMax, 1.0);
}
