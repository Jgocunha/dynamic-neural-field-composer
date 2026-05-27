#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "user_interface/fonts/IconsFontAwesome6.h"

namespace dnf_composer
{
	extern ImFont* g_MediumLargeFont;
	extern ImFont* g_BoldLargeFont;
	extern ImFont* g_BoldMediumFont;
	extern ImFont* g_MediumIconsFont;

	namespace user_interface
	{
		class HelpWindow final : public imgui_kit::UserInterfaceWindow
		{
		private:
			inline static bool isWindowActive = false;
			inline static int  activePage     = 0;

		public:
			HelpWindow() = default;
			void render() override { draw(); }
			static bool isActive()        { return isWindowActive; }
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
}
