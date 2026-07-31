#include <gtest/gtest.h>
#include <memory>

#include "ui_test_harness.h"
#include "user_interface/plot_control_window.h"
#include "visualization/visualization.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "elements/activation_function.h"

using namespace dnf_composer;
using namespace dnf_composer::user_interface;
using namespace dnf_composer::element;

namespace
{
	std::shared_ptr<Simulation> makeTestSimulation(const std::string& id = "plot-ctrl-test")
	{
		auto simulation = std::make_shared<Simulation>(id, 1.0, 0.0, 0.0);
		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common{ std::string("field"), 100 };
		simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));
		simulation->init();
		return simulation;
	}
}

// ---------------------------------------------------------------------------
// PlotControlWindow construction and basic rendering
// ---------------------------------------------------------------------------

TEST(PlotControlWindow, ConstructionWithVisualization)
{
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	EXPECT_NO_THROW({
		PlotControlWindow window{ vis };
	});
}

TEST(PlotControlWindow, RenderWithEmptyVisualization)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderWithSingleLinePlot)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot();  // Add a line plot

	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderWithSingleHeatmap)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot(PlotType::HEATMAP);  // Add a heatmap

	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderWithMultiplePlots)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot();
	vis->plot();
	vis->plot(PlotType::HEATMAP);

	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderWithPlotsAndData)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot("field", "activation");
	vis->plot("field", "output");

	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderMultipleFrames)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot();

	PlotControlWindow window{ vis };

	gui.frames(2, [&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderContentWithEmptyVisualization)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.renderContent();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderContentWithPlots)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot();
	vis->plot();

	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.renderContent();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderStaysStableAfterSimulationSteps)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	for (int i = 0; i < 5; ++i)
		sim->step();

	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot();

	PlotControlWindow window{ vis };

	gui.frames(2, [&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderWithComplexVisualization)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);

	// Build a complex visualization with multiple plots and data
	vis->plot();
	const int id1 = vis->getPlots().begin()->first->getUniqueIdentifier();
	vis->plot(id1, "field", "activation");
	vis->plot(id1, "field", "output");

	vis->plot(PlotType::HEATMAP);
	int id2 = -1;
	for (const auto& [plot, _] : vis->getPlots())
		id2 = std::max(id2, plot->getUniqueIdentifier());
	vis->plot(id2, "field", "weights");

	PlotControlWindow window{ vis };

	gui.frames(2, [&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, MultipleWindowsCanCoexist)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis1 = std::make_shared<Visualization>(sim);
	const auto vis2 = std::make_shared<Visualization>(sim);

	vis1->plot();
	vis2->plot(PlotType::HEATMAP);

	PlotControlWindow window1{ vis1 };
	PlotControlWindow window2{ vis2 };

	gui.frame([&] {
		window1.render();
		window2.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderAfterPlotAddition)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);

	PlotControlWindow window{ vis };

	gui.frame([&] {
		vis->plot();
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderAfterPlotRemoval)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);
	vis->plot();
	vis->plot();

	PlotControlWindow window{ vis };

	gui.frame([&] {
		window.render();
	});

	const int id = vis->getPlots().begin()->first->getUniqueIdentifier();
	vis->removePlot(id);

	gui.frame([&] {
		window.render();
	});
	SUCCEED();
}

TEST(PlotControlWindow, RenderWithMixedPlotTypes)
{
	test::HeadlessImGui gui;
	const auto sim = makeTestSimulation();
	const auto vis = std::make_shared<Visualization>(sim);

	for (int i = 0; i < 3; ++i)
	{
		if (i % 2 == 0)
			vis->plot();
		else
			vis->plot(PlotType::HEATMAP);
	}

	PlotControlWindow window{ vis };

	gui.frames(2, [&] {
		window.render();
	});
	SUCCEED();
}

// ---------------------------------------------------------------------------
// quick-populate decision logic (issue #57, from tests/user_interface/)
// ---------------------------------------------------------------------------

// quickPopulatePlotTypeFor() is the pure decision logic behind the Plot
// Control window's quick-populate button (see issue #57): it never touches
// ImGui, so it can be exercised headlessly, unlike PlotControlWindow::render()/
// renderContent(), which require a live ImGui/OpenGL context and are not
// unit-testable in this suite.

static std::shared_ptr<NeuralField> make1DField(const std::string& name = "nf-1d")
{
    const ElementCommonParameters cp{ name, 50 };
    const NeuralFieldParameters nfp{ 25.0, -5.0, SigmoidFunction(0.0, 10.0) };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<NeuralField2D> make2DField(const std::string& name = "nf-2d")
{
    const ElementCommonParameters cp{ name, ElementDimensions(10, 10, 1.0, 1.0) };
    const NeuralField2DParameters nfp{ 25.0, -5.0, SigmoidFunction(0.0, 10.0) };
    return std::make_shared<NeuralField2D>(cp, nfp);
}

TEST(QuickPopulatePlotTypeFor, OneDimensionalFieldGetsLinePlot)
{
    const auto field = make1DField();
    EXPECT_EQ(user_interface::quickPopulatePlotTypeFor(field), PlotType::LINE_PLOT);
}

TEST(QuickPopulatePlotTypeFor, TwoDimensionalFieldGetsHeatmap)
{
    const auto field = make2DField();
    EXPECT_EQ(user_interface::quickPopulatePlotTypeFor(field), PlotType::HEATMAP);
}

TEST(QuickPopulatePlotTypeFor, LabelMatchesElementDimensionality)
{
    const auto field1D = make1DField();
    const auto field2D = make2DField();
    EXPECT_EQ(field1D->getLabel(), ElementLabel::NEURAL_FIELD);
    EXPECT_EQ(field2D->getLabel(), ElementLabel::NEURAL_FIELD_2D);
    EXPECT_NE(user_interface::quickPopulatePlotTypeFor(field1D),
              user_interface::quickPopulatePlotTypeFor(field2D));
}
