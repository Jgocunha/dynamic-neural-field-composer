#pragma once

#include <type_traits>
//#if defined(_WIN32)
#include <imgui-platform-kit/user_interface.h>
//#elif defined(__linux__)
//#endif

#include "exceptions/exception.h"
#include "simulation/simulation.h"
#include "visualization/visualization.h"
#include "user_interface/main_window.h"

namespace dnf_composer
{
	// Base template: assumes T does not have a constructor that takes Simulation*
	template<typename T, typename = void>
	struct has_simulation_constructor : std::false_type {};

	// Specialization for types that take a Simulation*
	template<typename T>
	struct has_simulation_constructor<T, std::void_t<decltype(T(std::declval<std::shared_ptr<Simulation>>()))>> : std::true_type {};

	// Base template: assumes T does not have a constructor that takes Visualization*
	template<typename T, typename = void>
	struct has_visualization_constructor : std::false_type {};

	// Specialization for types that take a Visualization*
	template<typename T>
	struct has_visualization_constructor<T, std::void_t<decltype(T(std::declval<std::shared_ptr<Visualization>>()))>> : std::true_type {};

	class Application
	{
	private:
		std::shared_ptr<Simulation> simulation;
		std::shared_ptr<Visualization> visualization;
		std::shared_ptr<imgui_kit::UserInterface> gui;
		bool guiActive;
	public:
		explicit Application(const std::shared_ptr<Simulation>& simulation = nullptr,
			const std::shared_ptr<Visualization>& visualization = nullptr);

		void init() const;
		void step() const;
		void close() const;

		// For window types that do not require Simulation* or Visualization* arguments
		template<typename WindowType, typename... Args,
			std::enable_if_t<!has_simulation_constructor<WindowType>::value &&
			!has_visualization_constructor<WindowType>::value, int> = 0>
		void addWindow(Args&&... args) const {
			gui->addWindow<WindowType>(std::forward<Args>(args)...);
		}

		// For window types that require a Simulation* argument
		template<typename WindowType, typename... Args,
			std::enable_if_t<has_simulation_constructor<WindowType>::value, int> = 0>
		void addWindow(Args&&... args) const {
			gui->addWindow<WindowType>(simulation, std::forward<Args>(args)...);
		}

		// For window types that require a Visualization* argument
		template<typename WindowType, typename... Args,
			std::enable_if_t<!has_simulation_constructor<WindowType>::value&&
			has_visualization_constructor<WindowType>::value, int> = 0>
		void addWindow(Args&&... args) const {
			gui->addWindow<WindowType>(visualization, std::forward<Args>(args)...);
		}

		void toggleGUI();
		[[nodiscard]] bool hasGUIBeenClosed() const;
		[[nodiscard]] bool isGUIActive() const;

		~Application() = default;
	private:
		void setGUIParameters();
		void loadImGuiIniFile() const;
		static void enableKeyboardShortcuts();
	};
}

