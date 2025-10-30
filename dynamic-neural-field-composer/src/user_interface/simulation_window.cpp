// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "user_interface/simulation_window.h"




namespace dnf_composer::user_interface
{
	SimulationWindow::SimulationWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{
	}

	void SimulationWindow::render()
	{
		if (ImGui::Begin("Simulation Control", nullptr, imgui_kit::getGlobalWindowFlags()))
		{
			renderPanelContents();
		}
		ImGui::End();
	}

	void SimulationWindow::renderPanelContents() const
	{
		renderSimulationControlButtons();
		renderSimulationProperties();
		renderAddElement();          // add elements (NeuralField, GaussStimulus, etc.)
		renderSetInteraction();      // wire up inputs between elements
		renderRemoveElement();       // remove elements
		renderLogElementProperties();// log properties
		renderExportElementComponents(); // export components
	}

	void SimulationWindow::renderSimulationParametersCard() const
	{
		ImGui::PushID("sim_params");

		const float ui = ImGui::GetIO().FontGlobalScale;
		const float gap = ImGui::GetStyle().ItemInnerSpacing.x * 2.0f;

		const std::string id   = simulation->getIdentifier();
		double      dt   = simulation->getDeltaT();

		// keep local editable buffers
		static char idBuf[128];
		std::snprintf(idBuf, IM_ARRAYSIZE(idBuf), "%s", id.c_str());

		// Simulation ID
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Simulation");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(150.0f * ui);
		const bool idEdited = ImGui::InputText("##sim_id", idBuf, IM_ARRAYSIZE(idBuf),
										 ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

		// Commit on Enter or when losing focus after modification
		if (idEdited || (ImGui::IsItemDeactivatedAfterEdit()))
			simulation->setUniqueIdentifier(std::string(idBuf));

		// spacer between the two groups
		ImGui::SameLine(0.0f, gap);

		// Time step delta_t
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Time step");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(65.0f * ui);
		const bool dtEdited = ImGui::InputDouble("##dt_ms", &dt, 0.0, 0.0, "%.2f");

		if (dtEdited || ImGui::IsItemDeactivatedAfterEdit())
			simulation->setDeltaT(dt);

		ImGui::SameLine();
		ImGui::TextUnformatted("ms");

		ImGui::PopID();
	}

	void SimulationWindow::renderSimulationControlsCard() const
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const ImGuiStyle& st = ImGui::GetStyle();

		// Tile size + spacing
		const float tile = 90.0f * ui;
		const float gap  = st.ItemSpacing.x * 2.5f;

		// Palette
		constexpr auto bg      = ImVec4(0.96f, 0.98f, 0.99f, 1.0f);
		constexpr auto hover   = ImVec4(0.90f, 0.97f, 0.94f, 1.0f);
		constexpr auto active  = ImVec4(0.85f, 0.96f, 0.92f, 1.0f);
		const auto iconCol = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);  // icon glyph color
		const auto labelCol= ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);  // text below

		const ImU32 cBg      = ImGui::GetColorU32(bg);
		const ImU32 cHover   = ImGui::GetColorU32(hover);
		const ImU32 cActive  = ImGui::GetColorU32(active);
		const ImU32 cIcon    = ImGui::GetColorU32(iconCol);
		const ImU32 cLabel   = ImGui::GetColorU32(labelCol);

		ImGui::BeginGroup();

		if (widgets::renderIconTileButton("run",     ICON_FA_PLAY,         "Run",     tile, ui, cBg, cHover, cActive, cIcon, cLabel))
			simulation->init();

		ImGui::SameLine(0.0f, gap);
		if (widgets::renderIconTileButton("pause",   ICON_FA_PAUSE,        "Pause",   tile, ui, cBg, cHover, cActive, cIcon, cLabel))
			simulation->pause();

		ImGui::SameLine(0.0f, gap);
		if (widgets::renderIconTileButton("resume",  ICON_FA_FORWARD_FAST, "Resume",  tile, ui, cBg, cHover, cActive, cIcon, cLabel))
			simulation->resume();

		ImGui::SameLine(0.0f, gap);
		if (widgets::renderIconTileButton("stop",    ICON_FA_STOP,         "Stop",    tile, ui, cBg, cHover, cActive, cIcon, cLabel))
			simulation->close();

		ImGui::EndGroup();
	}

	void SimulationWindow::renderRunForIterationsCard() const
	{
		 ImGui::PushID("run_for_iterations_inline");

	    const float ui   = ImGui::GetIO().FontGlobalScale;
	    const float gap  = ImGui::GetStyle().ItemInnerSpacing.x * 2.0f;

	    // persistent state for this row
	    static int   iterationCount   = 1000;
	    static bool  runNSteps        = false;
	    static int   startIteration   = 0;

	    // "Run simulation for [ N ] iterations"
	    ImGui::AlignTextToFramePadding();
	    ImGui::TextUnformatted("Run simulation for");
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	    ImGui::SetNextItemWidth(60.0f * ui);
	    if (ImGui::InputInt("##iter_count", &iterationCount, 0, 0))
	        if (iterationCount < 1) iterationCount = 1;

	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	    ImGui::AlignTextToFramePadding();
	    ImGui::TextUnformatted("iterations");

	    // ---- play icon button at the end of the line ----
	    ImGui::SameLine(0.0f, gap);
	    const float tile = 36.0f * ui;                       // compact square
	    const ImVec2 iconBox(tile, tile);

	    // soft “tile”
	    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f * ui);
	    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0, 0));
	    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.96f, 0.98f, 0.99f, 1.0f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.97f, 0.94f, 1.0f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.85f, 0.96f, 0.92f, 1.0f));
	    ImGui::PushStyleColor(ImGuiCol_Text,          ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));

	    // center the glyph inside the square
	    ImGui::PushFont(g_MediumIconsFont);
	    const bool clicked = ImGui::Button(ICON_FA_PLAY, iconBox);
	    ImGui::PopFont();
	    ImGui::PopStyleColor(4);
	    ImGui::PopStyleVar(2);

	    if (clicked)
	    {
	        startIteration = simulation->getT();         // where we start
	        if (!simulation->isInitialized())            // init if needed
	        {
	            simulation->init();
	            startIteration = 0;
	        }
	        simulation->resume();                        // run
	        runNSteps = true;
	        tools::logger::log(tools::logger::INFO,
	            "Simulation has started running for " + std::to_string(iterationCount) + " steps.");
	    }

	    // auto-stop after N steps
	    if (runNSteps)
	    {
	        if (simulation->getT() >= startIteration + iterationCount)
	        {
	            runNSteps = false;
	            simulation->pause();
	            tools::logger::log(tools::logger::INFO,
	                "Simulation has finished running " + std::to_string(iterationCount) + " steps.");
	        }
	    }

	    ImGui::PopID();
	}


	void SimulationWindow::renderAddElementCard() const
	{
		ImGui::PushID("add_element_section");

		// Headline
		ImGui::PushFont(g_BoldFont);
		ImGui::TextUnformatted("Add elements");
		ImGui::PopFont();

	    ImGui::Columns(2, nullptr, false);

		// Left column
		ImGui::TextUnformatted("Select type");

		// Make the combo span the whole column width
		ImGui::SetNextItemWidth(-FLT_MIN);          // or ImGui::PushItemWidth(-FLT_MIN);
		static element::ElementLabel selected = element::ElementLabel::NEURAL_FIELD;
		const char* current = element::ElementLabelToString.at(selected).c_str();
		if (ImGui::BeginCombo("##element_type", current))
		{
			for (const auto& [fst, snd] : element::ElementLabelToString)
			{
				if (fst == element::ElementLabel::UNINITIALIZED) continue;
				const bool is_selected = (selected == fst);
				if (ImGui::Selectable(snd.c_str(), is_selected)) selected = fst;
				if (is_selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f,0.75f,0.40f,0.18f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.75f,0.40f,0.28f));
		const bool addRequested = ImGui::Button("Add element", ImVec2(-FLT_MIN, 0));
		ImGui::PopStyleColor(3);

		// Right column
	    ImGui::NextColumn();
	    ImGui::TextUnformatted("Define parameters");

	    // Fixed-size, scrollable pane for parameters
	    // (tweak the height multiplier if you want more/less visible rows)
	    const float col_w   = ImGui::GetColumnWidth();
	    const float pad_w   = ImGui::GetStyle().ItemSpacing.x * 1.0f;
	    const float child_w = col_w - pad_w; // fill the column width
	    const float child_h = 280.0f * ImGui::GetIO().FontGlobalScale;

		constexpr ImGuiWindowFlags child_flags =
	        ImGuiWindowFlags_AlwaysVerticalScrollbar |
	        ImGuiWindowFlags_AlwaysHorizontalScrollbar |
	        ImGuiWindowFlags_NoSavedSettings;

	    if (ImGui::BeginChild("##params_scroll", ImVec2(child_w, child_h), true, child_flags))
	    {
	        switch (selected)
	        {
	        case element::ElementLabel::NEURAL_FIELD:
	        {
	            static char   id[CHAR_SIZE] = "neural field u";
	            static int    x_max         = 100;
	            static double d_x           = 1.0;
	            static double resting       = -10.0;
	            static double tau           = 25.0;
	            static double sigmoid_k     = 5.0;


		        	ImGui::InputTextWithHint("ID", "enter text here", id, IM_ARRAYSIZE(id));
		        	ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale); // adjust width to taste
		        	ImGui::InputInt("Size", &x_max, 0, 0);
		        	ImGui::InputDouble("Step", &d_x, 0.0, 0.0, "%.2f");
		        	ImGui::InputDouble("Resting level", &resting, 0.0, 0.0, "%.2f");
		        	ImGui::InputDouble("Time scale", &tau, 0.0, 0.0, "%.2f");
		        	ImGui::InputDouble("Sigmoid steepness", &sigmoid_k, 0.0, 0.0, "%.2f");
		        	ImGui::PopItemWidth();

	            if (addRequested)
	            {
	                const element::SigmoidFunction activation{ 0, sigmoid_k };
	                const element::NeuralFieldParameters nfp{ tau, resting, activation };
	                const element::ElementIdentifiers ids{ id };
	                const element::ElementDimensions  dims{ x_max, d_x };
	                const element::ElementCommonParameters common{ ids, dims };
	                const auto nf = std::make_shared<element::NeuralField>(common, nfp);
	                simulation->addElement(nf);
	            }
	            break;
	        }
	        default:
	            ImGui::TextDisabled("Parameters UI coming next for this type.");
	            break;
	        }

	        // (Optional) ensure a small extra width, so a horizontal bar is always usable
	        // ImGui::Dummy(ImVec2(child_w + 120.0f, 0));
	    }
	    ImGui::EndChild();

	    ImGui::Columns(1);
	    ImGui::PopID();
	}

	void SimulationWindow::renderRemoveElementCard() const
	{
	    ImGui::PushID("remove_element_inline");

		// Headline
		ImGui::PushFont(g_BoldFont);
		ImGui::TextUnformatted("Remove elements");
		ImGui::PopFont();

	    //  Row: "Remove [combo] from simulation"
	    // Make the label baseline align with framed widgets (combo).
	    ImGui::AlignTextToFramePadding();
	    ImGui::TextUnformatted("Remove");

	    // Keep a little inner spacing before the combo
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	    static std::string selectedId; // persists across frames

	    // Combo: list all current elements
	    const char* preview = selectedId.empty() ? "element" : selectedId.c_str();
	    ImGui::SetNextItemWidth(180.0f * ImGui::GetIO().FontGlobalScale);
	    if (ImGui::BeginCombo("##element_combo", preview))
	    {
	        for (const auto& e : simulation->getElements())
	        {
	            const std::string& name = e->getUniqueName();
	            const bool is_sel = (selectedId == name);
	            if (ImGui::Selectable(name.c_str(), is_sel))
	                selectedId = name;
	            if (is_sel) ImGui::SetItemDefaultFocus();
	        }
	        ImGui::EndCombo();
	    }

	    // Keep text on the same baseline as combo
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	    ImGui::AlignTextToFramePadding();
	    ImGui::TextUnformatted("from simulation");

	    //  Trash icon button (right after the sentence)
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	    const float line_h  = ImGui::GetFrameHeight();
	    const ImVec2 iconSz(line_h + 10.0f, line_h);   // slightly not square button

	    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f,0.0f,0.0f,0.15f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f,0.0f,0.0f,0.25f));

	    ImGui::PushFont(g_MediumIconsFont);
	    const bool clicked = ImGui::Button(ICON_FA_TRASH, iconSz);
	    ImGui::PopFont();

	    ImGui::PopStyleColor(3);

	    if (clicked && !selectedId.empty())
	    {
	        simulation->removeElement(selectedId);
	        simulation->init();
	    }

	    ImGui::PopID();
	}

	void SimulationWindow::renderSetInteractionCard() const
	{
		ImGui::PushID("set_interactions_section");

	    // Headline
	    ImGui::PushFont(g_BoldFont);
	    ImGui::TextUnformatted("Set interactions");
	    ImGui::PopFont();

	    // Two columns like the mock
	    ImGui::Columns(2, nullptr, false);

	    //  Left column: target/source + connect button
	    const float leftW = 220.0f * ImGui::GetIO().FontGlobalScale;
	    ImGui::SetColumnWidth(0, leftW);

	    // Combos persist selections
	    static std::string selectedTarget;
	    static std::string selectedSource;

	    // Helpers to render a compact combo bound to a string
	    auto ComboFromElements = [&](const char* label, std::string& value)
	    {
	        ImGui::AlignTextToFramePadding();
	        ImGui::TextUnformatted(label);
	        ImGui::SetNextItemWidth(leftW - 20.0f * ImGui::GetIO().FontGlobalScale);
	        const char* preview = value.empty() ? label : value.c_str();
	        if (ImGui::BeginCombo((std::string("##") + label).c_str(), preview))
	        {
	            for (const auto& e : simulation->getElements())
	            {
	                const std::string& name = e->getUniqueName();
	                const bool is_sel = (value == name);
	                if (ImGui::Selectable(name.c_str(), is_sel))
	                    value = name;
	                if (is_sel) ImGui::SetItemDefaultFocus();
	            }
	            ImGui::EndCombo();
	        }
	    };

	    ComboFromElements("Target element", selectedTarget);
	    ComboFromElements("Source element", selectedSource);

	    ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f,0.75f,0.40f,0.18f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.75f,0.40f,0.28f));

		const float btnW = leftW - 15.0f;                             // same width as combos
		const float btnH = ImGui::GetFrameHeight() * 1.2f;   // a bit taller looks nicer

		// Disable until both target & source are chosen
		const bool canConnect = !selectedTarget.empty() && !selectedSource.empty();
		ImGui::BeginDisabled(!canConnect);
		const bool connectPressed = ImGui::Button("Connect", ImVec2(btnW, btnH));
		ImGui::EndDisabled();
		ImGui::PopStyleColor(3);

		if (connectPressed)
		{
			const auto target = simulation->getElement(selectedTarget);
			const auto input  = simulation->getElement(selectedSource);
			if (target && input && target->getUniqueIdentifier() != input->getUniqueIdentifier())
			{
				target->addInput(input);
				simulation->init();
			}
		}

	    //  Right column: current connections list (scrollable)
	    ImGui::NextColumn();
	    ImGui::TextUnformatted("Connected elements");
		ImGui::SameLine();
		widgets::renderHelpMarker("You can disconnect elements by pressing the 'unlink' buttons.");

	    const float listH = 210.0f * ImGui::GetIO().FontGlobalScale;
	    ImGui::BeginChild("##connections_scroll", ImVec2(0, listH), true,
	                      ImGuiWindowFlags_AlwaysVerticalScrollbar |
	                      ImGuiWindowFlags_NoSavedSettings);

	    if (!selectedTarget.empty())
	    {
		    if (const auto target = simulation->getElement(selectedTarget))
	        {
	            const auto& inputs = target->getInputs();
	            if (inputs.empty())
	            {
	                ImGui::TextDisabled("No connections.");
	            }
	            else
	            {
	                int idx = 0;
	                for (const auto& conn : inputs)
	                {
	                    ImGui::PushID(idx);
	                    // Row: element name + remove icon on the right
	                    ImGui::TextUnformatted(conn->getUniqueName().c_str());

	                    // Right-aligned red "unlink" icon
	                    ImGui::SameLine();
	                    const float h = ImGui::GetFrameHeight();

	                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f,0.0f,0.0f,0.15f));
	                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f,0.0f,0.0f,0.25f));
	                    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.90f,0.10f,0.10f,1.0f));
	                    ImGui::PushFont(g_MediumIconsFont);
	                    const bool removeClicked = ImGui::Button(ICON_FA_LINK_SLASH, ImVec2(h + 10.0f, h));
	                    ImGui::PopFont();
	                    ImGui::PopStyleColor(4);

	                    if (removeClicked)
	                    {
	                        target->removeInput(conn->getUniqueIdentifier());
	                        simulation->init();
	                    }

	                    ImGui::PopID();
	                    ++idx;
	                }
	            }
	        }
	    }
	    ImGui::EndChild();

	    ImGui::Columns(1);
	    ImGui::PopID();
	}

	void SimulationWindow::renderExportElementComponentCard() const
	{
		 ImGui::PushID("export_inline");

		// Headline
		ImGui::PushFont(g_BoldFont);
		ImGui::TextUnformatted("Export data");
		ImGui::PopFont();

	    // Baseline-aligned sentence with inline combos:
	    ImGui::AlignTextToFramePadding();
	    ImGui::TextUnformatted("Export");
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	    // Element combo
	    static std::string selectedElementId;
	    const char* elemPreview = selectedElementId.empty() ? "element" : selectedElementId.c_str();
	    ImGui::SetNextItemWidth(180.0f * ImGui::GetIO().FontGlobalScale);
	    if (ImGui::BeginCombo("##export_elem_combo", elemPreview))
	    {
	        for (const auto& e : simulation->getElements())
	        {
	            const std::string& name = e->getUniqueName();
	            const bool is_sel = (selectedElementId == name);
	            if (ImGui::Selectable(name.c_str(), is_sel))
	                selectedElementId = name;
	            if (is_sel) ImGui::SetItemDefaultFocus();
	        }
	        ImGui::EndCombo();
	    }

	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	    ImGui::AlignTextToFramePadding();

	    // Component combo (depends on selected element)
	    static std::string selectedComponent;
	    const char* compPreview = selectedComponent.empty() ? "component" : selectedComponent.c_str();
	    ImGui::SetNextItemWidth(160.0f * ImGui::GetIO().FontGlobalScale);

	    if (ImGui::BeginCombo("##export_comp_combo", compPreview))
	    {
	        if (!selectedElementId.empty())
	        {
	            if (const auto elem = simulation->getElement(selectedElementId))
	            {
	                for (const auto& comp : elem->getComponentList())
	                {
	                    const bool is_sel = (selectedComponent == comp);
	                    if (ImGui::Selectable(comp.c_str(), is_sel))
	                        selectedComponent = comp;
	                    if (is_sel) ImGui::SetItemDefaultFocus();
	                }
	            }
	        }
	        ImGui::EndCombo();
	    }

	    // Inline download icon button (directly after the component combo)
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	    const float h = ImGui::GetFrameHeight();
	    const ImVec2 iconSz(h + 10.0f, h);

	    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f,0.75f,0.40f,0.18f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.75f,0.40f,0.28f));
	    ImGui::PushFont(g_MediumIconsFont);
	    const bool exportClicked = ImGui::Button(ICON_FA_DOWNLOAD, iconSz);
	    ImGui::PopFont();
	    ImGui::PopStyleColor(3);

	    if (exportClicked && !selectedElementId.empty() && !selectedComponent.empty())
	        simulation->exportComponentToFile(selectedElementId, selectedComponent);

	    ImGui::PopID();
	}

	void SimulationWindow::renderLogElementParametersCard() const
	{
		ImGui::PushID("log_inline");

		// Headline
		ImGui::PushFont(g_BoldFont);
		ImGui::TextUnformatted("Log parameters");
		ImGui::PopFont();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Log");
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		// Element combo
		static std::string selectedId;
		const char* preview = selectedId.empty() ? "element" : selectedId.c_str();
		ImGui::SetNextItemWidth(180.0f * ImGui::GetIO().FontGlobalScale);
		if (ImGui::BeginCombo("##log_elem_combo", preview))
		{
			for (const auto& e : simulation->getElements())
			{
				const std::string& name = e->getUniqueName();
				const bool is_sel = (selectedId == name);
				if (ImGui::Selectable(name.c_str(), is_sel))
					selectedId = name;
				if (is_sel) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("parameters");

		// Inline terminal icon button (directly after a text)
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		const float h = ImGui::GetFrameHeight();
		const ImVec2 iconSz(h + 10.0f, h);

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f,0.75f,0.40f,0.18f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.75f,0.40f,0.28f));
		ImGui::PushFont(g_MediumIconsFont);
		const bool logClicked = ImGui::Button(ICON_FA_TERMINAL, iconSz);
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		if (logClicked && !selectedId.empty())
			if (const auto e = simulation->getElement(selectedId)) e->print();

		ImGui::PopID();
	}


	void SimulationWindow::renderSimulationControlButtons() const
	{

		if (ImGui::Button("Start"))
			simulation->init();

		ImGui::SameLine();

		if (ImGui::Button("Pause"))
			simulation->pause();

		ImGui::SameLine();

		if (ImGui::Button("Resume"))
			simulation->resume();

		ImGui::SameLine();

		if (ImGui::Button("Stop"))
			simulation->close();

		// Section for running a specific number of iterations
		ImGui::Separator();

		static bool runNSteps = false;
		static int iterationCount = 1000; // Default value
		static int currentIteration = 0;
		ImGui::SetNextItemWidth(120);
		ImGui::InputInt("Iterations", &iterationCount, 1, 10);
		if (iterationCount < 1) iterationCount = 1; // Ensure positive value

		ImGui::SameLine();
		if (ImGui::Button("Run N steps"))
		{
			// Run the simulation for the specified number of iterations
			currentIteration = simulation->getT();
			if (!simulation->isInitialized())
			{
				simulation->init();
				currentIteration = 0;
			}
			simulation->resume();
			runNSteps = true;
			tools::logger::log(tools::logger::INFO, "Simulation has started running for "
				+ std::to_string(iterationCount) + " steps.");
		}

		if (runNSteps)
		{
			// simulation has finished running the specified number of iterations
			if (simulation->getT() >= iterationCount + currentIteration)
			{
				runNSteps = false;
				simulation->pause();
				tools::logger::log(tools::logger::INFO, "Simulation has finished running "
					+ std::to_string(iterationCount) + " steps.");
			}
		}

	}

	void SimulationWindow::renderSimulationProperties() const
	{
		ImGui::Separator();

		const std::string& identifier = simulation->getIdentifier();
		const double deltaT = simulation->getDeltaT();
		const double tZero = simulation->getTZero();
		const double t = simulation->getT();

		ImGui::Text("Identifier: ");
		ImGui::SameLine();
		ImGui::TextUnformatted(identifier.c_str());

		ImGui::Text("Time Step (deltaT): ");
		ImGui::SameLine();
		ImGui::Text("%.3f", deltaT);

		ImGui::Text("Start Time (tZero): ");
		ImGui::SameLine();
		ImGui::Text("%.3f", tZero);

		ImGui::Text("Current Time (t): ");
		ImGui::SameLine();
		ImGui::Text("%.3f", t);

		ImGui::Separator();
	}

	void SimulationWindow::renderAddElement() const
	{
		ImGui::PushID("add element");
		if (ImGui::CollapsingHeader("Add element"))
		{
			for (const auto& pair : element::ElementLabelToString)
				if (pair.first != element::ElementLabel::UNINITIALIZED)
					renderElementProperties(pair);
		}
		ImGui::PopID();
	}

	void SimulationWindow::renderSetInteraction() const
	{
		ImGui::PushID("set interaction");
		if (ImGui::CollapsingHeader("Set interactions between elements"))
		{
			for (const auto& element : simulation->getElements())
			{
				const auto elementId = element->getUniqueName();
				if (ImGui::TreeNode(elementId.c_str()))
				{
					static std::string selectedElementId{};
					static int currentElementIdx = 0;

					// Section 1: Add New Input
					ImGui::Text("Select the element you want to define as input");
					if (ImGui::BeginListBox("##Element list available as inputs"))
					{
						for (const auto& other_element : simulation->getElements())
						{
							std::string inputElementId = other_element->getUniqueName();
							const bool isSelected = (currentElementIdx == other_element->getUniqueIdentifier());
							if (ImGui::Selectable(inputElementId.c_str(), isSelected))
							{
								selectedElementId = inputElementId;
								currentElementIdx = other_element->getUniqueIdentifier();
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}

					if (ImGui::Button("Add", { 100.0f, 30.0f }))
					{
						auto input = simulation->getElement(selectedElementId);
						element->addInput(input);
						simulation->init();
					}

					// Section 2: Show Existing Connections
					ImGui::Separator();
					ImGui::Text("Currently connected elements:");
					const auto& inputs = element->getInputs();
					if (inputs.empty())
					{
						ImGui::Text("No connections.");
					}
					else
					{
						for (size_t i = 0; i < inputs.size(); ++i)
						{
							const auto& connectedElement = inputs[i];
							ImGui::BulletText("%s", connectedElement->getUniqueName().c_str());

							// Add a "Remove" button for each connection
							ImGui::SameLine();
							std::string buttonLabel = "Remove##" + std::to_string(i);
							if (ImGui::Button(buttonLabel.c_str()))
							{
								element->removeInput(connectedElement->getUniqueIdentifier());
								simulation->init();
							}
						}
					}
					ImGui::Separator();

					ImGui::TreePop();
				}
			}
		}
		ImGui::PopID();
	}

	void SimulationWindow::renderRemoveElement() const
	{
		ImGui::PushID("remove element");
		if (ImGui::CollapsingHeader("Remove elements from simulation"))
		{
			for (const auto& element : simulation->getElements())
			{
				const auto elementId = element->getUniqueName();
				if (ImGui::TreeNode(elementId.c_str()))
				{
					if (ImGui::Button("Remove", { 100.0f, 30.0f }))
					{
						simulation->removeElement(elementId);
						simulation->init();
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::PopID();
	}

	void SimulationWindow::renderElementProperties(const std::pair<int, std::string>& elementId) const
	{
		if (ImGui::TreeNode(elementId.second.c_str()))
		{
			switch (elementId.first)
			{
			case element::ElementLabel::NEURAL_FIELD:
				addElementNeuralField();
				break;
			case element::ElementLabel::GAUSS_STIMULUS:
				addElementGaussStimulus();
				break;
			case element::ElementLabel::FIELD_COUPLING:
				addElementFieldCoupling();
				break;
			case element::ElementLabel::GAUSS_KERNEL:
				addElementGaussKernel();
				break;
			case element::ElementLabel::MEXICAN_HAT_KERNEL:
				addElementMexicanHatKernel();
				break;
			case element::ElementLabel::NORMAL_NOISE:
				addElementNormalNoise();
				break;
			case element::ElementLabel::GAUSS_FIELD_COUPLING:
				addElementGaussFieldCoupling();
				break;
			default:
				log(tools::logger::LogLevel::ERROR, "There is a missing element in the TreeNode in simulation window.");
				break;
			}
			ImGui::TreePop();
		}
	}

	void SimulationWindow::renderLogElementProperties() const
	{
		ImGui::PushID("log element parameters");
		if (ImGui::CollapsingHeader("Log element parameters"))
		{
			for (const auto& element : simulation->getElements())
			{
				const auto elementId = element->getUniqueName();
				if (ImGui::TreeNode(elementId.c_str()))
				{
					if (ImGui::Button("Log", { 100.0f, 30.0f }))
											{
						element->print();
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::PopID();
	}

	void SimulationWindow::renderExportElementComponents() const
	{
		ImGui::PushID("export element components");
		if (ImGui::CollapsingHeader("Export element components"))
		{
			for (const auto& element : simulation->getElements())
			{
				const auto elementId = element->getUniqueName();
				if (ImGui::TreeNode(elementId.c_str()))
				{
					for (const auto& componentName : element->getComponentList())
					{
						if (ImGui::TreeNode(componentName.c_str()))
						{
							if (ImGui::Button("Export", { 100.0f, 30.0f }))
							{
								simulation->exportComponentToFile(elementId, componentName);
							}
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::PopID();
	}

	void SimulationWindow::addElementNeuralField() const
	{
		ImGui::PushID("neural field");
		static char id[CHAR_SIZE] = "neural field u";
		ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
		static int x_max = 100;
		ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
		static double d_x = 1.0;
		ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
		static double tau = 25;
		ImGui::InputDouble("tau", &tau, 1.0f, 10.0f, "%.2f");
		static double sigmoidSteepness = 5.0f;
		ImGui::InputDouble("sigmoid steepness", &sigmoidSteepness, 1.0f, 10.0f, "%.2f");
		static double restingLevel = -10.0f;
		ImGui::InputDouble("resting level", &restingLevel, 1.0f, 10.0f, "%.2f");

		if (ImGui::Button("Add", { 100.0f, 30.0f }))
		{
			const element::SigmoidFunction activationFunction{ 0, sigmoidSteepness };
			const element::NeuralFieldParameters nfp = { tau, restingLevel, activationFunction };

			const element::ElementIdentifiers neuralFieldIdentifiers{id};
			const element::ElementDimensions neuralFieldDimensions{ x_max, d_x };
			const element::ElementCommonParameters commonParameters{ neuralFieldIdentifiers, neuralFieldDimensions };

			const std::shared_ptr<element::NeuralField> neuralField(new element::NeuralField(commonParameters, nfp));
			simulation->addElement(neuralField);
		}
		ImGui::PopID();
	}

	void SimulationWindow::addElementGaussStimulus() const
	{
		ImGui::PushID("gauss stimulus");
		static char id[CHAR_SIZE] = "gauss stimulus a";
		ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
		static int x_max = 100;
		ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
		static double d_x = 1.0;
		ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
		static double sigma = 5;
		ImGui::InputDouble("sigma", &sigma, 1.0f, 10.0f, "%.2f");
		static double amplitude = 20;
		ImGui::InputDouble("amplitude", &amplitude, 1.0f, 10.0f, "%.2f");
		static double position = 50;
		ImGui::InputDouble("position", &position, 1.0f, 10.0f, "%.2f");
		static bool normalized = false;
		ImGui::Checkbox("normalized", &normalized);
		static bool circular = false;
		ImGui::Checkbox("circular", &circular);

		if (ImGui::Button("Add", { 100.0f, 30.0f }))
		{
			const element::GaussStimulusParameters gsp = { sigma, amplitude, position, circular, normalized};
			const element::ElementDimensions dimensions{ x_max, d_x };
			const std::shared_ptr<element::GaussStimulus> gaussStimulus(new element::GaussStimulus ({id, dimensions}, gsp));
			simulation->addElement(gaussStimulus);
		}
		ImGui::PopID();
	}

	void SimulationWindow::addElementNormalNoise() const
	{
		ImGui::PushID("normal noise");
		static char id[CHAR_SIZE] = "normal noise a";
		ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
		static int x_max = 100;
		ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
		static double d_x = 1.0;
		ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
		static double amplitude = 0.01;
		ImGui::InputDouble("amplitude", &amplitude, 0.01f, 1.0f, "%.2f");

		if (ImGui::Button("Add", { 100.0f, 30.0f }))
		{
			const element::NormalNoiseParameters nnp = { amplitude };
			const element::ElementDimensions dimensions{ x_max, d_x };
			const std::shared_ptr<element::NormalNoise> normalNoise( new element::NormalNoise({ id, dimensions}, nnp));
			const element::GaussKernelParameters gkp = { 0.25, 0.2 };
			const std::shared_ptr<element::GaussKernel> gaussKernelNormalNoise(new element::GaussKernel({ std::string(id) + " gauss kernel", dimensions }, gkp));
			simulation->addElement(normalNoise);
			simulation->addElement(gaussKernelNormalNoise);
			simulation->createInteraction(id, "output", std::string(id) + " gauss kernel");
			simulation->createInteraction(std::string(id) + " gauss kernel", "output", id);
		}
		ImGui::PopID();
	}

	void SimulationWindow::addElementFieldCoupling() const
	{
		ImGui::PushID("field coupling");
		static char id[CHAR_SIZE] = "field coupling u -> v";
		ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
		static int x_max = 100;
		ImGui::InputInt("output x_max", &x_max, 1.0, 10.0);
		static double d_x = 1.0;
		ImGui::InputDouble("output d_x", &d_x, 0.1, 0.5, "%.2f");
		static int in_x_max = 100;
		ImGui::InputInt("input x_max", &in_x_max, 1.0, 10.0);
		static double in_d_x = 1.0;
		ImGui::InputDouble("input d_x", &in_d_x, 0.1, 0.5, "%.2f");
		static LearningRule learningRule = LearningRule::HEBB;
		if (ImGui::BeginCombo("learning rule", LearningRuleToString.at(learningRule).c_str()))
		{
			for (size_t i = 0; i < LearningRuleToString.size(); ++i)
			{
				const char* name = LearningRuleToString.at(static_cast<LearningRule>(i)).c_str();
				if (ImGui::Selectable(name, learningRule == static_cast<LearningRule>(i)))
				{
					learningRule = static_cast<LearningRule>(i);
				}
			}
			ImGui::EndCombo();
		}
		static double scalar = 1.0;
		ImGui::InputDouble("scalar", &scalar, 0.1f, 1.0f, "%.2f");
		static double learningRate = 0.01;
		ImGui::InputDouble("learning rate", &learningRate, 0.01f, 0.1f, "%.2f");

		if(ImGui::Button("Add", { 100.0f, 30.0f }))
		{
			const element::FieldCouplingParameters fcp = { {in_x_max, in_d_x}, learningRule, scalar, learningRate };
			const element::ElementDimensions dimensions{ x_max, d_x };
			const std::shared_ptr<element::FieldCoupling> fieldCoupling(new element::FieldCoupling({ id, dimensions }, fcp));
			simulation->addElement(fieldCoupling);
		}
		ImGui::PopID();
	}

	void SimulationWindow::addElementGaussFieldCoupling() const
	{
		ImGui::PushID("gauss field coupling");
		static char id[CHAR_SIZE] = "gauss field coupling u -> v";
		ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
		static int x_max = 100;
		ImGui::InputInt("output x_max", &x_max, 1.0, 10.0);
		static double d_x = 1.0;
		ImGui::InputDouble("output d_x", &d_x, 0.1, 0.5, "%.2f");
		static int in_x_max = 100;
		ImGui::InputInt("input x_max", &in_x_max, 1.0, 10.0);
		static double in_d_x = 1.0;
		ImGui::InputDouble("input d_x", &in_d_x, 0.1, 0.5, "%.2f");
		static double x_i = 1;
		ImGui::InputDouble("x_i", &x_i, 1.0f, 10.0f, "%.2f");
		static double x_j = 1;
		ImGui::InputDouble("x_j", &x_j, 1.0f, 10.0f, "%.2f");
		static double amplitude = 5;
		ImGui::InputDouble("amplitude", &amplitude, 1.0f, 10.0f, "%.2f");
		static double width = 5;
		ImGui::InputDouble("width", &width, 1.0f, 10.0f, "%.2f");
		static bool normalized = true;
		ImGui::Checkbox("normalized", &normalized);
		static bool circular = false;
		ImGui::Checkbox("circular", &circular);

		if (ImGui::Button("Add", { 100.0f, 30.0f }))
		{
			const element::GaussFieldCouplingParameters gfcp = { {in_x_max, in_d_x}, normalized, circular, {{x_i, x_j, amplitude, width}} };
			const element::ElementDimensions dimensions{ x_max, d_x };
			const std::shared_ptr<element::GaussFieldCoupling> gaussCoupling(new element::GaussFieldCoupling({ id, dimensions }, gfcp));
			simulation->addElement(gaussCoupling);
		}
		ImGui::PopID();
	}

	void SimulationWindow::addElementGaussKernel() const
	{
		ImGui::PushID("gauss kernel");
		static char id[CHAR_SIZE] = "gauss kernel u -> u";
		ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
		static int x_max = 100;
		ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
		static double d_x = 1.0;
		ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
		static double sigma = 20;
		ImGui::InputDouble("sigma", &sigma, 1.0f, 10.0f, "%.2f");
		static double amplitude = 2;
		ImGui::InputDouble("amplitude", &amplitude, 1.0f, 10.0f, "%.2f");
		static double amplitudeGlobal = -0.01;
		ImGui::InputDouble("amplitudeGlobal", &amplitudeGlobal, -0.01f, -0.1f, "%.2f");
		static bool normalized = true;
		ImGui::Checkbox("normalized", &normalized);
		static bool circular = false;
		ImGui::Checkbox("circular", &circular);

		if (ImGui::Button("Add", { 100.0f, 30.0f }))
		{
			const element::GaussKernelParameters gkp = { sigma, amplitude, amplitudeGlobal, circular, normalized};
			const element::ElementDimensions dimensions{ x_max, d_x };
			const std::shared_ptr<element::GaussKernel> gaussKernel(new element::GaussKernel({ id, dimensions }, gkp));
			simulation->addElement(gaussKernel);
		}
		ImGui::PopID();
	}

	void SimulationWindow::addElementMexicanHatKernel() const
	{
		ImGui::PushID("mexican hat kernel");
		static char id[CHAR_SIZE] = "mexican hat kernel u -> u";
		ImGui::InputTextWithHint("id", "enter text here", id, IM_ARRAYSIZE(id));
		static int x_max = 100;
		ImGui::InputInt("x_max", &x_max, 1.0, 10.0);
		static double d_x = 1.0;
		ImGui::InputDouble("d_x", &d_x, 0.1, 0.5, "%.2f");
		static double sigmaExc = 5;
		ImGui::InputDouble("sigmaExc", &sigmaExc, 1.0f, 10.0f, "%.2f");
		static double amplitudeExc = 15;
		ImGui::InputDouble("amplitudeExc", &amplitudeExc, 1.0f, 10.0f, "%.2f");
		static double sigmaInh = 10;
		ImGui::InputDouble("sigmaInh", &sigmaInh, 1.0f, 10.0f, "%.2f");
		static double amplitudeInh = 15;
		ImGui::InputDouble("amplitudeInh", &amplitudeInh, 1.0f, 10.0f, "%.2f");
		static double amplitudeGlobal = -0.01;
		ImGui::InputDouble("amplitudeGlobal", &amplitudeGlobal, -0.01f, -0.1f, "%.2f");
		static bool normalized = true;
		ImGui::Checkbox("normalized", &normalized);
		static bool circular = false;
		ImGui::Checkbox("circular", &circular);


		if (ImGui::Button("Add", { 100.0f, 30.0f }))
		{
			const element::MexicanHatKernelParameters mhkp = { sigmaExc, amplitudeExc, sigmaInh, amplitudeInh , amplitudeGlobal, circular, normalized};
			const element::ElementDimensions dimensions{ x_max, d_x };
			const std::shared_ptr<element::MexicanHatKernel> mexicanHatKernel(new element::MexicanHatKernel({ id, dimensions }, mhkp));
			simulation->addElement(mexicanHatKernel);
		}
		ImGui::PopID();
	}
}

