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
	private:
		void renderMainMenuBar();
		void renderFileWindows();
		void renderAdvancedSettingsWindows();
		void handleShortcuts();
	};
}
