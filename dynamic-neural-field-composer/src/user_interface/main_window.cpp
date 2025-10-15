#include "user_interface/main_window.h"

#include <imgui-platform-kit/themes.h>

namespace dnf_composer::user_interface
{
	MainWindow::MainWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{}


	void MainWindow::render()
	{
		renderFullscreenWindow();
		renderMainMenuBar();
		renderFileWindows();
		renderAdvancedSettingsWindows();
		handleShortcuts();
	}

    void MainWindow::renderFullscreenWindow()
    {
        static bool use_work_area = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus
		//| ImGuiWindowFlags_NoInputs           // Prevents all mouse/keyboard input
		//| ImGuiWindowFlags_NoBackground       // Makes window background transparent
		| ImGuiWindowFlags_NoScrollbar        // Removes scrollbars
		| ImGuiWindowFlags_NoScrollWithMouse  // Prevents mouse wheel scrolling
		| ImGuiWindowFlags_NoCollapse         // Prevents collapsing (redundant with NoDecoration but explicit)
		| ImGuiWindowFlags_NoTitleBar;        // Removes title bar (redundant with NoDecoration but explicit)

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
		ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

		// Set window to be behind all other windows
		//ImGui::SetNextWindowBgAlpha(0.0f); // Make background completely transparent

        if (ImGui::Begin("Fullscreen window", nullptr, flags))
        {}
        ImGui::End();
    }

    void MainWindow::renderMainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                    simulation->close();
                    simulation->clean();
                }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    FileDialog::file_dialog_open = true;
                    fileFlags.showOpenSimulationDialog = true;
                    FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    simulation->save();
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                {
                    FileDialog::file_dialog_open = true;
                    fileFlags.showSaveSimulationDialog = true;
                    FileDialog::file_dialog_open_type = FileDialog::FileDialogType::SelectFolder;
                }
                if (ImGui::MenuItem("Quit", "Ctrl+q"))
                {
                    simulation->save();
	                simulation->close();
					simulation->clean();
                    std::exit(0);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings"))
            {
                static char newIdentifier[128] = "";   // Buffer for editing the identifier
                static bool initialized = false;      // Flag to track initialization

                if (!initialized)
                {
					snprintf(newIdentifier, sizeof(newIdentifier), "%s", simulation->getIdentifier().c_str());
                	initialized = true;
                }

                ImGui::Text("Simulation Identifier");
                ImGui::InputText("##inline_identifier", newIdentifier, sizeof(newIdentifier));

                ImGui::SameLine();
                if (ImGui::Button("Save##menu_identifier"))
                {
                    simulation->setUniqueIdentifier(newIdentifier);
                }

                ImGui::Separator();

                static auto deltaT = static_cast<float>(simulation->getDeltaT());
                ImGui::Text("Time Step (deltaT) ");
                ImGui::SliderFloat("##menu_deltaT_slider", &deltaT, 0.001f, 25.0, "%.3f");
                if (ImGui::IsItemDeactivatedAfterEdit())
                    simulation->setDeltaT(deltaT);

                ImGui::Separator();

                ImGui::Text("Current Time (t) ");
                ImGui::SameLine();
                ImGui::Text("%.3f", simulation->getT());

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Simulation Control"))
            {
                if (ImGui::MenuItem("Start", "Ctrl+Space"))
                    simulation->init();
                if (ImGui::MenuItem("Stop", "Ctrl+C"))
                    simulation->close();
                if (ImGui::MenuItem("Pause", "Ctrl+P"))
                    simulation->pause();
                ImGui::EndMenu();
            }

        	if (ImGui::BeginMenu("Interface Settings"))
        	{
        		if (ImGui::MenuItem("Load Layout", "Ctrl+L"))
        		{
        			FileDialog::file_dialog_open = true;
        			fileFlags.showOpenLayoutDialog = true;
        			FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
        		}
        		if (ImGui::MenuItem("Save Layout"))
        		{
        			std::string savePath = std::string(PROJECT_DIR) + "/resources/layouts/"
						+ simulation->getIdentifier() + "_layout.ini";
        			ImGui::SaveIniSettingsToDisk(savePath.c_str());
        			log(tools::logger::LogLevel::INFO, "Saved layout to " + savePath + ".");
        		}
        		if (ImGui::MenuItem("Fixed Layout", nullptr, &interfaceFlags.fixedLayout))
        		{
        			toggleFixedLayout();
        		}

        		ImGui::Separator();

        		ImGui::MenuItem("Dear ImGuiStyle Editor", nullptr,
					&advancedSettingsFlags.showToolStyleEditor);
        		ImGui::MenuItem("Dear ImGui Kit Style Editor", nullptr,
					&advancedSettingsFlags.showImGuiKitStyleEditor);
        		ImGui::EndMenu();
        	}

            if (ImGui::BeginMenu("Advanced Settings"))
            {
                ImGui::MenuItem("Dear ImGui Demo", nullptr,
					&advancedSettingsFlags.showImGuiDemo);
                ImGui::MenuItem("ImPlot Demo", nullptr,
					&advancedSettingsFlags.showImPlotDemo);
                ImGui::MenuItem("Dear ImGui Metrics/Debugger", nullptr,
                    &advancedSettingsFlags.showToolMetrics);
                ImGui::MenuItem("Dear ImGui Debug Log", nullptr,
                    &advancedSettingsFlags.showToolDebugLog);
                ImGui::MenuItem("Dear ImGui ID Stack Tool", nullptr,
                    &advancedSettingsFlags.showToolIdStackTool);
                ImGui::MenuItem("About Dear ImGui", nullptr,
                    &advancedSettingsFlags.showToolAbout);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void MainWindow::renderFileWindows()
    {
        static char path[500] = "";
        static char* file_dialog_buffer = path;
        
        if (FileDialog::file_dialog_open) 
        {
            FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, file_dialog_buffer,
                sizeof(file_dialog_buffer), FileDialog::file_dialog_open_type);
        }
        else
        {
            if (strlen(path) > 0)
			{
                if( fileFlags.showSaveSimulationDialog)
				{
					simulation->save(path);
                    fileFlags.showSaveSimulationDialog = false;
                	snprintf(path, sizeof(path), "%s", "");
				}
                else if (fileFlags.showOpenSimulationDialog)
                {
                    simulation->read(path);
                    fileFlags.showOpenSimulationDialog = false;
                	snprintf(path, sizeof(path), "%s", "");
                }
                else if (fileFlags.showOpenLayoutDialog)
                {
                    handleOpenLayoutDialog(path);
					fileFlags.showOpenLayoutDialog = false;
                	snprintf(path, sizeof(path), "%s", "");
                }
			}
        }
    }

    void MainWindow::renderAdvancedSettingsWindows()
    {
        if (advancedSettingsFlags.showImGuiDemo)
            ImGui::ShowDemoWindow();
        if (advancedSettingsFlags.showImPlotDemo)
            ImPlot::ShowDemoWindow();
        if(advancedSettingsFlags.showToolMetrics)
			ImGui::ShowMetricsWindow(&advancedSettingsFlags.showToolMetrics);
        if (advancedSettingsFlags.showToolDebugLog)
            ImGui::ShowDebugLogWindow(&advancedSettingsFlags.showToolDebugLog);
        if (advancedSettingsFlags.showToolIdStackTool)
			ImGui::ShowStackToolWindow(&advancedSettingsFlags.showToolIdStackTool);
        if (advancedSettingsFlags.showToolStyleEditor)
        {
	        ImGui::Begin("Dear ImGui Style Editor", 
                &advancedSettingsFlags.showToolStyleEditor);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
        if (advancedSettingsFlags.showToolAbout)
			ImGui::ShowAboutWindow(&advancedSettingsFlags.showToolAbout);
        if (advancedSettingsFlags.showImGuiKitStyleEditor)
			imgui_kit::showImGuiKitThemeSelector(&advancedSettingsFlags.showImGuiKitStyleEditor);
    }

    void MainWindow::handleShortcuts()
    {
        const ImGuiIO& io = ImGui::GetIO();

	    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Space))
			simulation->init();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C))
			simulation->close();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_P))
			simulation->pause();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N))
		{
			simulation->close();
			simulation->clean();
		}
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O))
		{
			FileDialog::file_dialog_open = true;
			fileFlags.showOpenSimulationDialog = true;
			FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
		}
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
            simulation->save();
		if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S))
		{
			FileDialog::file_dialog_open = true;
			fileFlags.showSaveSimulationDialog = true;
			FileDialog::file_dialog_open_type = FileDialog::FileDialogType::SelectFolder;
		}
	    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_L))
	    {
	        FileDialog::file_dialog_open = true;
	        fileFlags.showOpenLayoutDialog = true;
	        FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
	    }
	    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q))
	        std::exit(0);
    }

    void MainWindow::handleOpenLayoutDialog(const char* path)
    {
	    if (std::filesystem::exists(path))
	    {
	        if (std::filesystem::path(path).extension() == ".ini")
	        {
	            const std::string iniFilePathStorage = path;
	            auto io = ImGui::GetIO();
	            io.IniFilename = iniFilePathStorage.c_str();
	            ImGui::LoadIniSettingsFromDisk(io.IniFilename);
	            log(tools::logger::LogLevel::INFO, "Layout file loaded successfully.");
	        }
	        else
	        {
	            log(tools::logger::LogLevel::ERROR, "File is not a .ini file.");
	        }
	    }
	    else
	    {
	        log(tools::logger::LogLevel::ERROR, "File does not exist.");
	    }
    }

	void MainWindow::toggleFixedLayout() const
	{
		if (interfaceFlags.fixedLayout)
		{
			imgui_kit::setGlobalWindowFlags(ImGuiWindowFlags_NoMove |
										   ImGuiWindowFlags_NoCollapse |
										   ImGuiWindowFlags_NoResize);

			log(tools::logger::LogLevel::INFO, "Fixed layout enabled - windows are locked in place.");
		}
		else
		{
			imgui_kit::setGlobalWindowFlags(0);
			log(tools::logger::LogLevel::INFO, "Flexible layout enabled - windows can be moved freely.");
		}
	}
}
