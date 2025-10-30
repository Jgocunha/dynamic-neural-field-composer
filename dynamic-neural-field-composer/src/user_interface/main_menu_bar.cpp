#include "user_interface/main_menu_bar.h"

#include <imgui-platform-kit/themes.h>

namespace dnf_composer::user_interface
{
	MainMenuBar::MainMenuBar(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{}

	void MainMenuBar::render()
	{
		renderMainMenuBar();
		renderFileWindows();
		renderAdvancedSettingsWindows();
		handleShortcuts();
	}

    void MainMenuBar::renderMainMenuBar()
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

                ImGui::Text("Simulation identifier");
            	static char idBuf[128];
            	std::snprintf(idBuf, IM_ARRAYSIZE(idBuf), "%s", simulation->getUniqueIdentifier().c_str());
            	const bool idEdited = ImGui::InputText("##sim_id", idBuf, IM_ARRAYSIZE(idBuf),
										 ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

            	// Commit on Enter or when losing focus after modification
            	if (idEdited || (ImGui::IsItemDeactivatedAfterEdit()))
            		simulation->setUniqueIdentifier(std::string(idBuf));

                ImGui::Separator();

                static auto deltaT = static_cast<float>(simulation->getDeltaT());
                ImGui::Text("Time step (ms) ");
                ImGui::SliderFloat("##menu_deltaT_slider", &deltaT, 0.001f, 25.0, "%.3f");
                if (ImGui::IsItemDeactivatedAfterEdit())
                    simulation->setDeltaT(deltaT);

                ImGui::Separator();

                ImGui::Text("Current time (ms) ");
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

    void MainMenuBar::renderFileWindows()
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
			}
        }
    }

    void MainMenuBar::renderAdvancedSettingsWindows()
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

    void MainMenuBar::handleShortcuts()
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
}
