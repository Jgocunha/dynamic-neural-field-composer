#include "user_interface/simulation_window.h"
#include "elements/neural_field.h"
#include <sstream>

extern ImFont* g_MonoMediumFont;
extern ImFont* g_MediumMediumFont;

namespace dnf_composer::user_interface
{
	int SimulationWindow::activePane = 0;

	SimulationWindow::SimulationWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{
	}

	void SimulationWindow::render()
	{
		ImGui::PushFont(g_BlackLargeFont);
		const bool open = ImGui::Begin("Simulation Control", nullptr,
			imgui_kit::getGlobalWindowFlags() | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopFont();
		if (open)
		{
			renderSidebarContents();
		}
		ImGui::End();
	}

	void SimulationWindow::renderSidebarContents() const
	{
		const float ui       = ImGui::GetIO().FontGlobalScale;
		const float sideW    = 58.0f * ui;
		const float gap      = 6.0f * ui;
		const float totalH   = ImGui::GetContentRegionAvail().y;
		const float contentW = ImGui::GetContentRegionAvail().x - sideW - gap;
		constexpr float rounding = 8.0f;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(6.0f * ui, 4.0f * ui));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
		if (ImGui::BeginChild("##sim_sidebar", {sideW, totalH}, true,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			drawIconStrip();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();

		ImGui::SameLine(0, gap);

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_TitleBg));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
		if (ImGui::BeginChild("##sim_content", {contentW, totalH}, true,
			ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			renderContentPaneTitle();
			switch (activePane)
			{
				case 0: renderAddElementCard();             break;
				case 1: renderRemoveElementCard();          break;
				case 2: renderSetInteractionCard();         break;
				case 3: renderLogElementParametersCard();   break;
				case 4: renderExportElementComponentCard(); break;
				case 5: renderMonitoringCard();             break;
				default: break;
			}
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	void SimulationWindow::renderContentPaneTitle()
	{
		ImGui::Spacing();

		struct PaneInfo { const char* icon; const char* name; };
		static constexpr PaneInfo kInfo[] = {
			{ ICON_FA_PLUS,     "Add elements"      },
			{ ICON_FA_TRASH,    "Remove elements"   },
			{ ICON_FA_LINK,     "Set interactions" },
			{ ICON_FA_TERMINAL, "Log parameters"   },
			{ ICON_FA_DOWNLOAD, "Export data"      },
			{ ICON_FA_BINOCULARS, "Monitoring"       },
		};

		ImGui::PushFont(g_MediumIconsFont);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
		ImGui::TextUnformatted(kInfo[activePane].icon);
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::SameLine(0, 8.0f);
		ImGui::PushFont(g_BlackMediumFont);
		ImGui::TextUnformatted(kInfo[activePane].name);
		ImGui::PopFont();

		ImGui::Separator();
		ImGui::Spacing();
	}

	void SimulationWindow::drawIconStrip()
	{
		struct PaneTab { const char* icon; const char* tooltip; };
		static constexpr PaneTab kPanes[] = {
			{ ICON_FA_PLUS,     "Add elements"     },
			{ ICON_FA_TRASH,    "Remove elements"  },
			{ ICON_FA_LINK,     "Set interactions" },
			{ ICON_FA_TERMINAL, "Log parameters"  },
			{ ICON_FA_DOWNLOAD, "Export data"     },
			{ ICON_FA_BINOCULARS, "Monitoring"      },
		};

		ImGui::SetCursorPos(ImVec2(0.0f, 16.0f));
		ImGui::BeginGroup();
		for (int i = 0; i < 6; ++i)
		{
			if (widgets::renderSidebarTab(kPanes[i].icon, "", activePane == i))
				activePane = i;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", kPanes[i].tooltip);
		}
		ImGui::EndGroup();
	}

	void SimulationWindow::renderAddElementCard() const
	{
		ImGui::PushID("add_element_section");

		// ── Dimensionality toggle ───────────────────────────────────────────
		static int dimensionality = 1;
		static element::ElementLabel selected1D = element::ElementLabel::NEURAL_FIELD;
		static element::ElementLabel selected2D = element::ElementLabel::NEURAL_FIELD_2D;
		element::ElementLabel& selected = (dimensionality == 1) ? selected1D : selected2D;

		{
			using L = element::ElementLabel;
			const float btnW    = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			const ImVec4 accent  = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
			const ImVec4 bg      = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			const ImVec4 bgHov   = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
			const ImVec4 textSel(1.f, 1.f, 1.f, 1.f);
			const ImVec4 textNorm = ImGui::GetStyleColorVec4(ImGuiCol_Text);

			auto dimBtn = [&](const char* label, int dim)
			{
				const bool on = (dimensionality == dim);
				ImGui::PushStyleColor(ImGuiCol_Button,        on ? accent : bg);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, on ? accent : bgHov);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  accent);
				ImGui::PushStyleColor(ImGuiCol_Text,          on ? textSel : textNorm);
				if (ImGui::Button(label, {btnW, 0})) dimensionality = dim;
				ImGui::PopStyleColor(4);
			};
			dimBtn("1D", 1);
			ImGui::SameLine();
			dimBtn("2D", 2);
		}

		ImGui::Spacing();

		// ── Filtered type combo ─────────────────────────────────────────────
		{
			using L = element::ElementLabel;
			static constexpr L k1D[] = {
				L::NEURAL_FIELD, L::GAUSS_STIMULUS, L::TIMED_GAUSS_STIMULUS,
				L::GAUSS_KERNEL, L::MEXICAN_HAT_KERNEL, L::OSCILLATORY_KERNEL,
				L::ASYMMETRIC_GAUSS_KERNEL, L::NORMAL_NOISE, L::CORRELATED_NORMAL_NOISE,
				L::FIELD_COUPLING, L::GAUSS_FIELD_COUPLING, L::BOOST_STIMULUS, L::MEMORY_TRACE
			};
			static constexpr L k2D[] = {
				L::NEURAL_FIELD_2D, L::GAUSS_STIMULUS_2D, L::TIMED_GAUSS_STIMULUS_2D,
				L::GAUSS_KERNEL_2D, L::MEXICAN_HAT_KERNEL_2D, L::OSCILLATORY_KERNEL_2D,
				L::ASYMMETRIC_GAUSS_KERNEL_2D, L::NORMAL_NOISE_2D, L::CORRELATED_NORMAL_NOISE_2D,
				L::BOOST_STIMULUS_2D, L::MEMORY_TRACE_2D
			};
			const L* pLabels = (dimensionality == 1) ? k1D : k2D;
			const int nLabels = (dimensionality == 1) ? static_cast<int>(std::size(k1D))
			                                          : static_cast<int>(std::size(k2D));

			ImGui::TextUnformatted("Type");
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##type_select", element::ElementLabelToString.at(selected).c_str()))
			{
				for (int i = 0; i < nLabels; ++i)
				{
					const L lbl = pLabels[i];
					const bool is_sel = (selected == lbl);
					if (ImGui::Selectable(element::ElementLabelToString.at(lbl).c_str(), is_sel))
						selected = lbl;
					if (is_sel) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ── Name input ──────────────────────────────────────────────────────
		static char id[CHAR_SIZE] = {};
		static element::ElementLabel prevSelected = element::ElementLabel::UNINITIALIZED;
		if (selected != prevSelected)
		{
			prevSelected = selected;
			int count = 0;
			for (const auto& e : simulation->getElements())
				if (e->getLabel() == selected) ++count;
			const std::string& typeName = element::ElementLabelToString.at(selected);
			std::snprintf(id, sizeof(id), "%s %d", typeName.c_str(), count + 1);
		}

		ImGui::Spacing();
		ImGui::TextUnformatted("Name");
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##id", "enter identifier", id, IM_ARRAYSIZE(id));
		ImGui::Spacing();

		// ── Parameters ──────────────────────────────────────────────────────
		static bool s_addRequested = false;
		const bool addRequested = s_addRequested;
		s_addRequested = false;

		switch (selected)
		{
			case element::ElementLabel::NEURAL_FIELD:               addElementNeuralField(id, addRequested);              break;
			case element::ElementLabel::GAUSS_STIMULUS:             addElementGaussStimulus(id, addRequested);            break;
			case element::ElementLabel::TIMED_GAUSS_STIMULUS:       addElementTimedGaussStimulus(id, addRequested);      break;
			case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:    addElementTimedGaussStimulus2D(id, addRequested);   break;
			case element::ElementLabel::GAUSS_KERNEL:               addElementGaussKernel(id, addRequested);              break;
			case element::ElementLabel::MEXICAN_HAT_KERNEL:         addElementMexicanHatKernel(id, addRequested);        break;
			case element::ElementLabel::OSCILLATORY_KERNEL:         addElementOscillatoryKernel(id, addRequested);       break;
			case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:    addElementAsymmetricGaussKernel(id, addRequested);  break;
			case element::ElementLabel::NORMAL_NOISE:               addElementNormalNoise(id, addRequested);              break;
			case element::ElementLabel::CORRELATED_NORMAL_NOISE:    addElementCorrelatedNormalNoise(id, addRequested);  break;
			case element::ElementLabel::FIELD_COUPLING:             addElementFieldCoupling(id, addRequested);           break;
			case element::ElementLabel::GAUSS_FIELD_COUPLING:       addElementGaussFieldCoupling(id, addRequested);     break;
			case element::ElementLabel::BOOST_STIMULUS:             addElementBoostStimulus(id, addRequested);           break;
			case element::ElementLabel::MEMORY_TRACE:               addElementMemoryTrace(id, addRequested);             break;
			case element::ElementLabel::NEURAL_FIELD_2D:            addElementNeuralField2D(id, addRequested);           break;
			case element::ElementLabel::GAUSS_STIMULUS_2D:          addElementGaussStimulus2D(id, addRequested);        break;
			case element::ElementLabel::GAUSS_KERNEL_2D:            addElementGaussKernel2D(id, addRequested);          break;
			case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:      addElementMexicanHatKernel2D(id, addRequested);    break;
			case element::ElementLabel::NORMAL_NOISE_2D:            addElementNormalNoise2D(id, addRequested);          break;
			case element::ElementLabel::OSCILLATORY_KERNEL_2D:      addElementOscillatoryKernel2D(id, addRequested);   break;
			case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D: addElementAsymmetricGaussKernel2D(id, addRequested); break;
			case element::ElementLabel::BOOST_STIMULUS_2D:          addElementBoostStimulus2D(id, addRequested);        break;
			case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D: addElementCorrelatedNormalNoise2D(id, addRequested); break;
			case element::ElementLabel::MEMORY_TRACE_2D:            addElementMemoryTrace2D(id, addRequested);          break;
			default: break;
		}

		ImGui::Spacing();
		{
			const float addBtnH = ImGui::GetFrameHeight() * 1.5f;
			const ImVec4 accent = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
			ImGui::PushStyleColor(ImGuiCol_Button,        accent);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accent.x * 0.9f, accent.y * 0.9f, accent.z * 0.9f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(accent.x * 0.8f, accent.y * 0.8f, accent.z * 0.8f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1, 1, 1, 1));
			if (ImGui::Button("     Add element", {-FLT_MIN, addBtnH}))
				s_addRequested = true;
			ImGui::PopStyleColor(4);

			const ImVec2 bMin = ImGui::GetItemRectMin();
			const ImVec2 bMax = ImGui::GetItemRectMax();
			ImGui::PushFont(g_MediumIconsFont);
			const ImVec2 iconSz = ImGui::CalcTextSize(ICON_FA_PLUS);
			const float  labelW = ImGui::CalcTextSize("     Add element").x;
			const float  iconX  = bMin.x + (bMax.x - bMin.x) * 0.5f - labelW * 0.5f;
			const float  iconY  = bMin.y + (bMax.y - bMin.y - iconSz.y) * 0.5f;
			ImGui::GetWindowDrawList()->AddText(g_MediumIconsFont, g_MediumIconsFont->LegacySize,
				{iconX, iconY}, IM_COL32(255, 255, 255, 255), ICON_FA_PLUS);
			ImGui::PopFont();
		}

		ImGui::PopID();
	}

	// ── helpers ─────────────────────────────────────────────────────────────
	// Two-column table: label left (normal font), input right (mono font).
	// Call beginParamTable / endParamTable around each group.

	static bool beginParamTable(const char* id)
	{
		return ImGui::BeginTable(id, 2, ImGuiTableFlags_None);
	}

	static void paramTableSetup()
	{
		ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthStretch, 1.0f);
		ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	}

	static void endParamTable()
	{
		ImGui::EndTable();
	}

	// Render one label-left / input-right row for int input
	static void paramRowInt(const char* label, const char* wid, int* v)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputInt(wid, v, 0, 0);
		ImGui::PopFont();
	}

	// Render one label-left / input-right row for double input
	static void paramRowDouble(const char* label, const char* wid, double* v, const char* fmt)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputDouble(wid, v, 0.0, 0.0, fmt);
		ImGui::PopFont();
	}

	// Render one label-left / checkbox-right row
	static void paramRowBool(const char* label, const char* wid, bool* v)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::TableSetColumnIndex(1);
		ImGui::Checkbox(wid, v);
	}

	// ── addElement functions ─────────────────────────────────────────────────

	void SimulationWindow::addElementNeuralField(const char* id, const bool addRequested) const
	{
		static int    x_max     = 100;
		static double d_x       = 1.0;
		static double resting   = -10.0;
		static double tau       = 25.0;
		static int    actFnType = element::SIGMOID;
		static double xShift    = 0.0;
		static double steepness = 5.0;
		static double absBeta   = 100.0;
		static const char* actFnNames[] = { "Sigmoid", "Heaviside", "AbsSigmoid" };

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##nf_dim")) {
			paramTableSetup();
			paramRowInt   ("Size",  "##nf_size", &x_max);
			paramRowDouble("Step",  "##nf_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Dynamics");
		if (beginParamTable("##nf_dyn")) {
			paramTableSetup();
			paramRowDouble("Resting level", "##nf_rest", &resting, "%.2f");
			paramRowDouble("Time scale",    "##nf_tau",  &tau,     "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Activation function");
		if (beginParamTable("##nf_act")) {
			paramTableSetup();
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Function");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN); ImGui::Combo("##nf_fn", &actFnType, actFnNames, 3);
			paramRowDouble("Shift", "##nf_xsh", &xShift, "%.2f");
			if (actFnType == element::SIGMOID)
				paramRowDouble("Steepness", "##nf_steep", &steepness, "%.2f");
			else if (actFnType == element::ABSSIGMOID)
				paramRowDouble("Beta", "##nf_beta", &absBeta, "%.2f");
			endParamTable();
		}

		if (addRequested)
		{
			std::unique_ptr<element::ActivationFunction> af;
			if (actFnType == element::SIGMOID)
				af = std::make_unique<element::SigmoidFunction>(xShift, steepness);
			else if (actFnType == element::HEAVISIDE)
				af = std::make_unique<element::HeavisideFunction>(xShift);
			else
				af = std::make_unique<element::AbsSigmoidFunction>(xShift, absBeta);
			const element::NeuralFieldParameters nfp{ tau, resting, *af };
			const element::ElementCommonParameters common{ element::ElementIdentifiers{id}, element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::NeuralField>(common, nfp));
		}
	}

	void SimulationWindow::addElementGaussStimulus(char* id, bool addRequested) const
	{
		static int    x_max      = 100;
		static double d_x        = 1.0;
		static double width      = 5.0;
		static double amplitude  = 15.0;
		static double position   = 50.0;
		static bool   circular   = true;
		static bool   normalized = false;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##gs_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##gs_size", &x_max);
			paramRowDouble("Step", "##gs_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Shape");
		if (beginParamTable("##gs_shp")) {
			paramTableSetup();
			paramRowDouble("Width",     "##gs_w",   &width,     "%.2f");
			paramRowDouble("Amplitude", "##gs_amp", &amplitude, "%.2f");
			paramRowDouble("Position",  "##gs_pos", &position,  "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##gs_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##gs_circ", &circular);
			paramRowBool("Normalized", "##gs_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::GaussStimulusParameters gsp{ width, amplitude, position, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::GaussStimulus>(common, gsp));
		}
	}

	void SimulationWindow::addElementTimedGaussStimulus(char* id, bool addRequested) const
	{
		static int    x_max      = 100;
		static double d_x        = 1.0;
		static double width      = 5.0;
		static double amplitude  = 15.0;
		static double position   = 50.0;
		static double tStart     = 0.0;
		static double tEnd       = 500.0;
		static bool   circular   = true;
		static bool   normalized = false;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##tgs_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##tgs_size", &x_max);
			paramRowDouble("Step", "##tgs_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Shape");
		if (beginParamTable("##tgs_shp")) {
			paramTableSetup();
			paramRowDouble("Width",     "##tgs_w",   &width,     "%.2f");
			paramRowDouble("Amplitude", "##tgs_amp", &amplitude, "%.2f");
			paramRowDouble("Position",  "##tgs_pos", &position,  "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Timing");
		if (beginParamTable("##tgs_tim")) {
			paramTableSetup();
			paramRowDouble("t start", "##tgs_ts", &tStart, "%.2f");
			paramRowDouble("t end",   "##tgs_te", &tEnd,   "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##tgs_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##tgs_circ", &circular);
			paramRowBool("Normalized", "##tgs_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			element::TimedGaussStimulusParameters tgsp{ width, amplitude, position, {{tStart, tEnd}}, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::TimedGaussStimulus>(common, tgsp));
		}
	}

	void SimulationWindow::addElementTimedGaussStimulus2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double width = 5.0, amplitude = 15.0;
		static double pos_x = 25.0, pos_y = 25.0;
		static double tStart = 0.0, tEnd = 10.0;
		static bool   circular = true, normalized = false;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##tgs2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##tgs2_xmax", &x_max);
			paramRowInt   ("y size", "##tgs2_ymax", &y_max);
			paramRowDouble("x step", "##tgs2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##tgs2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Shape");
		if (beginParamTable("##tgs2_shp")) {
			paramTableSetup();
			paramRowDouble("Width",      "##tgs2_w",    &width,     "%.2f");
			paramRowDouble("Amplitude",  "##tgs2_amp",  &amplitude, "%.2f");
			paramRowDouble("Position x", "##tgs2_posx", &pos_x,     "%.2f");
			paramRowDouble("Position y", "##tgs2_posy", &pos_y,     "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Timing");
		if (beginParamTable("##tgs2_tim")) {
			paramTableSetup();
			paramRowDouble("t start", "##tgs2_ts", &tStart, "%.2f");
			paramRowDouble("t end",   "##tgs2_te", &tEnd,   "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##tgs2_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##tgs2_circ", &circular);
			paramRowBool("Normalized", "##tgs2_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			element::TimedGaussStimulus2DParameters tgsp{ width, amplitude, pos_x, pos_y,
				{{tStart, tEnd}}, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::TimedGaussStimulus2D>(common, tgsp));
		}
	}

	void SimulationWindow::addElementGaussKernel(char* id, bool addRequested) const
	{
		static int    x_max           = 100;
		static double d_x             = 1.0;
		static double width           = 3.0;
		static double amplitude       = 3.0;
		static double amplitudeGlobal = -0.01;
		static bool   circular        = true;
		static bool   normalized      = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##gk_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##gk_size", &x_max);
			paramRowDouble("Step", "##gk_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Kernel");
		if (beginParamTable("##gk_ker")) {
			paramTableSetup();
			paramRowDouble("Width",      "##gk_w",    &width,           "%.2f");
			paramRowDouble("Amplitude",  "##gk_amp",  &amplitude,       "%.2f");
			paramRowDouble("Global amp", "##gk_ampg", &amplitudeGlobal, "%.4f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##gk_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##gk_circ", &circular);
			paramRowBool("Normalized", "##gk_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::GaussKernelParameters gkp{ width, amplitude, amplitudeGlobal, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::GaussKernel>(common, gkp));
		}
	}

	void SimulationWindow::addElementMexicanHatKernel(char* id, bool addRequested) const
	{
		static int    x_max           = 100;
		static double d_x             = 1.0;
		static double widthExc        = 2.5;
		static double amplitudeExc    = 11.0;
		static double widthInh        = 5.0;
		static double amplitudeInh    = 15.0;
		static double amplitudeGlobal = -0.1;
		static bool   circular        = true;
		static bool   normalized      = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##mhk_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##mhk_size", &x_max);
			paramRowDouble("Step", "##mhk_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Excitatory");
		if (beginParamTable("##mhk_exc")) {
			paramTableSetup();
			paramRowDouble("Width",     "##mhk_we",   &widthExc,     "%.2f");
			paramRowDouble("Amplitude", "##mhk_ampe", &amplitudeExc, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Inhibitory");
		if (beginParamTable("##mhk_inh")) {
			paramTableSetup();
			paramRowDouble("Width",     "##mhk_wi",   &widthInh,     "%.2f");
			paramRowDouble("Amplitude", "##mhk_ampi", &amplitudeInh, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Global");
		if (beginParamTable("##mhk_glo")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##mhk_ampg", &amplitudeGlobal, "%.4f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##mhk_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##mhk_circ", &circular);
			paramRowBool("Normalized", "##mhk_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::MexicanHatKernelParameters mhkp{ widthExc, amplitudeExc, widthInh, amplitudeInh, amplitudeGlobal, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::MexicanHatKernel>(common, mhkp));
		}
	}

	void SimulationWindow::addElementOscillatoryKernel(char* id, bool addRequested) const
	{
		static int    x_max           = 100;
		static double d_x             = 1.0;
		static double amplitude       = 1.0;
		static double decay           = 0.08;
		static double zeroCrossings   = 0.3;
		static double amplitudeGlobal = -0.01;
		static bool   circular        = true;
		static bool   normalized      = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##ok_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##ok_size", &x_max);
			paramRowDouble("Step", "##ok_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Kernel");
		if (beginParamTable("##ok_ker")) {
			paramTableSetup();
			paramRowDouble("Amplitude",      "##ok_amp",  &amplitude,       "%.2f");
			paramRowDouble("Decay",          "##ok_dec",  &decay,           "%.4f");
			paramRowDouble("Zero crossings", "##ok_zc",   &zeroCrossings,   "%.2f");
			paramRowDouble("Global amp",     "##ok_ampg", &amplitudeGlobal, "%.4f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##ok_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##ok_circ", &circular);
			paramRowBool("Normalized", "##ok_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::OscillatoryKernelParameters okp{ amplitude, decay, zeroCrossings, amplitudeGlobal, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::OscillatoryKernel>(common, okp));
		}
	}

	void SimulationWindow::addElementAsymmetricGaussKernel(char* id, bool addRequested) const
	{
		static int    x_max           = 100;
		static double d_x             = 1.0;
		static double width           = 3.0;
		static double amplitude       = 3.0;
		static double amplitudeGlobal = 0.0;
		static double timeShift       = 0.0;
		static bool   circular        = true;
		static bool   normalized      = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##agk_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##agk_size", &x_max);
			paramRowDouble("Step", "##agk_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Kernel");
		if (beginParamTable("##agk_ker")) {
			paramTableSetup();
			paramRowDouble("Width",      "##agk_w",    &width,           "%.2f");
			paramRowDouble("Amplitude",  "##agk_amp",  &amplitude,       "%.2f");
			paramRowDouble("Global amp", "##agk_ampg", &amplitudeGlobal, "%.4f");
			paramRowDouble("Time shift", "##agk_tsh",  &timeShift,       "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##agk_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##agk_circ", &circular);
			paramRowBool("Normalized", "##agk_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::AsymmetricGaussKernelParameters agkp{ width, amplitude, amplitudeGlobal, timeShift, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::AsymmetricGaussKernel>(common, agkp));
		}
	}

	void SimulationWindow::addElementNormalNoise(char* id, bool addRequested) const
	{
		static int    x_max     = 100;
		static double d_x       = 1.0;
		static double amplitude = 0.2;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##nn_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##nn_size", &x_max);
			paramRowDouble("Step", "##nn_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Noise");
		if (beginParamTable("##nn_noi")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##nn_amp", &amplitude, "%.4f");
			endParamTable();
		}

		if (addRequested)
		{
			const element::NormalNoiseParameters nnp{ amplitude };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::NormalNoise>(common, nnp));
		}
	}

	void SimulationWindow::addElementCorrelatedNormalNoise(char* id, bool addRequested) const
	{
		static int    x_max     = 100;
		static double d_x       = 1.0;
		static double amplitude = 0.05;
		static double width     = 2.0;
		static bool   circular  = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##cnn_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##cnn_size", &x_max);
			paramRowDouble("Step", "##cnn_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Noise");
		if (beginParamTable("##cnn_noi")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##cnn_amp", &amplitude, "%.4f");
			paramRowDouble("Width",     "##cnn_w",   &width,     "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##cnn_opt")) {
			paramTableSetup();
			paramRowBool("Circular", "##cnn_circ", &circular);
			endParamTable();
		}

		if (addRequested)
		{
			const element::CorrelatedNormalNoiseParameters cnnp{ amplitude, width, circular };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::CorrelatedNormalNoise>(common, cnnp));
		}
	}

	void SimulationWindow::addElementFieldCoupling(char* id, bool addRequested) const
	{
		static int         x_max_out    = 100;
		static double      d_x_out      = 1.0;
		static int         x_max_in     = 100;
		static double      d_x_in       = 1.0;
		static auto rule        = LearningRule::HEBB;
		static double      scalar       = 1.0;
		static double      learningRate = 0.01;

		ImGui::SeparatorText("Output dimensions");
		if (beginParamTable("##fc_odim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##fc_osize", &x_max_out);
			paramRowDouble("Step", "##fc_ostep", &d_x_out, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Input dimensions");
		if (beginParamTable("##fc_idim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##fc_isize", &x_max_in);
			paramRowDouble("Step", "##fc_istep", &d_x_in, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Learning");
		if (beginParamTable("##fc_learn")) {
			paramTableSetup();
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Rule");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##fc_rule", LearningRuleToString.at(rule).c_str()))
			{
				for (const auto& [lr, name] : LearningRuleToString)
				{
					const bool sel = (rule == lr);
					if (ImGui::Selectable(name.c_str(), sel)) rule = lr;
					if (sel) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			paramRowDouble("Scalar",        "##fc_scal", &scalar,       "%.2f");
			paramRowDouble("Learning rate", "##fc_lr",   &learningRate, "%.4f");
			endParamTable();
		}

		if (addRequested)
		{
			const element::ElementDimensions inDims{ x_max_in, d_x_in };
			const element::FieldCouplingParameters fcp{ inDims, rule, scalar, learningRate };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max_out, d_x_out } };
			simulation->addElement(std::make_shared<element::FieldCoupling>(common, fcp));
		}
	}

	void SimulationWindow::addElementGaussFieldCoupling(char* id, bool addRequested) const
	{
		static int    x_max_out  = 100;
		static double d_x_out    = 1.0;
		static int    x_max_in   = 100;
		static double d_x_in     = 1.0;
		static bool   normalized = false;
		static bool   circular   = true;

		ImGui::SeparatorText("Output dimensions");
		if (beginParamTable("##gfc_odim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##gfc_osize", &x_max_out);
			paramRowDouble("Step", "##gfc_ostep", &d_x_out, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Input dimensions");
		if (beginParamTable("##gfc_idim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##gfc_isize", &x_max_in);
			paramRowDouble("Step", "##gfc_istep", &d_x_in, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##gfc_opt")) {
			paramTableSetup();
			paramRowBool("Normalized", "##gfc_norm", &normalized);
			paramRowBool("Circular",   "##gfc_circ", &circular);
			endParamTable();
		}

		if (addRequested)
		{
			const element::GaussFieldCouplingParameters gfcp{ element::ElementDimensions{ x_max_in, d_x_in }, normalized, circular };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max_out, d_x_out } };
			simulation->addElement(std::make_shared<element::GaussFieldCoupling>(common, gfcp));
		}
	}

	void SimulationWindow::addElementBoostStimulus(char* id, bool addRequested) const
	{
		static int    x_max     = 100;
		static double d_x       = 1.0;
		static double amplitude = 5.0;
		static bool   isActive  = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##bs_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##bs_size", &x_max);
			paramRowDouble("Step", "##bs_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Stimulus");
		if (beginParamTable("##bs_stim")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##bs_amp", &amplitude, "%.2f");
			paramRowBool  ("Active",    "##bs_act", &isActive);
			endParamTable();
		}

		if (addRequested)
		{
			const element::BoostStimulusParameters bsp{ amplitude, isActive };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::BoostStimulus>(common, bsp));
		}
	}

	void SimulationWindow::addElementMemoryTrace(char* id, bool addRequested) const
	{
		static int    x_max     = 100;
		static double d_x       = 1.0;
		static double tauBuild  = 100.0;
		static double tauDecay  = 1000.0;
		static double threshold = 0.5;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##mt_dim")) {
			paramTableSetup();
			paramRowInt   ("Size", "##mt_size", &x_max);
			paramRowDouble("Step", "##mt_step", &d_x, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Dynamics");
		if (beginParamTable("##mt_dyn")) {
			paramTableSetup();
			paramRowDouble("Tau build",  "##mt_tauB", &tauBuild,  "%.2f");
			paramRowDouble("Tau decay",  "##mt_tauD", &tauDecay,  "%.2f");
			paramRowDouble("Threshold",  "##mt_thr",  &threshold, "%.2f");
			endParamTable();
		}

		if (addRequested)
		{
			const element::MemoryTraceParameters mtp{ tauBuild, tauDecay, threshold };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, d_x } };
			simulation->addElement(std::make_shared<element::MemoryTrace>(common, mtp));
		}
	}

	void SimulationWindow::addElementNeuralField2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double tau = 25.0, restingLevel = -5.0;
		static int    actFnType = element::SIGMOID;
		static double xShift    = 0.0;
		static double steepness = 5.0;
		static double absBeta   = 100.0;
		static const char* actFnNames[] = { "Sigmoid", "Heaviside", "AbsSigmoid" };

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##nf2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##nf2_xmax", &x_max);
			paramRowInt   ("y size", "##nf2_ymax", &y_max);
			paramRowDouble("x step", "##nf2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##nf2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Dynamics");
		if (beginParamTable("##nf2_dyn")) {
			paramTableSetup();
			paramRowDouble("Time scale",    "##nf2_tau",  &tau,          "%.2f");
			paramRowDouble("Resting level", "##nf2_rest", &restingLevel, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Activation function");
		if (beginParamTable("##nf2_act")) {
			paramTableSetup();
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Function");
			ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-FLT_MIN); ImGui::Combo("##nf2_fn", &actFnType, actFnNames, 3);
			paramRowDouble("Shift", "##nf2_xsh", &xShift, "%.2f");
			if (actFnType == element::SIGMOID)
				paramRowDouble("Steepness", "##nf2_steep", &steepness, "%.2f");
			else if (actFnType == element::ABSSIGMOID)
				paramRowDouble("Beta", "##nf2_beta", &absBeta, "%.2f");
			endParamTable();
		}

		if (addRequested)
		{
			std::unique_ptr<element::ActivationFunction> af;
			if (actFnType == element::SIGMOID)
				af = std::make_unique<element::SigmoidFunction>(xShift, steepness);
			else if (actFnType == element::HEAVISIDE)
				af = std::make_unique<element::HeavisideFunction>(xShift);
			else
				af = std::make_unique<element::AbsSigmoidFunction>(xShift, absBeta);
			const element::NeuralField2DParameters nfp{ tau, restingLevel, *af };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::NeuralField2D>(common, nfp));
		}
	}

	void SimulationWindow::addElementGaussStimulus2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double width = 5.0, amplitude = 15.0;
		static double pos_x = 25.0, pos_y = 25.0;
		static bool   circular = true, normalized = false;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##gs2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##gs2_xmax", &x_max);
			paramRowInt   ("y size", "##gs2_ymax", &y_max);
			paramRowDouble("x step", "##gs2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##gs2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Shape");
		if (beginParamTable("##gs2_shp")) {
			paramTableSetup();
			paramRowDouble("Width",      "##gs2_w",    &width,     "%.2f");
			paramRowDouble("Amplitude",  "##gs2_amp",  &amplitude, "%.2f");
			paramRowDouble("Position x", "##gs2_posx", &pos_x,     "%.2f");
			paramRowDouble("Position y", "##gs2_posy", &pos_y,     "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##gs2_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##gs2_circ", &circular);
			paramRowBool("Normalized", "##gs2_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::GaussStimulusParameters2D gsp( width, amplitude, pos_x, pos_y, circular, normalized );
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::GaussStimulus2D>(common, gsp));
		}
	}

	void SimulationWindow::addElementGaussKernel2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double width = 3.0, amplitude = 3.0, amplitudeGlobal = -0.01;
		static bool   circular = true, normalized = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##gk2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##gk2_xmax", &x_max);
			paramRowInt   ("y size", "##gk2_ymax", &y_max);
			paramRowDouble("x step", "##gk2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##gk2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Kernel");
		if (beginParamTable("##gk2_ker")) {
			paramTableSetup();
			paramRowDouble("Width",      "##gk2_w",    &width,           "%.2f");
			paramRowDouble("Amplitude",  "##gk2_amp",  &amplitude,       "%.2f");
			paramRowDouble("Global amp", "##gk2_ampg", &amplitudeGlobal, "%.4f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##gk2_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##gk2_circ", &circular);
			paramRowBool("Normalized", "##gk2_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::GaussKernel2DParameters gkp( width, amplitude, amplitudeGlobal, circular, normalized );
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::GaussKernel2D>(common, gkp));
		}
	}

	void SimulationWindow::addElementMexicanHatKernel2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double widthExc = 2.5, amplitudeExc = 11.0;
		static double widthInh = 5.0, amplitudeInh = 15.0;
		static double amplitudeGlobal = -0.1;
		static bool   circular = true, normalized = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##mhk2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##mhk2_xmax", &x_max);
			paramRowInt   ("y size", "##mhk2_ymax", &y_max);
			paramRowDouble("x step", "##mhk2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##mhk2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Excitatory");
		if (beginParamTable("##mhk2_exc")) {
			paramTableSetup();
			paramRowDouble("Width",     "##mhk2_we",   &widthExc,     "%.2f");
			paramRowDouble("Amplitude", "##mhk2_ampe", &amplitudeExc, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Inhibitory");
		if (beginParamTable("##mhk2_inh")) {
			paramTableSetup();
			paramRowDouble("Width",     "##mhk2_wi",   &widthInh,     "%.2f");
			paramRowDouble("Amplitude", "##mhk2_ampi", &amplitudeInh, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Global");
		if (beginParamTable("##mhk2_glo")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##mhk2_ampg", &amplitudeGlobal, "%.4f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##mhk2_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##mhk2_circ", &circular);
			paramRowBool("Normalized", "##mhk2_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::MexicanHatKernel2DParameters mhkp( widthExc, amplitudeExc, widthInh, amplitudeInh, amplitudeGlobal, circular, normalized );
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::MexicanHatKernel2D>(common, mhkp));
		}
	}

	void SimulationWindow::addElementNormalNoise2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double amplitude = 0.2;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##nn2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##nn2_xmax", &x_max);
			paramRowInt   ("y size", "##nn2_ymax", &y_max);
			paramRowDouble("x step", "##nn2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##nn2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Noise");
		if (beginParamTable("##nn2_noi")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##nn2_amp", &amplitude, "%.4f");
			endParamTable();
		}

		if (addRequested)
		{
			const element::NormalNoise2DParameters nnp( amplitude );
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::NormalNoise2D>(common, nnp));
		}
	}

	void SimulationWindow::addElementOscillatoryKernel2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double amplitude       = 1.0;
		static double decay           = 0.08;
		static double zeroCrossings   = 0.3;
		static double amplitudeGlobal = -0.01;
		static bool   circular        = true;
		static bool   normalized      = false;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##ok2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##ok2_xmax", &x_max);
			paramRowInt   ("y size", "##ok2_ymax", &y_max);
			paramRowDouble("x step", "##ok2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##ok2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Kernel");
		if (beginParamTable("##ok2_ker")) {
			paramTableSetup();
			paramRowDouble("Amplitude",      "##ok2_amp",  &amplitude,       "%.2f");
			paramRowDouble("Decay",          "##ok2_dec",  &decay,           "%.4f");
			paramRowDouble("Zero crossings", "##ok2_zc",   &zeroCrossings,   "%.2f");
			paramRowDouble("Global amp",     "##ok2_ampg", &amplitudeGlobal, "%.4f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##ok2_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##ok2_circ", &circular);
			paramRowBool("Normalized", "##ok2_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::OscillatoryKernel2DParameters okp{ amplitude, decay, zeroCrossings, amplitudeGlobal, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::OscillatoryKernel2D>(common, okp));
		}
	}

	void SimulationWindow::addElementAsymmetricGaussKernel2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double width           = 3.0;
		static double amplitude       = 3.0;
		static double amplitudeGlobal = 0.0;
		static double timeShift_x     = 0.0;
		static double timeShift_y     = 0.0;
		static bool   circular        = true;
		static bool   normalized      = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##agk2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##agk2_xmax", &x_max);
			paramRowInt   ("y size", "##agk2_ymax", &y_max);
			paramRowDouble("x step", "##agk2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##agk2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Kernel");
		if (beginParamTable("##agk2_ker")) {
			paramTableSetup();
			paramRowDouble("Width",        "##agk2_w",    &width,           "%.2f");
			paramRowDouble("Amplitude",    "##agk2_amp",  &amplitude,       "%.2f");
			paramRowDouble("Global amp",   "##agk2_ampg", &amplitudeGlobal, "%.4f");
			paramRowDouble("Time shift x", "##agk2_tsx",  &timeShift_x,     "%.2f");
			paramRowDouble("Time shift y", "##agk2_tsy",  &timeShift_y,     "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##agk2_opt")) {
			paramTableSetup();
			paramRowBool("Circular",   "##agk2_circ", &circular);
			paramRowBool("Normalized", "##agk2_norm", &normalized);
			endParamTable();
		}

		if (addRequested)
		{
			const element::AsymmetricGaussKernel2DParameters agkp{ width, amplitude, amplitudeGlobal,
				timeShift_x, timeShift_y, circular, normalized };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::AsymmetricGaussKernel2D>(common, agkp));
		}
	}

	void SimulationWindow::addElementBoostStimulus2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double amplitude = 5.0;
		static bool   isActive  = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##bs2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##bs2_xmax", &x_max);
			paramRowInt   ("y size", "##bs2_ymax", &y_max);
			paramRowDouble("x step", "##bs2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##bs2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Stimulus");
		if (beginParamTable("##bs2_stim")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##bs2_amp", &amplitude, "%.2f");
			paramRowBool  ("Active",    "##bs2_act", &isActive);
			endParamTable();
		}

		if (addRequested)
		{
			const element::BoostStimulus2DParameters bsp{ amplitude, isActive };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::BoostStimulus2D>(common, bsp));
		}
	}

	void SimulationWindow::addElementCorrelatedNormalNoise2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double amplitude = 0.05;
		static double width     = 1.0;
		static bool   circular  = true;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##cnn2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##cnn2_xmax", &x_max);
			paramRowInt   ("y size", "##cnn2_ymax", &y_max);
			paramRowDouble("x step", "##cnn2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##cnn2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Noise");
		if (beginParamTable("##cnn2_noi")) {
			paramTableSetup();
			paramRowDouble("Amplitude", "##cnn2_amp", &amplitude, "%.4f");
			paramRowDouble("Width",     "##cnn2_w",   &width,     "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Options");
		if (beginParamTable("##cnn2_opt")) {
			paramTableSetup();
			paramRowBool("Circular", "##cnn2_circ", &circular);
			endParamTable();
		}

		if (addRequested)
		{
			const element::CorrelatedNormalNoise2DParameters cnnp{ amplitude, width, circular };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::CorrelatedNormalNoise2D>(common, cnnp));
		}
	}

	void SimulationWindow::addElementMemoryTrace2D(char* id, bool addRequested) const
	{
		static int    x_max = 50, y_max = 50;
		static double d_x = 1.0, d_y = 1.0;
		static double tauBuild  = 100.0;
		static double tauDecay  = 1000.0;
		static double threshold = 0.5;

		ImGui::SeparatorText("Dimensions");
		if (beginParamTable("##mt2_dim")) {
			paramTableSetup();
			paramRowInt   ("x size", "##mt2_xmax", &x_max);
			paramRowInt   ("y size", "##mt2_ymax", &y_max);
			paramRowDouble("x step", "##mt2_dx",   &d_x, "%.2f");
			paramRowDouble("y step", "##mt2_dy",   &d_y, "%.2f");
			endParamTable();
		}
		ImGui::SeparatorText("Dynamics");
		if (beginParamTable("##mt2_dyn")) {
			paramTableSetup();
			paramRowDouble("Tau build",  "##mt2_tauB", &tauBuild,  "%.2f");
			paramRowDouble("Tau decay",  "##mt2_tauD", &tauDecay,  "%.2f");
			paramRowDouble("Threshold",  "##mt2_thr",  &threshold, "%.2f");
			endParamTable();
		}

		if (addRequested)
		{
			const element::MemoryTrace2DParameters mtp{ tauBuild, tauDecay, threshold };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::MemoryTrace2D>(common, mtp));
		}
	}


	void SimulationWindow::renderRemoveElementCard() const
	{
		struct ElemCategory { const char* label; ImU32 color; };
		static auto getCategory = [](const element::ElementLabel lbl) -> ElemCategory {
			using L = element::ElementLabel;
			switch (lbl) {
				case L::NEURAL_FIELD:    case L::NEURAL_FIELD_2D:
					return {"Field",    IM_COL32(74,  144, 217, 255)};
				case L::GAUSS_STIMULUS:  case L::TIMED_GAUSS_STIMULUS:
				case L::GAUSS_STIMULUS_2D: case L::TIMED_GAUSS_STIMULUS_2D:
				case L::BOOST_STIMULUS:  case L::BOOST_STIMULUS_2D:
					return {"Stimulus", IM_COL32(31,  158, 126, 255)};
				case L::GAUSS_KERNEL:    case L::MEXICAN_HAT_KERNEL:
				case L::OSCILLATORY_KERNEL: case L::ASYMMETRIC_GAUSS_KERNEL:
				case L::GAUSS_KERNEL_2D: case L::MEXICAN_HAT_KERNEL_2D:
				case L::OSCILLATORY_KERNEL_2D: case L::ASYMMETRIC_GAUSS_KERNEL_2D:
					return {"Kernel",   IM_COL32(192, 57,  43,  255)};
				case L::NORMAL_NOISE:    case L::CORRELATED_NORMAL_NOISE:
				case L::NORMAL_NOISE_2D: case L::CORRELATED_NORMAL_NOISE_2D:
					return {"Noise",    IM_COL32(230, 126, 34,  255)};
				case L::FIELD_COUPLING:  case L::GAUSS_FIELD_COUPLING:
					return {"Coupling", IM_COL32(142, 68,  173, 255)};
				case L::MEMORY_TRACE:    case L::MEMORY_TRACE_2D:
					return {"Memory",   IM_COL32(127, 140, 141, 255)};
				default:
					return {"Unknown",  IM_COL32(150, 150, 150, 255)};
			}
		};

		static char searchBuf[128] = {};
		static std::string pendingRemove;

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##re_search", "Search...", searchBuf, sizeof(searchBuf));
		ImGui::Spacing();

		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::TextUnformatted("Pick element to remove");
		ImGui::PopStyleColor();
		ImGui::Spacing();

		std::string filterLower(searchBuf);
		std::ranges::transform(filterLower, filterLower.begin(), ::tolower);

		const float rowH   = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
		const float dotR   = 5.0f;
		const float trashW = ImGui::GetFrameHeight() + 10.0f;
		const float typeW  = 65.0f * ImGui::GetIO().FontGlobalScale;

		if (ImGui::BeginChild("##re_list", {0, 0}, false, ImGuiWindowFlags_NoSavedSettings))
		{
			for (const auto& e : simulation->getElements())
			{
				const std::string& name = e->getUniqueName();
				const auto cat = getCategory(e->getLabel());

				if (!filterLower.empty())
				{
					std::string nameLower(name);
					std::ranges::transform(nameLower, nameLower.begin(), ::tolower);
					std::string catLower(cat.label);
					std::transform(catLower.begin(), catLower.end(), catLower.begin(), ::tolower);
					if (nameLower.find(filterLower) == std::string::npos &&
						catLower.find(filterLower)  == std::string::npos)
						continue;
				}

				ImGui::PushID(name.c_str());

				const ImVec2 rowMin = ImGui::GetCursorScreenPos();
				const float  avail  = ImGui::GetContentRegionAvail().x;
				const float  selH   = rowH - ImGui::GetStyle().ItemSpacing.y;

				ImGui::Selectable("##row", false, ImGuiSelectableFlags_AllowOverlap, {avail, selH});

				ImDrawList* dl = ImGui::GetWindowDrawList();
				const float cy    = rowMin.y + selH * 0.5f;
				const float cx    = rowMin.x + 12.0f;
				const float textY = rowMin.y + (selH - ImGui::GetTextLineHeight()) * 0.5f;

				dl->AddCircleFilled({cx, cy}, dotR, cat.color);
				dl->AddText({cx + dotR + 8.0f, textY}, ImGui::GetColorU32(ImGuiCol_Text), name.c_str());
				dl->AddText({rowMin.x + avail - trashW - typeW - 4.0f, textY},
					ImGui::GetColorU32(ImGuiCol_TextDisabled), cat.label);

				ImGui::SetCursorScreenPos({rowMin.x + avail - trashW, rowMin.y});
				ImGui::PushFont(g_MediumIconsFont);
				ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.06f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.12f));
				if (ImGui::Button(ICON_FA_TRASH, {trashW, selH}))
					pendingRemove = name;
				ImGui::PopStyleColor(3);
				ImGui::PopFont();

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		if (!pendingRemove.empty())
		{
			simulation->removeElement(pendingRemove);
			simulation->init();
			pendingRemove.clear();
		}
	}

	void SimulationWindow::renderSetInteractionCard() const
	{
		static std::string selectedTarget;
		static std::string selectedSource;
		static std::string pendingRemoveTarget;
		static std::string pendingRemoveSource;
		static char connSearch[128] = {};

		auto elementCombo = [&](const char* wid, const char* hint, std::string& value)
		{
			const char* preview = value.empty() ? hint : value.c_str();
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo(wid, preview))
			{
				for (const auto& e : simulation->getElements())
				{
					const std::string& name = e->getUniqueName();
					const bool sel = (value == name);
					if (ImGui::Selectable(name.c_str(), sel)) value = name;
					if (sel) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		};

		ImGui::TextUnformatted("Target element");
		elementCombo("##si_target", "Target element", selectedTarget);
		ImGui::Spacing();
		ImGui::TextUnformatted("Source element");
		elementCombo("##si_source", "Source element", selectedSource);
		ImGui::Spacing();

		// Connect button — right after the source combo
		{
			const bool canConn = !selectedTarget.empty() && !selectedSource.empty();
			const float btnH   = ImGui::GetFrameHeight() * 1.5f;
			const ImVec4 accent = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
			ImGui::PushStyleColor(ImGuiCol_Button,        accent);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
				ImVec4(accent.x * 0.9f, accent.y * 0.9f, accent.z * 0.9f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,
				ImVec4(accent.x * 0.8f, accent.y * 0.8f, accent.z * 0.8f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1, 1, 1, 1));
			ImGui::BeginDisabled(!canConn);
			const bool pressed = ImGui::Button("     Connect", {-FLT_MIN, btnH});
			ImGui::EndDisabled();
			ImGui::PopStyleColor(4);

			{
				const ImVec2 bMin = ImGui::GetItemRectMin();
				const ImVec2 bMax = ImGui::GetItemRectMax();
				ImGui::PushFont(g_MediumIconsFont);
				const ImVec2 iconSz = ImGui::CalcTextSize(ICON_FA_LINK);
				const float  labelW = ImGui::CalcTextSize("     Connect").x;
				const float  iconX  = bMin.x + (bMax.x - bMin.x) * 0.5f - labelW * 0.5f;
				const float  iconY  = bMin.y + (bMax.y - bMin.y - iconSz.y) * 0.5f;
				const ImU32  col    = canConn ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, 100);
				ImGui::GetWindowDrawList()->AddText(g_MediumIconsFont, g_MediumIconsFont->LegacySize,
					{iconX, iconY}, col, ICON_FA_LINK);
				ImGui::PopFont();
			}

			if (pressed)
			{
				const auto target = simulation->getElement(selectedTarget);
				const auto input  = simulation->getElement(selectedSource);
				if (target && input && target->getUniqueIdentifier() != input->getUniqueIdentifier())
				{
					target->addInput(input);
					simulation->init();
				}
			}
		}

		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::TextUnformatted("Existing connections");
		ImGui::PopStyleColor();
		ImGui::Spacing();

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##si_conn_search", "Search...", connSearch, sizeof(connSearch));
		ImGui::Spacing();

		std::string filterLower(connSearch);
		std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

		const float unlinkW = ImGui::GetFrameHeight() + 6.0f;

		if (ImGui::BeginChild("##si_connections", {0, 0}, false, ImGuiWindowFlags_NoSavedSettings))
		{
			bool any = false;
			for (const auto& tgt : simulation->getElements())
			{
				for (const auto& [src, comp] : tgt->getInputsAndComponents())
				{
					std::string label = src->getUniqueName() + " \xe2\x86\x92 " + tgt->getUniqueName();
					if (comp != "output") label += " (" + comp + ")";

					if (!filterLower.empty())
					{
						std::string labelLower = label;
						std::transform(labelLower.begin(), labelLower.end(), labelLower.begin(), ::tolower);
						if (labelLower.find(filterLower) == std::string::npos) continue;
					}

					any = true;
					ImGui::PushID(label.c_str());

					const ImVec2 rowMin = ImGui::GetCursorScreenPos();
					const float  avail  = ImGui::GetContentRegionAvail().x;
					const float  selH   = ImGui::GetFrameHeight();

					ImGui::Selectable("##conn_row", false, ImGuiSelectableFlags_AllowOverlap,
						{avail, selH});
					const bool hov = ImGui::IsItemHovered();

					ImDrawList* dl = ImGui::GetWindowDrawList();
					const float textY = rowMin.y + (selH - ImGui::GetTextLineHeight()) * 0.5f;
					dl->AddText({rowMin.x + 6.0f, textY}, ImGui::GetColorU32(ImGuiCol_Text),
						label.c_str());

					if (hov)
					{
						ImGui::SetCursorScreenPos({rowMin.x + avail - unlinkW, rowMin.y});
						ImGui::PushFont(g_MediumIconsFont);
						ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.12f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 0.0f, 0.0f, 0.22f));
						ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
						if (ImGui::Button(ICON_FA_LINK_SLASH, {unlinkW, selH}))
						{
							pendingRemoveTarget = tgt->getUniqueName();
							pendingRemoveSource = src->getUniqueName();
						}
						ImGui::PopStyleColor(4);
						ImGui::PopFont();
					}

					ImGui::PopID();
				}
			}
			if (!any)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
				ImGui::TextUnformatted("No connections.");
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndChild();

		if (!pendingRemoveTarget.empty())
		{
			if (const auto tgt = simulation->getElement(pendingRemoveTarget))
			{
				tgt->removeInput(pendingRemoveSource);
				simulation->init();
			}
			pendingRemoveTarget.clear();
			pendingRemoveSource.clear();
		}
	}

	void SimulationWindow::renderExportElementComponentCard() const
	{
		ImGui::PushID("export_inline");

		static std::string selectedElementId;
		static std::string selectedComponent;

		ImGui::TextUnformatted("EXPORT DATA");
		ImGui::Spacing();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Export");
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		const char* elemPreview = selectedElementId.empty() ? "element" : selectedElementId.c_str();
		ImGui::SetNextItemWidth(140.0f * ImGui::GetIO().FontGlobalScale);
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

		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		const float h = ImGui::GetFrameHeight();
		ImGui::PushFont(g_MediumIconsFont);
		const bool exportClicked = ImGui::Button(ICON_FA_DOWNLOAD, ImVec2(h + 10.0f, h));
		ImGui::PopFont();

		if (exportClicked && !selectedElementId.empty() && !selectedComponent.empty())
			simulation->exportComponentToFile(selectedElementId, selectedComponent);

		ImGui::PopID();
	}

	void SimulationWindow::renderLogElementParametersCard() const
	{
		struct ElemCategory { const char* label; ImU32 color; };
		static auto getCat = [](const element::ElementLabel lbl) -> ElemCategory {
			using L = element::ElementLabel;
			switch (lbl) {
				case L::NEURAL_FIELD:    case L::NEURAL_FIELD_2D:
					return {"Field",    IM_COL32(74,  144, 217, 255)};
				case L::GAUSS_STIMULUS:  case L::TIMED_GAUSS_STIMULUS:
				case L::GAUSS_STIMULUS_2D: case L::TIMED_GAUSS_STIMULUS_2D:
				case L::BOOST_STIMULUS:  case L::BOOST_STIMULUS_2D:
					return {"Stimulus", IM_COL32(31,  158, 126, 255)};
				case L::GAUSS_KERNEL:    case L::MEXICAN_HAT_KERNEL:
				case L::OSCILLATORY_KERNEL: case L::ASYMMETRIC_GAUSS_KERNEL:
				case L::GAUSS_KERNEL_2D: case L::MEXICAN_HAT_KERNEL_2D:
				case L::OSCILLATORY_KERNEL_2D: case L::ASYMMETRIC_GAUSS_KERNEL_2D:
					return {"Kernel",   IM_COL32(192, 57,  43,  255)};
				case L::NORMAL_NOISE:    case L::CORRELATED_NORMAL_NOISE:
				case L::NORMAL_NOISE_2D: case L::CORRELATED_NORMAL_NOISE_2D:
					return {"Noise",    IM_COL32(230, 126, 34,  255)};
				case L::FIELD_COUPLING:  case L::GAUSS_FIELD_COUPLING:
					return {"Coupling", IM_COL32(142, 68,  173, 255)};
				case L::MEMORY_TRACE:    case L::MEMORY_TRACE_2D:
					return {"Memory",   IM_COL32(127, 140, 141, 255)};
				default:
					return {"Unknown",  IM_COL32(150, 150, 150, 255)};
			}
		};

		static std::string selectedId;
		static char        searchBuf[128] = {};

		// ── Search bar ───────────────────────────────────────────────────────
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##lp_search", "Search...", searchBuf, sizeof(searchBuf));
		ImGui::Spacing();

		// ── Element list (bounded, filtered) ─────────────────────────────────
		std::string filterLower(searchBuf);
		std::ranges::transform(filterLower, filterLower.begin(), ::tolower);

		const float rowH  = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
		const float dotR  = 5.0f;
		const float typeW = 65.0f * ImGui::GetIO().FontGlobalScale;

		int matchCount = 0;
		for (const auto& e : simulation->getElements())
		{
			if (filterLower.empty()) { ++matchCount; continue; }
			std::string nl(e->getUniqueName());
			std::ranges::transform(nl, nl.begin(), ::tolower);
			std::string cl(getCat(e->getLabel()).label);
			std::ranges::transform(cl, cl.begin(), ::tolower);
			if (nl.find(filterLower) != std::string::npos || cl.find(filterLower) != std::string::npos)
				++matchCount;
		}

		const float maxListH = rowH * 8.0f;
		const float listH    = std::min(static_cast<float>(std::max(matchCount, 1)) * rowH, maxListH);

		if (ImGui::BeginChild("##lp_list", {0, listH}, false, ImGuiWindowFlags_NoSavedSettings))
		{
			for (const auto& e : simulation->getElements())
			{
				const std::string& name = e->getUniqueName();
				const auto cat = getCat(e->getLabel());

				if (!filterLower.empty())
				{
					std::string nl(name);
					std::ranges::transform(nl, nl.begin(), ::tolower);
					std::string cl(cat.label);
					std::ranges::transform(cl, cl.begin(), ::tolower);
					if (nl.find(filterLower) == std::string::npos && cl.find(filterLower) == std::string::npos)
						continue;
				}

				ImGui::PushID(name.c_str());

				const ImVec2 rowMin  = ImGui::GetCursorScreenPos();
				const float  avail   = ImGui::GetContentRegionAvail().x;
				const float  selH    = rowH - ImGui::GetStyle().ItemSpacing.y;
				const bool   selected = (selectedId == name);

				if (ImGui::Selectable("##lp_row", selected, 0, {avail, selH}))
					selectedId = name;

				ImDrawList* dl    = ImGui::GetWindowDrawList();
				const float cy    = rowMin.y + selH * 0.5f;
				const float textY = rowMin.y + (selH - ImGui::GetTextLineHeight()) * 0.5f;

				dl->AddCircleFilled({rowMin.x + 12.0f, cy}, dotR, cat.color);
				dl->AddText({rowMin.x + 12.0f + dotR + 8.0f, textY},
					ImGui::GetColorU32(ImGuiCol_Text), name.c_str());
				dl->AddText({rowMin.x + avail - typeW, textY},
					ImGui::GetColorU32(ImGuiCol_TextDisabled), cat.label);

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		// ── "Parameters" header + copy button ────────────────────────────────
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		const std::string paramText = [&]() -> std::string {
			if (selectedId.empty()) return {};
			const auto elem = simulation->getElement(selectedId);
			return elem ? elem->toString() : std::string{};
		}();

		const float copyW = ImGui::GetFrameHeight() + 6.0f;
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::TextUnformatted("Parameters");
		ImGui::PopStyleColor();
		ImGui::SameLine(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - copyW);
		ImGui::PushFont(g_MediumIconsFont);
		ImGui::BeginDisabled(paramText.empty());
		if (ImGui::Button(ICON_FA_COPY, {copyW, ImGui::GetFrameHeight()}))
			ImGui::SetClipboardText(paramText.c_str());
		ImGui::EndDisabled();
		ImGui::PopFont();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Copy parameters to clipboard");
		ImGui::Spacing();

		// ── Parameter display panel ───────────────────────────────────────────
		// Bracket-aware parser: splits "key: value, key: [nested], ..." into pairs.
		// Values enclosed in [] or {} are marked isList=true; plain values are isList=false.
		struct LpParam { std::string key, value; bool isList; };
		auto parseBlock = [](const std::string& content) -> std::vector<LpParam>
		{
			std::vector<LpParam> result;
			size_t i = 0;
			const size_t n = content.size();
			while (i < n)
			{
				while (i < n && (content[i] == ' ' || content[i] == ',')) ++i;
				if (i >= n) break;

				// Key: read until ':' at depth 0 (parens/brackets count as depth)
				const size_t keyStart = i;
				int depth = 0;
				while (i < n) {
					if (content[i] == '[' || content[i] == '{' || content[i] == '(') ++depth;
					else if (content[i] == ']' || content[i] == '}' || content[i] == ')') {
						if (depth > 0) --depth; else break;
					}
					else if (content[i] == ':' && depth == 0) break;
					++i;
				}
				if (i >= n || content[i] != ':') break;

				std::string key = content.substr(keyStart, i - keyStart);
				while (!key.empty() && (key.front() == ' ' || key.back() == ' '))
					{ if (key.front() == ' ') key.erase(0, 1); if (!key.empty() && key.back() == ' ') key.pop_back(); }
				++i; // skip ':'
				while (i < n && content[i] == ' ') ++i;

				// Value: bracket-enclosed (strip brackets) or plain (read until depth-0 comma)
				std::string value;
				bool isList = false;
				if (i < n && (content[i] == '[' || content[i] == '{'))
				{
					const char open = content[i], close = (open == '[') ? ']' : '}';
					int d = 0; ++i;
					const size_t vs = i;
					while (i < n) {
						if (content[i] == open) ++d;
						else if (content[i] == close) { if (d == 0) break; --d; }
						++i;
					}
					value = content.substr(vs, i - vs);
					if (i < n) ++i;
					isList = true;
				}
				else
				{
					const size_t vs = i;
					int vd = 0;
					while (i < n) {
						if (content[i] == '(' || content[i] == '[' || content[i] == '{') ++vd;
						else if (content[i] == ')' || content[i] == ']' || content[i] == '}') {
							if (vd > 0) --vd; else break;
						}
						else if (content[i] == ',' && vd == 0) break;
						++i;
					}
					value = content.substr(vs, i - vs);
					while (!value.empty() && value.back() == ' ') value.pop_back();
				}
				if (!key.empty()) result.push_back({std::move(key), std::move(value), isList});
			}
			return result;
		};

		if (ImGui::BeginChild("##lp_params", {0, 0}, false, ImGuiWindowFlags_NoSavedSettings))
		{
			if (paramText.empty())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
				ImGui::TextUnformatted("Select an element to view its parameters.");
				ImGui::PopStyleColor();
			}
			else
			{
				const ImVec4 accent = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
				std::istringstream stream(paramText);
				std::string line;
				bool firstLine = true;

				while (std::getline(stream, line))
				{
					if (line.empty()) { ImGui::Spacing(); firstLine = false; continue; }

					if (firstLine)
					{
						// Title: element type name — black medium font, accent color
						ImGui::PushFont(g_BlackMediumFont);
						ImGui::TextUnformatted(line.c_str());
						ImGui::PopFont();
						ImGui::Separator();
						ImGui::Spacing();
						firstLine = false;
					}
					else
					{
						// Find the first '[' or '{' to split the section name from content
						const size_t brk = line.find_first_of("[{");
						if (brk == std::string::npos)
						{
							ImGui::PushFont(g_MonoMediumFont);
							ImGui::TextWrapped("%s", line.c_str());
							ImGui::PopFont();
							continue;
						}

						// Section name: text before the bracket, strip trailing ':' and spaces
						std::string section = line.substr(0, brk);
						while (!section.empty() && (section.back() == ':' || section.back() == ' '))
							section.pop_back();

						// Extract the inner content of the outer bracket
						const char outerOpen  = line[brk];
						const char outerClose = (outerOpen == '[') ? ']' : '}';
						int depth = 0;
						size_t innerStart = brk + 1, innerEnd = brk + 1;
						for (size_t j = brk; j < line.size(); ++j) {
							if (line[j] == outerOpen)  ++depth;
							else if (line[j] == outerClose) { --depth; if (depth == 0) { innerEnd = j; break; } }
						}
						const std::string inner = line.substr(innerStart, innerEnd - innerStart);

						// Subtitle: section name — black medium font, normal text color
						if (!section.empty())
						{
							ImGui::PushFont(g_LightMediumFont);
							ImGui::TextUnformatted(section.c_str());
							ImGui::PopFont();
						}

						const auto params = parseBlock(inner);
						for (const auto& p : params)
						{
							if (p.isList)
							{
								// Sub-subtitle: key — medium font, accent color
								ImGui::PushFont(g_LightMediumFont);
								ImGui::TextUnformatted(("  " + p.key).c_str());
								ImGui::PopFont();

								// Each sub-param on its own line in mono
								const auto subParams = parseBlock(p.value);
								for (const auto& sp : subParams)
								{
									ImGui::PushFont(g_MonoMediumFont);
									ImGui::TextWrapped("    %s: %s", sp.key.c_str(), sp.value.c_str());
									ImGui::PopFont();
								}
							}
							else
							{
								// Plain param: mono, indented
								ImGui::PushFont(g_MonoMediumFont);
								ImGui::TextWrapped("  %s: %s", p.key.c_str(), p.value.c_str());
								ImGui::PopFont();
							}
						}
						ImGui::Spacing();
					}
				}
			}
		}
		ImGui::EndChild();
	}

	void SimulationWindow::renderMonitoringCard() const
	{
		ImGui::PushID("monitoring_section");

		ImGui::TextUnformatted("MONITORING");
		ImGui::Spacing();

		bool anyNF = false;
		for (const auto& e : simulation->getElements())
		{
			if (e->getLabel() != element::ElementLabel::NEURAL_FIELD)
				continue;
			anyNF = true;

			const auto* nf = dynamic_cast<const element::NeuralField*>(e.get());
			if (!nf) continue;

			ImGui::PushFont(g_BlackMediumFont);
			ImGui::TextUnformatted(e->getUniqueName().c_str());
			ImGui::PopFont();
			ImGui::Separator();

			ImGui::Text("Stability: %s", nf->isStable() ? "Stable" : "Unstable");
			ImGui::Text("Lowest activation: %.2f",  nf->getLowestActivation());
			ImGui::Text("Highest activation: %.2f", nf->getHighestActivation());

			const auto bumps = nf->getBumps();
			ImGui::Text("Number of bumps: %d", static_cast<int>(bumps.size()));

			for (int i = 0; i < static_cast<int>(bumps.size()); ++i)
			{
				const auto& b = bumps[i];
				ImGui::Text("  Bump %d: pos %.2f  amp %.2f  width %.2f", i, b.centroid, b.amplitude, b.width);
				ImGui::Text("           vel %.2f  acc %.2f", b.velocity, b.acceleration);
			}
			ImGui::Spacing();
		}

		if (!anyNF)
			ImGui::TextDisabled("No neural fields in simulation.");

		ImGui::PopID();
	}
}
