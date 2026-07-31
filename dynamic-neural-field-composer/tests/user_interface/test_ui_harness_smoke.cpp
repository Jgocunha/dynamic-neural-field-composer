// Feasibility proof for the headless UI harness: if these pass, every
// user_interface/ render() path is reachable from the test suite.

#include <gtest/gtest.h>

#include <memory>

#include "ui_test_harness.h"

#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "user_interface/log_window.h"
#include "user_interface/field_metrics_window.h"

using namespace dnf_composer;

namespace
{
	std::shared_ptr<Simulation> makeSimulation()
	{
		auto simulation = std::make_shared<Simulation>("ui-smoke", 1.0, 0.0, 0.0);
		const element::AbsSigmoidFunction sigmoid{ 0.0, 100.0 };
		const element::NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
		const element::ElementCommonParameters common{ std::string("field"), 100 };
		simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));
		simulation->init();
		return simulation;
	}
}

// The context itself comes up without a window or GL loader.
TEST(UiHarnessSmoke, ContextInitialisesHeadless)
{
	const test::HeadlessImGui gui;
	ASSERT_NE(ImGui::GetCurrentContext(), nullptr);
	EXPECT_EQ(ImGui::GetIO().DisplaySize.x, 1280.0F);
}

// An empty frame completes — proves NewFrame/Render need no backend.
TEST(UiHarnessSmoke, EmptyFrameRenders)
{
	test::HeadlessImGui gui;
	gui.frame([] {});
	EXPECT_NE(ImGui::GetDrawData(), nullptr);
}

// A real window's render() executes end-to-end.
TEST(UiHarnessSmoke, LogWindowRenders)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// A window that walks live simulation data and pushes fonts/icons.
TEST(UiHarnessSmoke, FieldMetricsWindowRendersWithSimulation)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulation();
	user_interface::FieldMetricsWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Rendering after the simulation has advanced must stay stable.
TEST(UiHarnessSmoke, FieldMetricsWindowRendersAfterSteps)
{
	test::HeadlessImGui gui;
	const auto simulation = makeSimulation();
	for (int i = 0; i < 5; ++i)
		simulation->step();

	user_interface::FieldMetricsWindow window{ simulation };
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}
