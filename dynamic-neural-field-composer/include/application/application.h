#pragma once

#include <type_traits>
//#if defined(_WIN32)
#include <imgui-platform-kit/user_interface.h>
//#elif defined(__linux__)
//#endif

#include "exceptions/exception.h"
#include "simulation/simulation.h"
#include "visualization/visualization.h"
#include "user_interface/fonts/IconsFontAwesome6.h"
#include "user_interface/fonts/fa.h"

#define IMGUI_ENABLE_FREETYPE
#include "user_interface/fonts/imgui_freetype.h"

namespace dnf_composer
{
	enum class UIMode { Dynamic = 0, Static = 1 };

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
		static float uiScalePct; // user-controlled UI scale, 50–200%
		static UIMode uiMode;

	public:
		static float  getUiScalePct()          { return uiScalePct; }
		static void   setUiScalePct(float pct) { uiScalePct = pct; }
		static UIMode getUIMode()              { return uiMode; }
		static void   setUIMode(UIMode m)      { uiMode = m; }

	public:
		explicit Application(const std::shared_ptr<Simulation>& simulation = nullptr,
			const std::shared_ptr<Visualization>& visualization = nullptr);

		void init() const;
		void step() const;
		void close() const;
		static void registerSettingsHandler();

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
		static void enableKeyboardShortcuts();
		static void appendFonts();
	};

	// Text fonts — 3 sizes per weight
	inline ImFont* g_LightMediumFont;   // Cera Pro Light  @ 18 // main font
	inline ImFont* g_LightSmallFont;    // Cera Pro Light  @ 12
	inline ImFont* g_LightLargeFont;    // Cera Pro Light  @ 24

	inline ImFont* g_MediumSmallFont;   // Cera Pro Medium @ 12
	inline ImFont* g_MediumMediumFont;  // Cera Pro Medium @ 18
	inline ImFont* g_MediumLargeFont;   // Cera Pro Medium @ 24

	inline ImFont* g_BoldSmallFont;     // Cera Pro Bold   @ 12
	inline ImFont* g_BoldMediumFont;    // Cera Pro Bold   @ 18
	inline ImFont* g_BoldLargeFont;     // Cera Pro Bold   @ 24

	inline ImFont* g_BlackSmallFont;    // Cera Pro Black  @ 20
	inline ImFont* g_BlackMediumFont;   // Cera Pro Black  @ 24
	inline ImFont* g_BlackLargeFont;    // Cera Pro Black  @ 30

	inline ImFont* g_MonoSmallFont;     // JetBrainsMono   @ 16
	inline ImFont* g_MonoMediumFont;    // JetBrainsMono   @ 20
	inline ImFont* g_MonoLargeFont;     // JetBrainsMono   @ 26

	inline constexpr size_t g_FontCount = 15; // icon fonts not counted
	inline ImFont* g_SmallIconsFont;    // Font Awesome @ 12
	inline ImFont* g_MediumIconsFont;   // Font Awesome @ 20
	inline ImFont* g_LargeIconsFont;    // Font Awesome @ 48
}