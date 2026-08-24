#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "user_interface/fonts/IconsFontAwesome6.h"

namespace dnf_composer::user_interface
{
	/// @brief Modal-style help window with an "About"/"How to use"/"Quick tips"/"Resources" nav.
	class HelpWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		inline static bool isWindowActive = false;
		inline static int  activePage     = 0;

	public:
		HelpWindow() = default;
		/// @brief Draw the help window for this frame.
		void render() override { draw(); }
		/// @brief Check whether the help window is currently open.
		/// @return True if the help window is active.
		static bool isActive()        { return isWindowActive; }
		/// @brief Open or close the help window.
		/// @param v True to open the window, false to close it.
		static void setActive(bool v) { isWindowActive = v; }
		~HelpWindow() override = default;

	private:
		static void draw();
		static void renderPageNav();
		static void renderPageAbout();
		static void renderPageHowToUse();
		static void renderPageQuickTips();
		static void renderPageResources();
	};
}
