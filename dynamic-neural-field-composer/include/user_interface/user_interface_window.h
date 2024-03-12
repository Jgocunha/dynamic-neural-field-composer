#pragma once


#include <memory>

#include "imgui.h"
//#include "imgui_impl_win32.h"
#include "imgui_impl_glfw.h"
//#include "imgui_impl_dx12.h"
#include "imgui_impl_opengl3.h"

#include "implot.h"
#include "implot_internal.h"

enum CharSize : size_t
{
	CHAR_SIZE = 50
};

namespace dnf_composer
{
	namespace user_interface
	{

		enum UserInterfaceWindowType : int
		{
			MONITORING_WINDOW,
			ELEMENT_WINDOW,
			LOG_WINDOW,
			PLOT_WINDOW,
			SIMULATION_WINDOW,
		};

		struct UserInterfaceWindowParameters
		{
			// This struct is intentionally left empty.
			// Derived structs are expected to define specific member variables
			// and functions relevant to their respective elements.
			UserInterfaceWindowParameters() = default;
			// Declare a virtual destructor
			virtual ~UserInterfaceWindowParameters() = default;
		};

		class UserInterfaceWindow
		{
		public:
			UserInterfaceWindow() = default;
			virtual void render() = 0;
			virtual ~UserInterfaceWindow() = default;

			UserInterfaceWindow(const UserInterfaceWindow&) = delete;
			UserInterfaceWindow& operator=(const UserInterfaceWindow&) = delete;
			UserInterfaceWindow(UserInterfaceWindow&&) = delete;
			UserInterfaceWindow& operator=(UserInterfaceWindow&&) = delete;
		};
	}
}

