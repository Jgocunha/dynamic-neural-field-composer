// Tests for MainMenuBar UI component.

#include <gtest/gtest.h>

#include <memory>

#include "ui_test_harness.h"
#include "user_interface/main_menu_bar.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus.h"

using namespace dnf_composer;

namespace
{
	std::shared_ptr<Simulation> makeEmptySimulation()
	{
		auto simulation = std::make_shared<Simulation>("menu-bar-empty", 1.0, 0.0, 0.0);
		return simulation;
	}

	std::shared_ptr<Simulation> makeSimulationWithElements()
	{
		auto simulation = std::make_shared<Simulation>("menu-bar-elements", 1.0, 0.0, 0.0);

		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common1d{ std::string("field1d"), 100 };
		simulation->addElement(std::make_shared<element::NeuralField>(common1d, nfp));

		const element::NeuralField2DParameters nfp2d{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common2d{ std::string("field2d"), 50 };
		simulation->addElement(std::make_shared<element::NeuralField2D>(common2d, nfp2d));

		const element::GaussStimulusParameters stimParams{ 10.0, 0.5, 0.5 };
		const element::ElementCommonParameters commonStim{ std::string("stimulus"), 100 };
		simulation->addElement(std::make_shared<element::GaussStimulus>(commonStim, stimParams));

		return simulation;
	}
}

// Test that MainMenuBar constructs and renders with empty simulation.
TEST(MainMenuBar, RendersWithEmptySimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeEmptySimulation();
	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that MainMenuBar renders with a populated simulation.
TEST(MainMenuBar, RendersWithPopulatedSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that MainMenuBar renders before simulation initialization.
TEST(MainMenuBar, RendersBeforeInit)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that MainMenuBar renders after simulation initialization.
TEST(MainMenuBar, RendersAfterInit)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	simulation->init();
	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that MainMenuBar renders after simulation steps.
TEST(MainMenuBar, RendersAfterSteps)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	simulation->init();
	for (int i = 0; i < 5; ++i)
		simulation->step();

	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that MainMenuBar renders with paused simulation.
TEST(MainMenuBar, RendersWithPausedSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	simulation->init();
	simulation->pause();
	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that MainMenuBar renders with resumed simulation.
TEST(MainMenuBar, RendersWithResumedSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	simulation->init();
	simulation->pause();
	simulation->resume();
	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering across multiple frames with state changes.
TEST(MainMenuBar, RendersMultipleFramesWithStateChanges)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	user_interface::MainMenuBar window{ simulation };

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

// Test rendering with many simulation steps.
TEST(MainMenuBar, RendersAfterManySteps)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	simulation->init();
	for (int i = 0; i < 100; ++i)
		simulation->step();

	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering multiple consecutive frames maintains state.
TEST(MainMenuBar, RendersMaintainsStateAcrossFrames)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	user_interface::MainMenuBar window{ simulation };

	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with a simulation using different timescale.
TEST(MainMenuBar, RendersWithDifferentTimescale)
{
	test::HeadlessImGui gui;
	auto simulation = std::make_shared<Simulation>("menu-bar-timescale", 0.01, 0.0, 0.0);

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 100 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));

	user_interface::MainMenuBar window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering after cleaning and re-adding elements.
TEST(MainMenuBar, RendersAfterCleanAndRebuild)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulationWithElements();
	simulation->init();
	simulation->step();

	user_interface::MainMenuBar window{ simulation };
	gui.frame([&] { window.render(); });

	// Simulate state changes
	simulation->clean();
	gui.frame([&] { window.render(); });

	const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
	const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
	const element::ElementCommonParameters common{ std::string("field"), 100 };
	simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));
	simulation->init();

	gui.frame([&] { window.render(); });
	SUCCEED();
}
