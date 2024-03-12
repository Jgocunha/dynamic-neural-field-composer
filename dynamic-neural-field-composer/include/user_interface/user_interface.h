#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "implot.h"
#include "implot_internal.h"

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif


#include <vector>
#include <string>
#include <iostream>

#include "user_interface_window.h"
#include "resources/resources.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif


static void glfw_error_callback(int error, const char* description);

namespace dnf_composer
{
	namespace user_interface
	{
		class UserInterface
		{
		private:
			std::vector<std::shared_ptr<UserInterfaceWindow>> windows;

			GLFWwindow* window;
			bool closeUI;
			int windowWidth = 1280;
            int windowHeight = 720;

			//ImVec4 clear_color = ImVec4(0.2f, 0.2f, 0.2f, 1.00f); // Darkish gray
		public:
			UserInterface();
			UserInterface(const UserInterface&) = delete;
			UserInterface& operator=(const UserInterface&) = delete;
			UserInterface(UserInterface&&) = delete;
			UserInterface& operator=(UserInterface&&) = delete;

			void init();
			void step();
			void close() const;

			void activateWindow(const std::shared_ptr<UserInterfaceWindow>& window);

			bool getCloseUI() const;
			~UserInterface() = default;

		private:
			void render() const;
		};
	}
}