#include "user_interface/simulation_window.h"
#include "elements/neural_field.h"

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

	void SimulationWindow::renderContentPaneTitle() const
	{
		struct PaneInfo { const char* icon; const char* name; };
		static constexpr PaneInfo kInfo[] = {
			{ ICON_FA_PLUS,     "Add element"      },
			{ ICON_FA_TRASH,    "Remove element"   },
			{ ICON_FA_LINK,     "Set interactions" },
			{ ICON_FA_TERMINAL, "Log parameters"   },
			{ ICON_FA_DOWNLOAD, "Export data"      },
			{ ICON_FA_DATABASE, "Monitoring"       },
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
			{ ICON_FA_PLUS,     "Add Element"     },
			{ ICON_FA_TRASH,    "Remove Element"  },
			{ ICON_FA_LINK,     "Set Interaction" },
			{ ICON_FA_TERMINAL, "Log Parameters"  },
			{ ICON_FA_DOWNLOAD, "Export Data"     },
			{ ICON_FA_DATABASE, "Monitoring"      },
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

		ImGui::TextUnformatted("TYPE");
		ImGui::Spacing();

		static element::ElementLabel selected = element::ElementLabel::NEURAL_FIELD;

		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##type_select", element::ElementLabelToString.at(selected).c_str()))
		{
			for (const auto& [lbl, name] : element::ElementLabelToString)
			{
				if (lbl == element::ElementLabel::UNINITIALIZED) continue;
				const bool is_sel = (selected == lbl);
				if (ImGui::Selectable(name.c_str(), is_sel)) selected = lbl;
				if (is_sel) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

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
		ImGui::TextUnformatted("Identifier");
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##id", "enter identifier", id, IM_ARRAYSIZE(id));

		ImGui::Spacing();
		ImGui::TextUnformatted("PARAMETERS");
		ImGui::Spacing();

		static bool s_addRequested = false;
		const bool addRequested = s_addRequested;
		s_addRequested = false;

		const float child_h = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight() * 1.2f - ImGui::GetStyle().ItemSpacing.y * 2;
		if (ImGui::BeginChild("##params_scroll", ImVec2(0, child_h), true, ImGuiWindowFlags_NoSavedSettings))
		{
			switch (selected)
			{
				case element::ElementLabel::NEURAL_FIELD:              addElementNeuralField(id, addRequested);              break;
				case element::ElementLabel::GAUSS_STIMULUS:            addElementGaussStimulus(id, addRequested);            break;
				case element::ElementLabel::TIMED_GAUSS_STIMULUS:      addElementTimedGaussStimulus(id, addRequested);      break;
				case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:   addElementTimedGaussStimulus2D(id, addRequested);   break;
				case element::ElementLabel::GAUSS_KERNEL:              addElementGaussKernel(id, addRequested);              break;
				case element::ElementLabel::MEXICAN_HAT_KERNEL:        addElementMexicanHatKernel(id, addRequested);        break;
				case element::ElementLabel::OSCILLATORY_KERNEL:        addElementOscillatoryKernel(id, addRequested);       break;
				case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:   addElementAsymmetricGaussKernel(id, addRequested);  break;
				case element::ElementLabel::NORMAL_NOISE:              addElementNormalNoise(id, addRequested);              break;
				case element::ElementLabel::CORRELATED_NORMAL_NOISE:   addElementCorrelatedNormalNoise(id, addRequested);  break;
				case element::ElementLabel::FIELD_COUPLING:            addElementFieldCoupling(id, addRequested);           break;
				case element::ElementLabel::GAUSS_FIELD_COUPLING:      addElementGaussFieldCoupling(id, addRequested);     break;
				case element::ElementLabel::BOOST_STIMULUS:            addElementBoostStimulus(id, addRequested);           break;
				case element::ElementLabel::MEMORY_TRACE:              addElementMemoryTrace(id, addRequested);             break;
				case element::ElementLabel::NEURAL_FIELD_2D:           addElementNeuralField2D(id, addRequested);           break;
				case element::ElementLabel::GAUSS_STIMULUS_2D:         addElementGaussStimulus2D(id, addRequested);        break;
				case element::ElementLabel::GAUSS_KERNEL_2D:           addElementGaussKernel2D(id, addRequested);          break;
				case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:     addElementMexicanHatKernel2D(id, addRequested);    break;
				case element::ElementLabel::NORMAL_NOISE_2D:           addElementNormalNoise2D(id, addRequested);          break;
				case element::ElementLabel::OSCILLATORY_KERNEL_2D:     addElementOscillatoryKernel2D(id, addRequested);   break;
				case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D: addElementAsymmetricGaussKernel2D(id, addRequested); break;
				case element::ElementLabel::BOOST_STIMULUS_2D:         addElementBoostStimulus2D(id, addRequested);        break;
				case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D: addElementCorrelatedNormalNoise2D(id, addRequested); break;
				case element::ElementLabel::MEMORY_TRACE_2D:           addElementMemoryTrace2D(id, addRequested);          break;
				default: break;
			}
		}
		ImGui::EndChild();

		ImGui::Spacing();
		const float addBtnW = ImGui::GetContentRegionAvail().x;
		const float addBtnH = ImGui::GetFrameHeight() * 1.2f;
		if (ImGui::Button("+ Add element", ImVec2(addBtnW, addBtnH)))
			s_addRequested = true;

		ImGui::PopID();
	}

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",            &x_max,     0, 0);
		ImGui::InputDouble("Step",         &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Resting level",&resting,   0.0, 0.0, "%.2f");
		ImGui::InputDouble("Time scale",   &tau,       0.0, 0.0, "%.2f");
		ImGui::Combo("Activation fn.",     &actFnType, actFnNames, 3);
		ImGui::InputDouble("X shift",      &xShift,    0.0, 0.0, "%.2f");
		if (actFnType == element::SIGMOID)
			ImGui::InputDouble("Steepness", &steepness, 0.0, 0.0, "%.2f");
		else if (actFnType == element::ABSSIGMOID)
			ImGui::InputDouble("Beta",      &absBeta,   0.0, 0.0, "%.2f");
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",          &x_max,     0, 0);
		ImGui::InputDouble("Step",       &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",      &width,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude",  &amplitude, 0.0, 0.0, "%.2f");
		ImGui::InputDouble("Position",   &position,  0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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
		static double tEnd       = 10.0;
		static bool   circular   = true;
		static bool   normalized = false;

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",         &x_max,     0, 0);
		ImGui::InputDouble("Step",      &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",     &width,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.2f");
		ImGui::InputDouble("Position",  &position,  0.0, 0.0, "%.2f");
		ImGui::InputDouble("T start",   &tStart,    0.0, 0.0, "%.2f");
		ImGui::InputDouble("T end",     &tEnd,      0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",       &x_max,     0, 0);
		ImGui::InputInt("Y size",       &y_max,     0, 0);
		ImGui::InputDouble("X step",    &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",    &d_y,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",     &width,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.2f");
		ImGui::InputDouble("Position x",&pos_x,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("Position y",&pos_y,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("T start",   &tStart,    0.0, 0.0, "%.2f");
		ImGui::InputDouble("T end",     &tEnd,      0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",           &x_max,           0, 0);
		ImGui::InputDouble("Step",        &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",       &width,           0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude",   &amplitude,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Global amp",  &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",             &x_max,           0, 0);
		ImGui::InputDouble("Step",          &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width exc",     &widthExc,        0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude exc", &amplitudeExc,    0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width inh",     &widthInh,        0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude inh", &amplitudeInh,    0.0, 0.0, "%.2f");
		ImGui::InputDouble("Global amp",    &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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
		static bool   normalized      = false;

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",              &x_max,           0, 0);
		ImGui::InputDouble("Step",           &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude",      &amplitude,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Decay",          &decay,           0.0, 0.0, "%.4f");
		ImGui::InputDouble("Zero crossings", &zeroCrossings,   0.0, 0.0, "%.2f");
		ImGui::InputDouble("Global amp",     &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",          &x_max,           0, 0);
		ImGui::InputDouble("Step",       &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",      &width,           0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude",  &amplitude,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Global amp", &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::InputDouble("Time shift", &timeShift,       0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",         &x_max,     0, 0);
		ImGui::InputDouble("Step",      &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.4f");
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",         &x_max,     0, 0);
		ImGui::InputDouble("Step",      &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.4f");
		ImGui::InputDouble("Width",     &width,     0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular", &circular);
		ImGui::PopItemWidth();

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
		static LearningRule rule        = LearningRule::HEBB;
		static double      scalar       = 1.0;
		static double      learningRate = 0.01;

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Out size",    &x_max_out, 0, 0);
		ImGui::InputDouble("Out step", &d_x_out,   0.0, 0.0, "%.2f");
		ImGui::InputInt("In size",     &x_max_in,  0, 0);
		ImGui::InputDouble("In step",  &d_x_in,    0.0, 0.0, "%.2f");
		ImGui::PopItemWidth();

		ImGui::SetNextItemWidth(110.0f * ImGui::GetIO().FontGlobalScale);
		if (ImGui::BeginCombo("Rule", LearningRuleToString.at(rule).c_str()))
		{
			for (const auto& [lr, name] : LearningRuleToString)
			{
				const bool sel = (rule == lr);
				if (ImGui::Selectable(name.c_str(), sel)) rule = lr;
				if (sel) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputDouble("Scalar",        &scalar,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Learning rate", &learningRate, 0.0, 0.0, "%.4f");
		ImGui::PopItemWidth();

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
		static bool   normalized = true;
		static bool   circular   = false;

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Out size",    &x_max_out, 0, 0);
		ImGui::InputDouble("Out step", &d_x_out,   0.0, 0.0, "%.2f");
		ImGui::InputInt("In size",     &x_max_in,  0, 0);
		ImGui::InputDouble("In step",  &d_x_in,    0.0, 0.0, "%.2f");
		ImGui::Checkbox("Normalized",  &normalized);
		ImGui::Checkbox("Circular",    &circular);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",         &x_max,     0, 0);
		ImGui::InputDouble("Step",      &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.2f");
		ImGui::Checkbox("Active", &isActive);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("Size",          &x_max,     0, 0);
		ImGui::InputDouble("Step",       &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Tau build",  &tauBuild,  0.0, 0.0, "%.2f");
		ImGui::InputDouble("Tau decay",  &tauDecay,  0.0, 0.0, "%.2f");
		ImGui::InputDouble("Threshold",  &threshold, 0.0, 0.0, "%.2f");
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",           &x_max,       0, 0);
		ImGui::InputInt("Y size",           &y_max,       0, 0);
		ImGui::InputDouble("X step",        &d_x,         0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",        &d_y,         0.0, 0.0, "%.2f");
		ImGui::InputDouble("Tau",           &tau,         0.0, 0.0, "%.2f");
		ImGui::InputDouble("Resting level", &restingLevel,0.0, 0.0, "%.2f");
		ImGui::PopItemWidth();

		if (addRequested)
		{
			const element::NeuralField2DParameters nfp{ tau, restingLevel, element::SigmoidFunction(0.0, 10.0) };
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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",       &x_max,     0, 0);
		ImGui::InputInt("Y size",       &y_max,     0, 0);
		ImGui::InputDouble("X step",    &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",    &d_y,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",     &width,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.2f");
		ImGui::InputDouble("Position x",&pos_x,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("Position y",&pos_y,     0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",              &x_max,           0, 0);
		ImGui::InputInt("Y size",              &y_max,           0, 0);
		ImGui::InputDouble("X step",           &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",           &d_y,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",            &width,           0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude",        &amplitude,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude global", &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",              &x_max,           0, 0);
		ImGui::InputInt("Y size",              &y_max,           0, 0);
		ImGui::InputDouble("X step",           &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",           &d_y,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width exc",        &widthExc,        0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude exc",    &amplitudeExc,    0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width inh",        &widthInh,        0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude inh",    &amplitudeInh,    0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude global", &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",       &x_max,     0, 0);
		ImGui::InputInt("Y size",       &y_max,     0, 0);
		ImGui::InputDouble("X step",    &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",    &d_y,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.4f");
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",            &x_max,           0, 0);
		ImGui::InputInt("Y size",            &y_max,           0, 0);
		ImGui::InputDouble("X step",         &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",         &d_y,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude",      &amplitude,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Decay",          &decay,           0.0, 0.0, "%.4f");
		ImGui::InputDouble("Zero crossings", &zeroCrossings,   0.0, 0.0, "%.2f");
		ImGui::InputDouble("Global amp",     &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",          &x_max,           0, 0);
		ImGui::InputInt("Y size",          &y_max,           0, 0);
		ImGui::InputDouble("X step",       &d_x,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",       &d_y,             0.0, 0.0, "%.2f");
		ImGui::InputDouble("Width",        &width,           0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude",    &amplitude,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Global amp",   &amplitudeGlobal, 0.0, 0.0, "%.4f");
		ImGui::InputDouble("Time shift x", &timeShift_x,     0.0, 0.0, "%.2f");
		ImGui::InputDouble("Time shift y", &timeShift_y,     0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular",   &circular);
		ImGui::Checkbox("Normalized", &normalized);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",       &x_max,     0, 0);
		ImGui::InputInt("Y size",       &y_max,     0, 0);
		ImGui::InputDouble("X step",    &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",    &d_y,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.2f");
		ImGui::Checkbox("Active", &isActive);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",       &x_max,     0, 0);
		ImGui::InputInt("Y size",       &y_max,     0, 0);
		ImGui::InputDouble("X step",    &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",    &d_y,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Amplitude", &amplitude, 0.0, 0.0, "%.4f");
		ImGui::InputDouble("Width",     &width,     0.0, 0.0, "%.2f");
		ImGui::Checkbox("Circular", &circular);
		ImGui::PopItemWidth();

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

		ImGui::PushItemWidth(80.0f * ImGui::GetIO().FontGlobalScale);
		ImGui::InputInt("X size",        &x_max,     0, 0);
		ImGui::InputInt("Y size",        &y_max,     0, 0);
		ImGui::InputDouble("X step",     &d_x,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Y step",     &d_y,       0.0, 0.0, "%.2f");
		ImGui::InputDouble("Tau build",  &tauBuild,  0.0, 0.0, "%.2f");
		ImGui::InputDouble("Tau decay",  &tauDecay,  0.0, 0.0, "%.2f");
		ImGui::InputDouble("Threshold",  &threshold, 0.0, 0.0, "%.2f");
		ImGui::PopItemWidth();

		if (addRequested)
		{
			const element::MemoryTrace2DParameters mtp{ tauBuild, tauDecay, threshold };
			const element::ElementCommonParameters common{ std::string(id), element::ElementDimensions{ x_max, y_max, d_x, d_y } };
			simulation->addElement(std::make_shared<element::MemoryTrace2D>(common, mtp));
		}
	}


	void SimulationWindow::renderRemoveElementCard() const
	{
		ImGui::PushID("remove_element_inline");

		static std::string selectedId;

		ImGui::TextUnformatted("REMOVE ELEMENT");
		ImGui::Spacing();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Remove");
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

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

		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("from simulation");
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		const float line_h = ImGui::GetFrameHeight();
		const ImVec2 iconSz(line_h + 10.0f, line_h);

		ImGui::PushFont(g_MediumIconsFont);
		const bool clicked = ImGui::Button(ICON_FA_TRASH, iconSz);
		ImGui::PopFont();

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

		static std::string selectedTarget;
		static std::string selectedSource;

		ImGui::TextUnformatted("SET INTERACTION");
		ImGui::Spacing();

		ImGui::Columns(2, nullptr, false);

		const float leftW = 220.0f * ImGui::GetIO().FontGlobalScale;
		ImGui::SetColumnWidth(0, leftW);

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

		ImGui::NextColumn();
		ImGui::TextUnformatted("Connected elements");
		ImGui::SameLine();
		widgets::renderHelpMarker("You can disconnect elements by pressing the 'unlink' buttons.");

		const float listH = 120.0f * ImGui::GetIO().FontGlobalScale;
		ImGui::BeginChild("##connections_scroll", ImVec2(0, listH), true, ImGuiWindowFlags_NoSavedSettings);

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
						ImGui::TextUnformatted(conn->getUniqueName().c_str());
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

		const float btnW = ImGui::GetContentRegionAvail().x;
		const float btnH = ImGui::GetFrameHeight() * 1.2f;
		const bool canConnect = !selectedTarget.empty() && !selectedSource.empty();
		ImGui::BeginDisabled(!canConnect);
		const bool connectPressed = ImGui::Button("Connect", ImVec2(btnW, btnH));
		ImGui::EndDisabled();

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

		ImGui::PopID();
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
		ImGui::PushID("log_inline");

		static std::string selectedId;

		ImGui::TextUnformatted("LOG INFORMATION");
		ImGui::Spacing();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Log");
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		const char* preview = selectedId.empty() ? "element" : selectedId.c_str();
		ImGui::SetNextItemWidth(140.0f * ImGui::GetIO().FontGlobalScale);
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
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		const float h = ImGui::GetFrameHeight();
		ImGui::PushFont(g_MediumIconsFont);
		const bool logClicked = ImGui::Button(ICON_FA_TERMINAL, ImVec2(h + 10.0f, h));
		ImGui::PopFont();

		if (logClicked && !selectedId.empty())
			if (const auto e = simulation->getElement(selectedId)) e->print();

		ImGui::PopID();
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
