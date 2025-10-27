// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "user_interface/simulation_window.h"

#include "application/application.h"


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

	void SimulationWindow::render(const ImRect& bounds, bool* p_open) const
	{
		// Pin this window into the given rect
		ImGui::SetNextWindowPos(bounds.Min);
		ImGui::SetNextWindowSize(ImVec2(bounds.Max.x - bounds.Min.x, bounds.Max.y - bounds.Min.y));
		const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
							   | ImGuiWindowFlags_NoMove
							   | ImGuiWindowFlags_NoResize;

		if (ImGui::Begin("Simulation Control", p_open, flags))
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

	void SimulationWindow::renderAddElementCard() const
	{
		ImGui::PushID("add_element_card");
	    ImGui::Columns(2, nullptr, false);

	    // ---------------- Left column ----------------
	    ImGui::TextUnformatted("Select type");

	    static element::ElementLabel selected = element::ElementLabel::NEURAL_FIELD;
	    const char* current = element::ElementLabelToString.at(selected).c_str();
	    if (ImGui::BeginCombo("##element_type", current))
	    {
	        for (const auto& [fst, snd] : element::ElementLabelToString)
	        {
	            if (fst == element::ElementLabel::UNINITIALIZED) continue;
	            const bool is_selected = (selected == fst);
	            if (ImGui::Selectable(snd.c_str(), is_selected))
	                selected = fst;
	            if (is_selected) ImGui::SetItemDefaultFocus();
	        }
	        ImGui::EndCombo();
	    }

	    ImGui::Spacing();
	    const bool addRequested = ImGui::Button("Add element", ImVec2(-FLT_MIN, 0));
		//ImGui::PushFont(g_IconsFont);
		//const bool addRequested = ImGui::Button(ICON_FA_SQUARE_PLUS, ImVec2(-FLT_MIN, 0));
		//// ICON_FA_PLUS, ICON_FA_CIRCLE_PLUS, ICON_FA_SQUARE_PLUS
		//ImGui::PopFont();
	    // Right column
	    ImGui::NextColumn();
	    ImGui::TextUnformatted("Define parameters");

	    // Fixed-size, scrollable pane for parameters
	    // (tweak the height multiplier if you want more/less visible rows)
	    const float col_w   = ImGui::GetColumnWidth();
	    const float pad_w   = ImGui::GetStyle().ItemSpacing.x * 1.0f;
	    const float child_w = col_w - pad_w; // fill the column width
	    const float child_h = 220.0f * ImGui::GetIO().FontGlobalScale;

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
	    ImGui::PushID("remove_element_card");

	    // --- Row: "Remove [combo] from simulation"  -------------------------
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

	    // --- Trash icon button (right after the sentence) --------------------
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	    const float line_h  = ImGui::GetFrameHeight();
	    const ImVec2 iconSz(line_h, line_h);                 // square button

	    // Center the icon glyph inside the square by adjusting FramePadding.
	    const ImVec2 glyphSz = ImGui::CalcTextSize(ICON_FA_TRASH);
	    const auto pad = ImVec2(
	        std::max(0.0f, (iconSz.x - glyphSz.x) * 0.5f),
	        std::max(0.0f, (iconSz.y - glyphSz.y) * 0.5f)
	    );

	    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, pad);
	    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f,0.0f,0.0f,0.15f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f,0.0f,0.0f,0.25f));
	    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.90f,0.10f,0.10f,1.0f)); // red icon

	    ImGui::PushFont(g_IconsFont);
	    const bool clicked = ImGui::Button(ICON_FA_TRASH, iconSz);
	    ImGui::PopFont();

	    ImGui::PopStyleColor(4);
	    ImGui::PopStyleVar();

	    if (clicked && !selectedId.empty())
	    {
	        simulation->removeElement(selectedId);
	        simulation->init();
	    }

	    ImGui::PopID();
	}

	void SimulationWindow::renderSetInteractionCard() const
	{
		ImGui::PushID("set_interactions_card");

	    // Headline
	    ImGui::PushFont(g_BoldFont);
	    ImGui::TextUnformatted("Set interactions");
	    ImGui::PopFont();

	    // Two columns like the mock
	    ImGui::Columns(2, nullptr, false);

	    // ---------- Left column: target/source + connect button ----------
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

	    ImGui::Spacing(); ImGui::Spacing();

	    // Big connect button (plug icon)
	    const float btnSide = 80.0f * ImGui::GetIO().FontGlobalScale;
	    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f,0.75f,0.40f,0.18f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.75f,0.40f,0.28f));
	    ImGui::PushFont(g_IconsFont);
	    bool connectPressed = ImGui::Button(ICON_FA_PLUG, ImVec2(btnSide, btnSide));
	    ImGui::PopFont();
	    ImGui::PopStyleColor(3);

	    if (connectPressed && !selectedTarget.empty() && !selectedSource.empty())
	    {
	        auto target = simulation->getElement(selectedTarget);
	        auto input  = simulation->getElement(selectedSource);
	        if (target && input && target->getUniqueIdentifier() != input->getUniqueIdentifier())
	        {
	            target->addInput(input);   // same API used in your old set-interaction UI
	            simulation->init();        // re-init after wiring
	        }
	    }

	    // ---------- Right column: current connections list (scrollable) ----------
	    ImGui::NextColumn();
	    ImGui::TextUnformatted("Current connected elements");

	    const float listH = 210.0f * ImGui::GetIO().FontGlobalScale;
	    ImGui::BeginChild("##connections_scroll", ImVec2(0, listH), true,
	                      ImGuiWindowFlags_AlwaysVerticalScrollbar |
	                      ImGuiWindowFlags_NoSavedSettings);

	    if (!selectedTarget.empty())
	    {
	        auto target = simulation->getElement(selectedTarget);
	        if (target)
	        {
	            const auto& inputs = target->getInputs();   // vector<shared_ptr<Element>>
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
	                    const float rightX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	                    ImVec2 pos = ImGui::GetCursorScreenPos();
	                    const float h = ImGui::GetFrameHeight();
	                    pos.x = rightX - h;                   // square button at far right
	                    ImGui::SetCursorScreenPos(pos);

	                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f,0.0f,0.0f,0.15f));
	                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f,0.0f,0.0f,0.25f));
	                    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.90f,0.10f,0.10f,1.0f));
	                    ImGui::PushFont(g_IconsFont);
	                    const bool removeClicked = ImGui::Button(ICON_FA_LINK_SLASH, ImVec2(h, h));
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
	    ImGui::TextUnformatted("component");
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	    // Component combo (depends on selected element)
	    static std::string selectedComponent;
	    const char* compPreview = selectedComponent.empty() ? "component" : selectedComponent.c_str();
	    ImGui::SetNextItemWidth(160.0f * ImGui::GetIO().FontGlobalScale);

	    if (ImGui::BeginCombo("##export_comp_combo", compPreview))
	    {
	        if (!selectedElementId.empty())
	        {
	            auto elem = simulation->getElement(selectedElementId);
	            if (elem)
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

	    // Right-aligned download icon button
	    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	    const float h = ImGui::GetFrameHeight();
	    const ImVec2 iconSz(h, h);

	    // Place at absolute right edge of this window’s content region
	    const float right = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	    ImVec2 btnPos = ImGui::GetCursorScreenPos();
	    btnPos.x = right - iconSz.x;
	    ImGui::SetCursorScreenPos(btnPos);

	    // Center glyph inside the square button
	    const ImVec2 glyph = ImGui::CalcTextSize(ICON_FA_DOWNLOAD);
	    ImVec2 pad(ImMax(0.0f, (iconSz.x - glyph.x) * 0.5f),
	               ImMax(0.0f, (iconSz.y - glyph.y) * 0.5f));

	    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, pad);
	    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f,0.75f,0.40f,0.18f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.75f,0.40f,0.28f));
	    ImGui::PushFont(g_IconsFont);
	    bool exportClicked = ImGui::Button(ICON_FA_DOWNLOAD, iconSz);
	    ImGui::PopFont();
	    ImGui::PopStyleColor(3);
	    ImGui::PopStyleVar();

	    if (exportClicked && !selectedElementId.empty() && !selectedComponent.empty())
	        simulation->exportComponentToFile(selectedElementId, selectedComponent); // your existing API :contentReference[oaicite:2]{index=2}

	    ImGui::PopID();
	}

	void SimulationWindow::renderLogElementParametersCard() const
	{
		 ImGui::PushID("log_inline");

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

	    // Right-aligned terminal icon button
	    const float h = ImGui::GetFrameHeight();
	    const ImVec2 iconSz(h, h);

	    const float right = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	    ImVec2 btnPos = ImGui::GetCursorScreenPos();
	    btnPos.x = right - iconSz.x;
	    ImGui::SetCursorScreenPos(btnPos);

	    const ImVec2 glyph = ImGui::CalcTextSize(ICON_FA_TERMINAL);
	    ImVec2 pad(ImMax(0.0f, (iconSz.x - glyph.x) * 0.5f),
	               ImMax(0.0f, (iconSz.y - glyph.y) * 0.5f));

	    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, pad);
	    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
	    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f,0.75f,0.40f,0.18f));
	    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.75f,0.40f,0.28f));
	    ImGui::PushFont(g_IconsFont);
	    bool logClicked = ImGui::Button(ICON_FA_TERMINAL, iconSz);
	    ImGui::PopFont();
	    ImGui::PopStyleColor(3);
	    ImGui::PopStyleVar();

	    if (logClicked && !selectedId.empty())
	        if (auto e = simulation->getElement(selectedId)) e->print(); // your existing print() path :contentReference[oaicite:3]{index=3}

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

