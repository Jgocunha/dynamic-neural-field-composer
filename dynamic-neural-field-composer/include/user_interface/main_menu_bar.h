#pragma once

#include <imgui-platform-kit/user_interface_window.h>
#include <algorithm>
#include <imgui-platform-kit/themes.h>
#include <cmath>

#include "application/application.h"
#include "simulation/simulation.h"
#include "tools/file_dialog.h"

//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::user_interface
{
	struct FileFlags
	{
		bool showOpenSimulationDialog = false;
		bool showSaveSimulationDialog = false;
		bool showOpenLayoutDialog = false;
	};

	struct AdvancedSettingsFlags
	{
		bool showToolMetrics = false;
		bool showToolDebugLog = false;
		bool showToolIdStackTool = false;
		bool showToolStyleEditor = false;
		bool showToolAbout = false;
		bool showImGuiDemo = false;
		bool showImPlotDemo = false;
		bool showImGuiKitStyleEditor = false;
	};

	/// @brief What a quit trigger (Quit menu item or Ctrl+Q shortcut) should do to
	/// the simulation before the application is signalled to close.
	///
	/// Background (issue #122): the menu used to call `std::exit(0)` directly from
	/// inside the render callback. That terminates the process without unwinding
	/// the stack -- no destructors run, `Application::close()` never executes, and
	/// there is no way to unit-test it (you cannot assert on a dead process). The
	/// fix instead has the menu call `Application::requestQuit()`, which makes
	/// `Application::hasGUIBeenClosed()` return true, so the ordinary main loop shuts
	/// down through its normal, already-correct path (`Application::close()`, then
	/// every local's destructor as `main()` returns). Existing main loops keep
	/// working unchanged.
	///
	/// This enum captures the two triggers' pre-existing (and deliberately
	/// different) behavior: the Quit menu item saves before closing; the Ctrl+Q
	/// shortcut does not. That asymmetry predates this fix and is preserved as-is
	/// -- it is a pre-existing product decision, not part of issue #122's scope.
	enum class QuitAction
	{
		None,        ///< Neither trigger fired this frame -- no action.
		SaveAndQuit, ///< Quit menu item: save, close, and clean the simulation, then quit.
		QuitOnly     ///< Ctrl+Q shortcut: quit without touching the simulation.
	};

	/// @brief Pure decision function: given which quit trigger(s) fired this frame,
	/// decide what should happen. No ImGui, no Simulation, no OS calls -- so unlike
	/// `MainMenuBar::render()`/`handleShortcuts()`, it can be unit-tested headlessly
	/// (same pattern as `quickPopulatePlotTypeFor()` in plot_control_window.h).
	/// If both triggers somehow fire in the same frame, the menu item's
	/// save-before-closing behavior takes precedence (the safer of the two).
	[[nodiscard]] QuitAction decideQuitAction(bool quitMenuItemClicked, bool ctrlQPressed) noexcept;

	class MainMenuBar final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		AdvancedSettingsFlags advancedSettingsFlags;
		FileFlags fileFlags;
	public:
		explicit MainMenuBar(const std::shared_ptr<Simulation>& simulation);
		MainMenuBar(const MainMenuBar&) = delete;
		MainMenuBar& operator=(const MainMenuBar&) = delete;
		MainMenuBar(MainMenuBar&&) = delete;
		MainMenuBar& operator=(MainMenuBar&&) = delete;

		void render() override;
		~MainMenuBar() override = default;

		/// @brief Carry out the simulation-side effects of @p action (if any) and,
		/// unless @p action is QuitAction::None, call `Application::requestQuit()`.
		/// Public (rather than an implementation detail of render()/handleShortcuts())
		/// specifically so it can be driven directly from tests without a live ImGui
		/// context -- it touches only Simulation and the quit request, never ImGui.
		void executeQuitAction(QuitAction action);
	private:
		void renderMainMenuBar();
		void renderFileWindows();
		void renderAdvancedSettingsWindows();
		void handleShortcuts();
	};
}
