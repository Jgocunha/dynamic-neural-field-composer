// Tests for HelpWindow UI component.

#include <gtest/gtest.h>

#include "ui_test_harness.h"
#include "user_interface/help_window.h"

using namespace dnf_composer;

namespace
{
	// Clear the help window state before each test
	class HelpWindowTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			user_interface::HelpWindow::setActive(false);
		}
	};
}

// Test that HelpWindow constructs with default state.
TEST_F(HelpWindowTest, ConstructsDefault)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;
	EXPECT_FALSE(user_interface::HelpWindow::isActive());
	SUCCEED();
}

// Test that HelpWindow renders with default inactive state.
TEST_F(HelpWindowTest, RendersInactiveDefault)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that HelpWindow renders when active.
TEST_F(HelpWindowTest, RendersWhenActive)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	user_interface::HelpWindow::setActive(true);
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test that HelpWindow renders when inactive.
TEST_F(HelpWindowTest, RendersWhenInactive)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	user_interface::HelpWindow::setActive(false);
	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}

// Test toggling active state during rendering.
TEST_F(HelpWindowTest, TogglesActiveDuringRendering)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	user_interface::HelpWindow::setActive(false);
	gui.frame([&] { window.render(); });

	user_interface::HelpWindow::setActive(true);
	gui.frame([&] { window.render(); });

	user_interface::HelpWindow::setActive(false);
	gui.frame([&] { window.render(); });

	SUCCEED();
}

// Test rendering multiple frames with active state.
TEST_F(HelpWindowTest, RendersMultipleFramesActive)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	user_interface::HelpWindow::setActive(true);
	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

// Test rendering multiple frames with inactive state.
TEST_F(HelpWindowTest, RendersMultipleFramesInactive)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	user_interface::HelpWindow::setActive(false);
	gui.frames(5, [&] { window.render(); });
	SUCCEED();
}

// Test that isActive() correctly reflects the active state.
TEST_F(HelpWindowTest, IsActiveReflectsState)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	user_interface::HelpWindow::setActive(true);
	EXPECT_TRUE(user_interface::HelpWindow::isActive());

	user_interface::HelpWindow::setActive(false);
	EXPECT_FALSE(user_interface::HelpWindow::isActive());

	user_interface::HelpWindow::setActive(true);
	EXPECT_TRUE(user_interface::HelpWindow::isActive());
}

// Test rendering with rapid state changes.
TEST_F(HelpWindowTest, RendersWithRapidStateChanges)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	for (int i = 0; i < 5; ++i)
	{
		user_interface::HelpWindow::setActive(true);
		gui.frame([&] { window.render(); });
		user_interface::HelpWindow::setActive(false);
		gui.frame([&] { window.render(); });
	}

	SUCCEED();
}

// Test multiple windows with different states.
TEST_F(HelpWindowTest, MultipleWindowInstancesRender)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window1;
	user_interface::HelpWindow window2;

	user_interface::HelpWindow::setActive(true);
	gui.frame([&] {
		window1.render();
		window2.render();
	});

	user_interface::HelpWindow::setActive(false);
	gui.frame([&] {
		window1.render();
		window2.render();
	});

	SUCCEED();
}

// Test rendering after state has been set multiple times.
TEST_F(HelpWindowTest, RendersAfterMultipleStateChanges)
{
	test::HeadlessImGui gui;
	user_interface::HelpWindow window;

	user_interface::HelpWindow::setActive(true);
	user_interface::HelpWindow::setActive(false);
	user_interface::HelpWindow::setActive(true);
	user_interface::HelpWindow::setActive(false);

	gui.frames(2, [&] { window.render(); });
	SUCCEED();
}
