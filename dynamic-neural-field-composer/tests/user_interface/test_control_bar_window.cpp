// Tests for ControlBarWindow UI component.

#include <gtest/gtest.h>

#include <memory>

#include "ui_test_harness.h"
#include "user_interface/control_bar_window.h"
#include "user_interface/log_window.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"

using namespace dnf_composer;

namespace
{
	std::shared_ptr<Simulation> makeEmptySimulation()
	{
		auto simulation = std::make_shared<Simulation>("control-bar-empty", 1.0, 0.0, 0.0);
		return simulation;
	}

	std::shared_ptr<Simulation> makeSimulationWithFields()
	{
		auto simulation = std::make_shared<Simulation>("control-bar-fields", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common1d{ std::string("field1d"), 100 };
		simulation->addElement(std::make_shared<element::NeuralField>(common1d, nfp));

		const element::NeuralField2DParameters nfp2d{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common2d{ std::string("field2d"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField2D>(common2d, nfp2d));

		return simulation;
	}
}

// Test that ControlBarWindow constructs and renders with empty simulation.
TEST(ControlBarWindow, RendersWithEmptySimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeEmptySimulation();
	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that ControlBarWindow renders before simulation init.
TEST(ControlBarWindow, RendersBeforeInit)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that ControlBarWindow renders after simulation init.
TEST(ControlBarWindow, RendersAfterInit)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that ControlBarWindow renders after a single step.
TEST(ControlBarWindow, RendersAfterSingleStep)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	simulation->step();
	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that ControlBarWindow renders after multiple steps.
TEST(ControlBarWindow, RendersAfterMultipleSteps)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	for (int i = 0; i < 10; ++i)
		simulation->step();

	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that ControlBarWindow handles paused simulation.
TEST(ControlBarWindow, RendersWithPausedSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	simulation->pause();
	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that ControlBarWindow handles resumed simulation.
TEST(ControlBarWindow, RendersWithResumedSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	simulation->pause();
	simulation->resume();
	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering across multiple frames with state transitions.
TEST(ControlBarWindow, RendersMultipleFramesWithTransitions)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	user_interface::ControlBarWindow window{ simulation };

	gui.frame([&] { window.render(); });

	simulation->init();
	gui.frame([&] { window.render(); });

	simulation->step();
	gui.frame([&] { window.render(); });

	simulation->pause();
	gui.frame([&] { window.render(); });

	simulation->resume();
	gui.frame([&] { window.render(); });

	SUCCEED();
}

// Test with different delta T values (time scaling).
TEST(ControlBarWindow, RendersWithDifferentTimescales)
{
	test::HeadlessImGui gui;
	auto simulation = std::make_shared<Simulation>("control-bar-timescale", 0.1, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 100 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));

	simulation->init();
	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with mixed log window state.
TEST(ControlBarWindow, RendersWithLogWindowInteraction)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	user_interface::ControlBarWindow window{ simulation };

	// Interact with log window while rendering control bar
	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Control bar test message");
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with many steps to exercise any timing/profiling display.
TEST(ControlBarWindow, RendersAfterManySteps)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	for (int i = 0; i < 50; ++i)
		simulation->step();

	user_interface::ControlBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}
