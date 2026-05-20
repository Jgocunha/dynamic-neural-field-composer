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

/// @defgroup application Application
/// @brief Top-level application loop, window management, and font registry.

namespace dnf_composer
{
	/// @brief Selects whether UI windows are managed dynamically or in a fixed static layout.
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

	/// @brief Top-level application that owns the GUI, simulation, and visualization.
	///
	/// Application drives the main loop:
	/// @code
	///   app.init();
	///   while (!app.hasGUIBeenClosed())
	///       app.step();
	///   app.close();
	/// @endcode
	///
	/// Windows are registered via @c addWindow<T>() before @c init(). The template
	/// specializations automatically forward a Simulation or Visualization pointer
	/// to constructors that require one.
	///
	/// @ingroup application
	class Application
	{
	private:
		std::shared_ptr<Simulation> simulation;
		std::shared_ptr<Visualization> visualization;
		std::shared_ptr<imgui_kit::UserInterface> gui;
		bool guiActive;
		static float uiScalePct; ///< User-controlled UI scale percentage (50–200%).
		static UIMode uiMode;    ///< Current UI layout mode.

	public:
		static float  getUiScalePct()          { return uiScalePct; }
		static void   setUiScalePct(float pct) { uiScalePct = pct; }
		static UIMode getUIMode()              { return uiMode; }
		static void   setUIMode(UIMode m)      { uiMode = m; }

	public:
		/// @brief Construct an Application.
		/// @param simulation    Shared simulation to drive (may be nullptr).
		/// @param visualization Shared visualization to render (may be nullptr).
		explicit Application(const std::shared_ptr<Simulation>& simulation = nullptr,
			const std::shared_ptr<Visualization>& visualization = nullptr);

		/// @brief Initialize the GUI and all registered windows. Call once before @c step().
		void init() const;

		/// @brief Advance the simulation by one step and render all windows.
		void step() const;

		void close() const;

		/// @brief Register the application's ImGui settings handler (for persistence).
		static void registerSettingsHandler();

		/// @brief Register a window that needs no Simulation or Visualization pointer.
		template<typename WindowType, typename... Args,
			std::enable_if_t<!has_simulation_constructor<WindowType>::value &&
			!has_visualization_constructor<WindowType>::value, int> = 0>
		void addWindow(Args&&... args) const {
			gui->addWindow<WindowType>(std::forward<Args>(args)...);
		}

		/// @brief Register a window that takes a Simulation shared_ptr as first argument.
		template<typename WindowType, typename... Args,
			std::enable_if_t<has_simulation_constructor<WindowType>::value, int> = 0>
		void addWindow(Args&&... args) const {
			gui->addWindow<WindowType>(simulation, std::forward<Args>(args)...);
		}

		/// @brief Register a window that takes a Visualization shared_ptr as first argument.
		template<typename WindowType, typename... Args,
			std::enable_if_t<!has_simulation_constructor<WindowType>::value&&
			has_visualization_constructor<WindowType>::value, int> = 0>
		void addWindow(Args&&... args) const {
			gui->addWindow<WindowType>(visualization, std::forward<Args>(args)...);
		}

		/// @brief Toggle the GUI on or off at runtime.
		void toggleGUI();

		/// @brief Return true if the user has closed the main window.
		[[nodiscard]] bool hasGUIBeenClosed() const;

		[[nodiscard]] bool isGUIActive() const;

		~Application() = default;
	private:
		void setGUIParameters();
		static void enableKeyboardShortcuts();
		static void appendFonts();
	};

	// Text fonts — 3 sizes per weight
	inline ImFont* g_LightMediumFont;   ///< Cera Pro Light  @ 18 (main font)
	inline ImFont* g_LightSmallFont;    ///< Cera Pro Light  @ 12
	inline ImFont* g_LightLargeFont;    ///< Cera Pro Light  @ 24

	inline ImFont* g_MediumSmallFont;   ///< Cera Pro Medium @ 12
	inline ImFont* g_MediumMediumFont;  ///< Cera Pro Medium @ 18
	inline ImFont* g_MediumLargeFont;   ///< Cera Pro Medium @ 24

	inline ImFont* g_BoldSmallFont;     ///< Cera Pro Bold   @ 12
	inline ImFont* g_BoldMediumFont;    ///< Cera Pro Bold   @ 18
	inline ImFont* g_BoldLargeFont;     ///< Cera Pro Bold   @ 24

	inline ImFont* g_BlackSmallFont;    ///< Cera Pro Black  @ 20
	inline ImFont* g_BlackMediumFont;   ///< Cera Pro Black  @ 24
	inline ImFont* g_BlackLargeFont;    ///< Cera Pro Black  @ 30

	inline ImFont* g_MonoSmallFont;     ///< JetBrainsMono   @ 16
	inline ImFont* g_MonoMediumFont;    ///< JetBrainsMono   @ 20
	inline ImFont* g_MonoLargeFont;     ///< JetBrainsMono   @ 26

	inline constexpr size_t g_FontCount = 15; ///< Number of text font variants (icon fonts not counted).
	inline ImFont* g_SmallIconsFont;    ///< Font Awesome @ 12
	inline ImFont* g_MediumIconsFont;   ///< Font Awesome @ 20
	inline ImFont* g_LargeIconsFont;    ///< Font Awesome @ 26
}
