// Tests for LogWindow UI component.

#include <gtest/gtest.h>

#include "ui_test_harness.h"
#include "user_interface/log_window.h"

using namespace dnf_composer;

namespace
{
	// Clear the log before each test
	class LogWindowTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			// Reset static state
			user_interface::LogWindow::setActive(false);
			user_interface::LogWindow::setExpanded(false);
		}
	};
}

// Test that LogWindow constructs and renders with default state.
TEST_F(LogWindowTest, ConstructsAndRendersDefault)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that addLog can be called with messages and then rendered.
TEST_F(LogWindowTest, RendersWithLogMessages)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	// Add some log messages
	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Info message");
	user_interface::LogWindow::addLog(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error message");
	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Warning message");

	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that setActive(true) affects rendering.
TEST_F(LogWindowTest, RendersWhenActive)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::setActive(true);
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that setActive(false) affects rendering.
TEST_F(LogWindowTest, RendersWhenInactive)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::setActive(false);
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that setExpanded(true) affects rendering.
TEST_F(LogWindowTest, RendersWhenExpanded)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::setExpanded(true);
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that setExpanded(false) affects rendering.
TEST_F(LogWindowTest, RendersWhenNotExpanded)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::setExpanded(false);
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test rendering with multiple state combinations.
TEST_F(LogWindowTest, RendersWithActiveAndExpanded)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::setActive(true);
	user_interface::LogWindow::setExpanded(true);
	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Test message");

	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that rendering multiple frames with log content works correctly.
TEST_F(LogWindowTest, RendersMultipleFramesWithContent)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Frame 1");
	gui.frame([&] { window.render(); });

	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Frame 2");
	gui.frame([&] { window.render(); });

	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Frame 3");
	gui.frame([&] { window.render(); });

	SUCCEED();
}

// Test that addLog with formatted strings works.
TEST_F(LogWindowTest, AddLogWithFormatting)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Message with value: %d", 42);
	user_interface::LogWindow::addLog(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Float value: %.2f", 3.14159f);

	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test active state toggling during rendering.
TEST_F(LogWindowTest, TogglesActiveStateDuringRendering)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::setActive(true);
	gui.frame([&] { window.render(); });

	user_interface::LogWindow::setActive(false);
	gui.frame([&] { window.render(); });

	user_interface::LogWindow::setActive(true);
	gui.frame([&] { window.render(); });

	SUCCEED();
}

// Test expanded state toggling during rendering.
TEST_F(LogWindowTest, TogglesExpandedStateDuringRendering)
{
	test::HeadlessImGui gui;
	user_interface::LogWindow window;

	user_interface::LogWindow::setExpanded(false);
	gui.frame([&] { window.render(); });

	user_interface::LogWindow::setExpanded(true);
	gui.frame([&] { window.render(); });

	user_interface::LogWindow::setExpanded(false);
	gui.frame([&] { window.render(); });

	SUCCEED();
}
