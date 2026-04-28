#include "user_interface/main_menu_bar.h"


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

                ImGui::Text("Simulation ID");
            	static char idBuf[128];
            	std::snprintf(idBuf, IM_ARRAYSIZE(idBuf), "%s", simulation->getUniqueIdentifier().c_str());
            	const bool idEdited = ImGui::InputText("##sim_id", idBuf, IM_ARRAYSIZE(idBuf),
										 ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

            	// Commit on Enter or when losing focus after modification
            	if (idEdited || (ImGui::IsItemDeactivatedAfterEdit()))
            		simulation->setUniqueIdentifier(std::string(idBuf));

                ImGui::Separator();

                static auto deltaT = static_cast<float>(simulation->getDeltaT());
                ImGui::Text("Simulation time step (dt)");
                ImGui::SliderFloat("##menu_deltaT_slider", &deltaT, 0.1f, 25.0, "%.2f");
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    if (std::isfinite(deltaT) && deltaT > 0.0f)
                        simulation->setDeltaT(deltaT);
                    deltaT = static_cast<float>(simulation->getDeltaT());  // revert to last valid value on invalid input
                }

                ImGui::Separator();

                ImGui::Text("Simulation time");
                ImGui::SameLine();
                ImGui::Text("%.2f", simulation->getT());
                ImGui::SameLine();
                ImGui::Text(" t");

                const long long stepNs = simulation->getLastStepDuration().count();
                ImGui::Text("Real-time per step");
                ImGui::SameLine();
                ImGui::Text("%lld ns", stepNs);

                const long long totalNs  = simulation->getTotalRunDuration().count();
                const long long totalUs  = totalNs / 1'000LL;
                const long long h        = totalUs / 3'600'000'000LL;
                const long long m        = (totalUs % 3'600'000'000LL) / 60'000'000LL;
                const long long s        = (totalUs % 60'000'000LL) / 1'000'000LL;
                const long long ms       = (totalUs % 1'000'000LL) / 1'000LL;
                ImGui::Text("Real time");
                ImGui::SameLine();
                ImGui::Text("%lldh %lldm %llds %lldms", h, m, s, ms);

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
        		static constexpr int presets[] = { 80, 90, 100, 110, 125, 150 };
        		static constexpr int presetCount = IM_ARRAYSIZE(presets);

        		// find index of the current scale in presets (or nearest)
        		const int current = static_cast<int>(Application::getUiScalePct());
        		int currentIdx = 2; // default to 100%
        		for (int i = 0; i < presetCount; ++i)
        			if (presets[i] == current) { currentIdx = i; break; }

        		char previewBuf[16];
        		snprintf(previewBuf, sizeof(previewBuf), "%d%%", current);

        		ImGui::Text("Zoom");
        		ImGui::SameLine();
        		ImGui::SetNextItemWidth(90.0f);
        		if (ImGui::BeginCombo("##zoom", previewBuf, ImGuiComboFlags_HeightSmall))
        		{
        			for (int i = 0; i < presetCount; ++i)
        			{
        				char label[16];
        				snprintf(label, sizeof(label), "%d%%", presets[i]);
        				const bool selected = (presets[i] == current);
        				if (ImGui::Selectable(label, selected))
        					Application::setUiScalePct(static_cast<float>(presets[i]));
        				if (selected)
        					ImGui::SetItemDefaultFocus();
        			}
        			ImGui::EndCombo();
        		}
        		ImGui::SameLine();
        		ImGui::TextDisabled("Ctrl + / Ctrl -");

        		ImGui::Separator();
        		const auto& io = ImGui::GetIO();
        		ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
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

            // Right-aligned version string
            {
                char verBuf[16];
                snprintf(verBuf, sizeof(verBuf), "V %d.%d.%d",
                         DNF_COMPOSER_VERSION_MAJOR, DNF_COMPOSER_VERSION_MINOR, DNF_COMPOSER_VERSION_PATCH);
                const float textW = ImGui::CalcTextSize(verBuf).x;
                const float avail = ImGui::GetContentRegionAvail().x;
                if (avail > textW + 8.0f)
                {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - textW - 4.0f);
                    ImGui::Text("%s", verBuf);
                }
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
			ImGui::ShowIDStackToolWindow(&advancedSettingsFlags.showToolIdStackTool);
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

	    // Zoom in/out through presets
	    static constexpr int presets[] = { 70, 80, 90, 100, 110, 125, 150 };
	    static constexpr int presetCount = IM_ARRAYSIZE(presets);
	    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Equal)) // Ctrl++
	    {
	        const int cur = static_cast<int>(Application::getUiScalePct());
	        for (int i = 0; i < presetCount - 1; ++i)
	            if (presets[i] == cur) { Application::setUiScalePct(static_cast<float>(presets[i + 1])); break; }
	    }
	    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Minus)) // Ctrl+-
	    {
	        const int cur = static_cast<int>(Application::getUiScalePct());
	        for (int i = 1; i < presetCount; ++i)
	            if (presets[i] == cur) { Application::setUiScalePct(static_cast<float>(presets[i - 1])); break; }
	    }
    }
}
