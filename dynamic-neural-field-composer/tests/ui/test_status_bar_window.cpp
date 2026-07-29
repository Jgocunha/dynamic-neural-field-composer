// Tests for StatusBarWindow UI component.

#include <gtest/gtest.h>

#include <memory>

#include "ui_test_harness.h"
#include "user_interface/status_bar_window.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"

using namespace dnf_composer;

namespace
{
	std::shared_ptr<Simulation> makeEmptySimulation()
	{
		auto simulation = std::make_shared<Simulation>("status-bar-empty", 1.0, 0.0, 0.0);
		return simulation;
	}

	std::shared_ptr<Simulation> makeSimulationWithFields()
	{
		auto simulation = std::make_shared<Simulation>("status-bar-fields", 1.0, 0.0, 0.0);

		// Add a 1D neural field
		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common1d{ std::string("field1d"), 100 };
		simulation->addElement(std::make_shared<element::NeuralField>(common1d, nfp));

		// Add a 2D neural field
		const element::NeuralField2DParameters nfp2d{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common2d{ std::string("field2d"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField2D>(common2d, nfp2d));

		// Add a 1D stimulus
		const element::GaussStimulusParameters stimParams{ 10.0, 0.5, 0.5 };
		const element::ElementCommonParameters commonStim1d{ std::string("stim1d"), 100 };
		simulation->addElement(std::make_shared<element::GaussStimulus>(commonStim1d, stimParams));

		// Add a 2D stimulus
		const element::GaussStimulus2DParameters stimParams2d{ 10.0, 0.5, 0.5, 0.5, false, false };
		const element::ElementCommonParameters commonStim2d{ std::string("stim2d"), 50 };
		simulation->addElement(std::make_shared<element::GaussStimulus2D>(commonStim2d, stimParams2d));

		return simulation;
	}
}

// Test that StatusBarWindow constructs and renders with empty simulation.
TEST(StatusBarWindow, RendersWithEmptySimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeEmptySimulation();
	user_interface::StatusBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that StatusBarWindow renders with a simulation containing fields.
TEST(StatusBarWindow, RendersWithFieldsBeforeInit)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	user_interface::StatusBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that StatusBarWindow renders after simulation initialization.
TEST(StatusBarWindow, RendersAfterInit)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	user_interface::StatusBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that StatusBarWindow renders after simulation steps.
TEST(StatusBarWindow, RendersAfterSteps)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	for (int i = 0; i < 5; ++i)
		simulation->step();

	user_interface::StatusBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that StatusBarWindow renders across multiple frames with changing state.
TEST(StatusBarWindow, RendersMultipleFramesWithStateChanges)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	user_interface::StatusBarWindow window{ simulation };

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

// Test that StatusBarWindow handles paused simulation.
TEST(StatusBarWindow, RendersWithPausedSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	simulation->pause();

	user_interface::StatusBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that StatusBarWindow renders with resumed simulation.
TEST(StatusBarWindow, RendersWithResumedSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	simulation->pause();
	simulation->resume();

	user_interface::StatusBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with large number of steps to exercise timing display.
TEST(StatusBarWindow, RendersAfterManySteps)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithFields();
	simulation->init();
	for (int i = 0; i < 100; ++i)
		simulation->step();

	user_interface::StatusBarWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}
