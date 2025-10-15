#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "tools/file_dialog.h"

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

	struct InterfaceFlags
	{
		bool dockingEnabled = true;
		bool fixedLayout = false;
	};

	class MainWindow : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		AdvancedSettingsFlags advancedSettingsFlags;
		FileFlags fileFlags;
		InterfaceFlags interfaceFlags;
	public:
		explicit MainWindow(const std::shared_ptr<Simulation>& simulation);
		MainWindow(const MainWindow&) = delete;
		MainWindow& operator=(const MainWindow&) = delete;
		MainWindow(MainWindow&&) = delete;
		MainWindow& operator=(MainWindow&&) = delete;

		void render() override;
		~MainWindow() override = default;
	private:
		static void renderFullscreenWindow();
		void renderMainMenuBar();
		void renderFileWindows();
		void renderAdvancedSettingsWindows();
		void handleShortcuts();
		static void handleOpenLayoutDialog(const char* path);
		void toggleFixedLayout() const;
	public:
		[[nodiscard]] bool isFixedLayout() const { return interfaceFlags.fixedLayout; }
	};
}
