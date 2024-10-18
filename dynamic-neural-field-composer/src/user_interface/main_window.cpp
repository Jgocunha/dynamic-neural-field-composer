#include "user_interface/main_window.h"

namespace dnf_composer::user_interface
{
	MainWindow::MainWindow(const std::shared_ptr<Simulation>& simulation)
	: simulation{ simulation }
    {
	
	}

	void MainWindow::render()
	{
        // [Put this elsewhere] Enable Keyboard Controls
        auto io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
        // [Put this elsewhere] Using the light style with borders
        ImGui::StyleColorsLight();
        ImGuiStyle& style = ImGui::GetStyle();
        style.FrameBorderSize = 1.0f;

        //renderFullscreenWindow();
		renderMainMenuBar();
        renderFileWindows();
        renderAdvancedSettingsWindows();
        handleShortcuts();
	}

    void MainWindow::renderFullscreenWindow()
    {
        static bool use_work_area = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
        // Based on your use case you may want one or the other.
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        if (ImGui::Begin("Fullscreen window", nullptr, flags))
        {
        }
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
                    fileFlags.showOpenFileDialog = true;
                    FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    simulation->save();
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                {
                    FileDialog::file_dialog_open = true;
                    fileFlags.showSaveFileDialog = true;
                    FileDialog::file_dialog_open_type = FileDialog::FileDialogType::SelectFolder;
                }
                if (ImGui::MenuItem("Quit", "Alt+F4"))
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

            if (ImGui::BeginMenu("Advanced Settings"))
            {
                ImGui::MenuItem("Dear ImGui Metrics/Debugger", nullptr,
                    &advancedSettingsFlags.showToolMetrics);
                ImGui::MenuItem("Dear ImGui Debug Log", nullptr,
                    &advancedSettingsFlags.showToolDebugLog);
                ImGui::MenuItem("Dear ImGui ID Stack Tool", nullptr,
                    &advancedSettingsFlags.showToolIdStackTool);
                ImGui::MenuItem("Dear ImGuiStyle Editor", nullptr,
                    &advancedSettingsFlags.showToolStyleEditor);
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
                if( fileFlags.showSaveFileDialog)
				{
					simulation->save(path);
                    fileFlags.showSaveFileDialog = false;
					strcpy_s(path, "");
				}
                else if (fileFlags.showOpenFileDialog)
                {
                    simulation->read(path);
                    fileFlags.showOpenFileDialog = false;
                    strcpy_s(path, "");
                }
			}
        }
    }

    void MainWindow::renderAdvancedSettingsWindows()
    {
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
    }

    void MainWindow::handleShortcuts()
    {
        const ImGuiIO& io = ImGui::GetIO();

	    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
			simulation->init();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
			simulation->close();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P)))
			simulation->pause();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_N)))
		{
			simulation->close();
			simulation->clean();
		}
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O)))
		{
			FileDialog::file_dialog_open = true;
			fileFlags.showOpenFileDialog = true;
			FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
		}
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
            simulation->save();
		if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
		{
			FileDialog::file_dialog_open = true;
			fileFlags.showSaveFileDialog = true;
			FileDialog::file_dialog_open_type = FileDialog::FileDialogType::SelectFolder;
		}
    }

}
