// Tests for PlotsWindow UI component.

#include <gtest/gtest.h>

#include <memory>

#include "ui_test_harness.h"
#include "user_interface/plots_window.h"
#include "visualization/visualization.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"

using namespace dnf_composer;

namespace
{
	std::shared_ptr<Visualization> makeVisualizationWithEmptySimulation()
	{
		auto simulation = std::make_shared<Simulation>("plots-empty", 1.0, 0.0, 0.0);
		auto visualization = std::make_shared<Visualization>(simulation);
		return visualization;
	}

	std::shared_ptr<Visualization> makeVisualizationWithFields()
	{
		auto simulation = std::make_shared<Simulation>("plots-fields", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common1d{ std::string("field1d"), 100 };
		simulation->addElement(std::make_shared<element::NeuralField>(common1d, nfp));

		const element::NeuralField2DParameters nfp2d{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common2d{ std::string("field2d"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField2D>(common2d, nfp2d));

		auto visualization = std::make_shared<Visualization>(simulation);
		return visualization;
	}

	std::shared_ptr<Visualization> makeVisualizationWithPlots()
	{
		auto simulation = std::make_shared<Simulation>("plots-with-data", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common1d{ std::string("field1d"), 100 };
		simulation->addElement(std::make_shared<element::NeuralField>(common1d, nfp));

		const element::NeuralField2DParameters nfp2d{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common2d{ std::string("field2d"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField2D>(common2d, nfp2d));

		auto visualization = std::make_shared<Visualization>(simulation);

		// Add plots with data sources
		visualization->plot("field1d", "activation");
		visualization->plot("field2d", "activation");

		return visualization;
	}
}

// Test that PlotsWindow constructs and renders with empty simulation.
TEST(PlotsWindow, RendersWithEmptySimulation)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithEmptySimulation();
	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that PlotsWindow renders with a visualization containing fields.
TEST(PlotsWindow, RendersWithFieldsBeforeInit)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithFields();
	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that PlotsWindow renders with plots added.
TEST(PlotsWindow, RendersWithPlots)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that PlotsWindow renders after simulation initialization.
TEST(PlotsWindow, RendersAfterInit)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	visualization->getSimulation()->init();
	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that PlotsWindow renders after simulation steps.
TEST(PlotsWindow, RendersAfterSteps)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();
	simulation->init();
	for (int i = 0; i < 5; ++i)
		simulation->step();

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with 1D and 2D plots.
TEST(PlotsWindow, RendersMixedDimensionalityPlots)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();
	simulation->init();
	simulation->step();

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering multiple frames with changing simulation state.
TEST(PlotsWindow, RendersMultipleFramesWithStateChanges)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();
	user_interface::PlotsWindow window{ visualization };

	gui.frame([&] { window.render(); });

	simulation->init();
	gui.frame([&] { window.render(); });

	simulation->step();
	gui.frame([&] { window.render(); });

	for (int i = 0; i < 4; ++i)
		simulation->step();
	gui.frame([&] { window.render(); });

	SUCCEED();
}

// Test rendering with paused simulation.
TEST(PlotsWindow, RendersWithPausedSimulation)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();
	simulation->init();
	simulation->pause();

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering after resuming paused simulation.
TEST(PlotsWindow, RendersAfterResume)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();
	simulation->init();
	simulation->pause();
	simulation->resume();

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with many steps to exercise plot rendering with data.
TEST(PlotsWindow, RendersAfterManySteps)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();
	simulation->init();
	for (int i = 0; i < 50; ++i)
		simulation->step();

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with multiple plots added.
TEST(PlotsWindow, RendersWithMultiplePlots)
{
	test::HeadlessImGui gui;
	auto simulation = std::make_shared<Simulation>("plots-multiple", 1.0, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common1d{ std::string("field1d"), 100 };
	simulation->addElement(std::make_shared<element::NeuralField>(common1d, nfp));

	auto visualization = std::make_shared<Visualization>(simulation);

	// Add multiple plots
	visualization->plot("field1d", "activation");
	visualization->plot("field1d", "input");
	visualization->plot("field1d", "output");

	simulation->init();
	for (int i = 0; i < 10; ++i)
		simulation->step();

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with window ID suffix (to avoid collisions).
TEST(PlotsWindow, RendersWithWindowIdSuffix)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();

	visualization->setWindowIdSuffix("_test");
	simulation->init();
	simulation->step();

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });

	visualization->clearWindowIdSuffix();
	gui.frame([&] { window.render(); });

	SUCCEED();
}

// Test that rendering handles empty plots gracefully.
TEST(PlotsWindow, RendersEmptyVisualization)
{
	test::HeadlessImGui gui;
	auto simulation = std::make_shared<Simulation>("plots-empty-viz", 1.0, 0.0, 0.0);
	auto visualization = std::make_shared<Visualization>(simulation);

	user_interface::PlotsWindow window{ visualization };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test continuous rendering with plot data accumulation.
TEST(PlotsWindow, RendersWithContinuousDataAccumulation)
{
	test::HeadlessImGui gui;
	const auto visualization = makeVisualizationWithPlots();
	const auto simulation = visualization->getSimulation();
	simulation->init();

	user_interface::PlotsWindow window{ visualization };

	for (int frame = 0; frame < 5; ++frame)
	{
		gui.frame([&] { window.render(); });
		simulation->step();
	}

	SUCCEED();
}
