// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "user_interface/element_window.h"

namespace dnf_composer::user_interface
{
	ElementWindow::ElementWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{
	}

	void ElementWindow::render()
	{
		ImGui::PushFont(g_BlackLargeFont);
		const bool open = ImGui::Begin("Element Control", nullptr, imgui_kit::getGlobalWindowFlags());
		ImGui::PopFont();
		if (open)
		{
			//renderModifyElementParameters();
			renderElementControlCard();
		}
		ImGui::End();
	}

	void ElementWindow::renderElementControlCard() const
	{
		constexpr ImGuiWindowFlags childFlags =
        ImGuiWindowFlags_AlwaysVerticalScrollbar |
        ImGuiWindowFlags_NoSavedSettings;

		widgets::renderHelpMarker("Left click and drag or double-click to change element parameter values.");
	    ImGui::BeginChild("##element_scroll", ImVec2(0, 0), false, childFlags);

	    // group elements by type
	    std::map<element::ElementLabel, std::vector<std::shared_ptr<element::Element>>> byType;
	    for (const auto& e : simulation->getElements())
	        byType[e->getLabel()].push_back(e);

	    const float innerW   = ImGui::GetContentRegionAvail().x;
		const float ui       = ImGui::GetIO().FontGlobalScale;
		const float dragW    = 130.0f * ui;
		const float panelPadX = 10.0f * ui; // left and right from beginElementPanel
		const float spacingX = ImGui::GetStyle().ItemSpacing.x;

		auto PanelHeightFor = [&](const std::shared_ptr<element::Element>& e) -> float
		{
			const float frameH   = ImGui::GetFrameHeight();
			const float spacingY = ImGui::GetStyle().ItemSpacing.y;
			const float rowH     = frameH + spacingY;
			const float panelPad = 2.0f * 8.0f * ui; // top and bottom padding from beginElementPanel

			auto h = [&](const int rows) {
				return rows * rowH - spacingY + panelPad;
			};

			switch (e->getLabel())
			{
				case element::ElementLabel::NORMAL_NOISE:           return h(1);
				case element::ElementLabel::NEURAL_FIELD:           return h(2);
				case element::ElementLabel::GAUSS_STIMULUS:         return h(4);
				case element::ElementLabel::GAUSS_KERNEL:           return h(4);
				case element::ElementLabel::FIELD_COUPLING:         return h(5);
				case element::ElementLabel::MEXICAN_HAT_KERNEL:     return h(6);
				case element::ElementLabel::OSCILLATORY_KERNEL:     return h(5);
				case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:return h(5);
				case element::ElementLabel::BOOST_STIMULUS:         return h(2);
				case element::ElementLabel::MEMORY_TRACE:           return h(3);
				case element::ElementLabel::NEURAL_FIELD_2D:        return h(2);
				case element::ElementLabel::GAUSS_STIMULUS_2D:      return h(5);
				case element::ElementLabel::GAUSS_KERNEL_2D:        return h(4);
				case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:  return h(6);
				case element::ElementLabel::NORMAL_NOISE_2D:         return h(1);
				case element::ElementLabel::OSCILLATORY_KERNEL_2D:  return h(5);
				case element::ElementLabel::GAUSS_FIELD_COUPLING:
				{
					const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(e);
					const int numCouplings = static_cast<int>(gfc->getParameters().couplings.size());
					return h(2 + 5 * numCouplings);
				}
				default: return h(4);
			}
		};

		const float maxNaturalW = 1.0f * panelPadX + dragW + spacingX + ImGui::CalcTextSize("circular + normalized").x;
		const float panelW = ImMin(maxNaturalW, innerW);

	    for (const auto& [label, elems] : byType)
	    {
	        ImGui::PushFont(g_BoldLargeFont);
	        ImGui::TextUnformatted(getElementTypeDisplayName(label).c_str());
	        ImGui::PopFont();
	        ImGui::Spacing();

	        for (const auto& e : elems)
	        {
	            // small title above each panel
	            ImGui::TextUnformatted(e->getUniqueName().c_str());

	            const ImVec4 tint = getColorForElementType(label);
	            const ImVec2 size(panelW, PanelHeightFor(e));

	            // draw a panel first (behind), then render the editor inside it
	            PanelScope scope = beginElementPanel(tint, size);
	            {
	                switchElementToModify(e);   // draws inputs at p.rect.Min + pad
	            }
	            endElementPanel(scope);

	            ImGui::Spacing();
	        }

	        ImGui::Spacing();
	    }

	    ImGui::EndChild();
	}

	void ElementWindow::renderModifyElementParameters() const
	{
		// Group elements by type
		std::map<element::ElementLabel, std::vector<std::shared_ptr<element::Element>>> elementsByType;

		for (const auto& element : simulation->getElements())
		{
			elementsByType[element->getLabel()].push_back(element);
		}

		// Render each group
		for (const auto& [label, elements] : elementsByType)
		{
			ImVec4 color = getColorForElementType(label);
			ImGui::PushStyleColor(ImGuiCol_Header, color);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.x * 1.1f, color.y * 1.1f, color.z * 1.1f, color.w));

			std::string headerName = getElementTypeDisplayName(label) + " (" + std::to_string(elements.size()) + ")";

			if (ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (const auto& element : elements)
				{
					switchElementToModify(element);
					ImGui::Separator();
				}
			}

			ImGui::PopStyleColor(2);
		}

	}

	void ElementWindow::switchElementToModify(const std::shared_ptr<element::Element>& element)
	{
		const std::string elementId = element->getUniqueName();
		const element::ElementLabel label = element->getLabel();

		// Set text color based on the element label
		//ImVec4 elementColor = getColorForElementType(label);
		//ImGui::PushStyleColor(ImGuiCol_Text, elementColor);
		//ImGui::SeparatorText((getIconForElementType(label) + " " + elementId).c_str());
		//ImGui::PopStyleColor();

		//ImGui::SeparatorText( ("Element " + elementId).c_str() );

		switch (label)
		{
		case element::ElementLabel::NEURAL_FIELD:
			modifyElementNeuralField(element);
			break;
		case element::ElementLabel::GAUSS_STIMULUS:
			modifyElementGaussStimulus(element);
			break;
		case element::ElementLabel::FIELD_COUPLING:
			modifyElementFieldCoupling(element);
			break;
		case element::ElementLabel::GAUSS_KERNEL:
			modifyElementGaussKernel(element);
			break;
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
			modifyElementMexicanHatKernel(element);
			break;
		case element::ElementLabel::NORMAL_NOISE:
			modifyElementNormalNoise(element);
			break;
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			modifyElementGaussFieldCoupling(element);
			break;
		case element::ElementLabel::OSCILLATORY_KERNEL:
			modifyElementOscillatoryKernel(element);
			break;
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
			modifyElementAsymmetricGaussKernel(element);
			break;
		case element::ElementLabel::BOOST_STIMULUS:
			modifyElementBoostStimulus(element);
			break;
		case element::ElementLabel::MEMORY_TRACE:
			modifyElementMemoryTrace(element);
			break;
		case element::ElementLabel::NEURAL_FIELD_2D:
			modifyElementNeuralField2D(element);
			break;
		case element::ElementLabel::GAUSS_STIMULUS_2D:
			modifyElementGaussStimulus2D(element);
			break;
		case element::ElementLabel::GAUSS_KERNEL_2D:
			modifyElementGaussKernel2D(element);
			break;
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
			modifyElementMexicanHatKernel2D(element);
			break;
		case element::ElementLabel::NORMAL_NOISE_2D:
			modifyElementNormalNoise2D(element);
			break;
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
			modifyElementOscillatoryKernel2D(element);
			break;
		case element::ElementLabel::UNINITIALIZED:
			break;
		default:
			log(tools::logger::LogLevel::ERROR, "There is a missing element in the "
			    "TreeNode in simulation window.");
			break;
		}
	}

	void ElementWindow::modifyElementNeuralField(const std::shared_ptr<element::Element>& element)
	{
			const float ui = ImGui::GetIO().FontGlobalScale;

			const auto neuralField = std::dynamic_pointer_cast<element::NeuralField>(element);
			element::NeuralFieldParameters nfp = neuralField->getParameters();

			auto restingLevel = static_cast<float>(nfp.startingRestingLevel);
			auto tau = static_cast<float>(nfp.tau);

			std::string label = "##" + element->getUniqueName() + "Resting level";
			ImGui::SetNextItemWidth(150.0f * ui);
			ImGui::DragFloat(label.c_str(), &restingLevel, 0.1f, -30.0f, 0.0f);
			ImGui::SameLine(); ImGui::Text("Resting level");

			label = "##" + element->getUniqueName() + "Tau";
			ImGui::SetNextItemWidth(150.0f * ui);
			ImGui::DragFloat(label.c_str(), &tau, 0.5f, 1.0f, 300.0f);
			ImGui::SameLine(); ImGui::Text("Tau");

		static constexpr double epsilon = 1e-6;
		if (std::abs(restingLevel - static_cast<float>(nfp.startingRestingLevel)) > epsilon)
		{
			nfp.startingRestingLevel = restingLevel;
			neuralField->setParameters(nfp);
		}

		if (std::abs(tau - static_cast<float>(nfp.tau)) > epsilon)
		{
			nfp.tau = tau;
			neuralField->setParameters(nfp);
		}
	}

	void ElementWindow::modifyElementGaussStimulus(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto stimulus = std::dynamic_pointer_cast<element::GaussStimulus>(element);
		element::GaussStimulusParameters gsp = stimulus->getParameters();

		auto amplitude = static_cast<float>(gsp.amplitude);
		auto width = static_cast<float>(gsp.width);
		auto position = static_cast<float>(gsp.position);
		bool circular = gsp.circular;
		bool normalized = gsp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, 0, 30);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Width";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &width, 0.01f, 0, 30);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Position";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &position, 0.1f,
			0.0f, static_cast<float>(stimulus->getElementCommonParameters().dimensionParameters.x_max));
		ImGui::SameLine(); ImGui::Text("Position");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");

		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::SameLine(); ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(gsp.amplitude)) > epsilon ||
			std::abs(width - static_cast<float>(gsp.width)) > epsilon ||
			std::abs(position - static_cast<float>(gsp.position)) > epsilon ||
			circular != gsp.circular ||
			normalized != gsp.normalized)
		{
			gsp.amplitude = amplitude;
			gsp.width = width;
			gsp.position = position;
			gsp.circular = circular;
			gsp.normalized = normalized;
			stimulus->setParameters(gsp);
		}
	}

	void ElementWindow::modifyElementFieldCoupling(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto fieldCoupling = std::dynamic_pointer_cast<element::FieldCoupling>(element);
		element::FieldCouplingParameters fcp = fieldCoupling->getParameters();

		auto scalar = static_cast<float>(fcp.scalar);
		auto learningRate = static_cast<float>(fcp.learningRate);
		bool activateLearning = fcp.isLearningActive;

		std::string label = "##" + element->getUniqueName() + "Learning rule";
		ImGui::SetNextItemWidth(150.0f * ui);
		if (ImGui::BeginCombo(label.c_str(), LearningRuleToString.at(fcp.learningRule).c_str()))
		{
			for (size_t i = 0; i < LearningRuleToString.size(); ++i)
			{
				const char* name = LearningRuleToString.at(static_cast<LearningRule>(i)).c_str();
				if (ImGui::Selectable(name, fcp.learningRule == static_cast<LearningRule>(i)))
				{
					fcp.learningRule = static_cast<LearningRule>(i);
					fieldCoupling->setParameters(fcp);
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine(); ImGui::Text("Learning rule");

		label = "##" + element->getUniqueName() + "Learning rate";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &learningRate, 0.01f, 0, 10);
		ImGui::SameLine(); ImGui::Text("Learning rate");

		label = "##" + element->getUniqueName() + "Scalar";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &scalar, 0.1f, -20, 20);
		ImGui::SameLine(); ImGui::Text("Scalar");

		label = "##" + element->getUniqueName() + "Activate learning";
		ImGui::Checkbox(label.c_str(), &activateLearning);
		ImGui::SameLine(); ImGui::Text("Activate learning");

		static constexpr double epsilon = 1e-6;
		if (std::abs(scalar - static_cast<float>(fcp.scalar)) > epsilon)
		{
			fcp.scalar = scalar;
			fieldCoupling->setParameters(fcp);
		}
		if (activateLearning != fcp.isLearningActive)
		{
			fcp.isLearningActive = activateLearning;
			fieldCoupling->setParameters(fcp);
		}
		if (std::abs(learningRate - static_cast<float>(fcp.learningRate)) > epsilon)
		{
			fcp.learningRate = learningRate;
			fieldCoupling->setParameters(fcp);
		}

		ImGui::PushID(element->getUniqueName().c_str()); // Use unique ID for scope

		if (ImGui::Button("Load"))
		{
			fieldCoupling->readWeights();
		}
		ImGui::SameLine();

		if (ImGui::Button("Save"))
		{
			fieldCoupling->writeWeights();
		}
		ImGui::SameLine();

		if (ImGui::Button("Clear"))
		{
			fieldCoupling->clearWeights();
		}

		ImGui::PopID(); // End unique ID scope
	}

	void ElementWindow::modifyElementGaussKernel(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto kernel = std::dynamic_pointer_cast<element::GaussKernel>(element);
		element::GaussKernelParameters gkp = kernel->getParameters();

		auto amplitude = static_cast<float>(gkp.amplitude);
		auto width = static_cast<float>(gkp.width);
		auto amplitudeGlobal = static_cast<float>(gkp.amplitudeGlobal);
		bool circular = gkp.circular;
		bool normalized = gkp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Width";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &width, 0.1f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Amp. global";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.1f, -10, 10);
		ImGui::SameLine(); ImGui::Text("Amp. global");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");

		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::SameLine(); ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(gkp.amplitude)) > epsilon ||
			std::abs(width - static_cast<float>(gkp.width)) > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(gkp.amplitudeGlobal)) > epsilon ||
			circular != gkp.circular ||
			normalized != gkp.normalized)
		{
			gkp.amplitude = amplitude;
			gkp.width = width;
			gkp.amplitudeGlobal = amplitudeGlobal;
			gkp.circular = circular;
			gkp.normalized = normalized;
			kernel->setParameters(gkp);
		}
	}

	void ElementWindow::modifyElementMexicanHatKernel(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto kernel = std::dynamic_pointer_cast<element::MexicanHatKernel>(element);
		element::MexicanHatKernelParameters mhkp = kernel->getParameters();

		auto amplitudeExc = static_cast<float>(mhkp.amplitudeExc);
		auto widthExc = static_cast<float>(mhkp.widthExc);
		auto amplitudeInh = static_cast<float>(mhkp.amplitudeInh);
		auto widthInh = static_cast<float>(mhkp.widthInh);
		auto amplitudeGlobal = static_cast<float>(mhkp.amplitudeGlobal);
		bool circular = mhkp.circular;
		bool normalized = mhkp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amp. exc.";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeExc, 0.1f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amp. exc.");

		label = "##" + element->getUniqueName() + "Width exc.";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &widthExc, 0.1f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width exc.");

		label = "##" + element->getUniqueName() + "Amp. inh.";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeInh, 0.1f, 0.0f, 100.0f);
		ImGui::SameLine(); ImGui::Text("Amp. inh.");

		label = "##" + element->getUniqueName() + "Width inh.";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &widthInh, 0.1f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width inh.");

		label = "##" + element->getUniqueName() + "Amp. global";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.01f, -10.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Amp. global");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");

		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::SameLine(); ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if(std::abs(amplitudeExc - static_cast<float>(mhkp.amplitudeExc)) > epsilon ||
			std::abs(widthExc - static_cast<float>(mhkp.widthExc)) > epsilon ||
			std::abs(amplitudeInh - static_cast<float>(mhkp.amplitudeInh)) > epsilon ||
			std::abs(widthInh - static_cast<float>(mhkp.widthInh)) > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(mhkp.amplitudeGlobal)) > epsilon ||
			circular != mhkp.circular ||
			normalized != mhkp.normalized)
		{
			mhkp.amplitudeExc = amplitudeExc;
			mhkp.widthExc = widthExc;
			mhkp.amplitudeInh = amplitudeInh;
			mhkp.widthInh = widthInh;
			mhkp.amplitudeGlobal = amplitudeGlobal;
			mhkp.circular = circular;
			mhkp.normalized = normalized;
			kernel->setParameters(mhkp);
		}
	}

	void ElementWindow::modifyElementNormalNoise(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto normalNoise = std::dynamic_pointer_cast<element::NormalNoise>(element);
		element::NormalNoiseParameters nnp = normalNoise->getParameters();

		auto amplitude = static_cast<float>(nnp.amplitude);

		const std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.01f, 0.0f, 5.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(nnp.amplitude)) > epsilon)
		{
			nnp.amplitude = amplitude;
			normalNoise->setParameters(nnp);
		}
	}

	void ElementWindow::modifyElementGaussFieldCoupling(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
		element::GaussFieldCouplingParameters gfcp = gfc->getParameters();
		const int size = gfc->getMaxSpatialDimension();
		const auto other_size = gfc->getInputFieldDimensions().x_max;

		bool normalized = gfcp.normalized;
		bool circular = gfcp.circular;

		std::string label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		std::string text = "Circular";
		ImGui::SameLine(); ImGui::Text(text.c_str());

		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::SameLine(); ImGui::Checkbox(label.c_str(), &normalized);
		text = "Normalized";
		ImGui::SameLine(); ImGui::Text(text.c_str());

		for (size_t couplingIndex = 0; couplingIndex < gfcp.couplings.size(); ++couplingIndex)
		{
			auto& coupling = gfcp.couplings[couplingIndex];

			auto x_i = static_cast<float>(coupling.x_i);
			auto x_j = static_cast<float>(coupling.x_j);
			auto amplitude = static_cast<float>(coupling.amplitude);
			auto width = static_cast<float>(coupling.width);

			ImGui::Text("from (%.1f) to (%.1f)", x_i, x_j);

			label = "##" + element->getUniqueName() + "x_i" + std::to_string(couplingIndex);
			ImGui::SetNextItemWidth(150.0f * ui);
			ImGui::DragFloat(label.c_str(), &x_i, 0.05f, 0.0f, static_cast<float>(other_size));
			text = "x_i " + std::to_string(couplingIndex);
			ImGui::SameLine(); ImGui::Text(text.c_str());

			label = "##" + element->getUniqueName() + "x_j" + std::to_string(couplingIndex);
			ImGui::SetNextItemWidth(150.0f * ui);
			ImGui::DragFloat(label.c_str(), &x_j, 0.05f, 0.0f, static_cast<float>(size));
			text = "x_j " + std::to_string(couplingIndex);
			ImGui::SameLine(); ImGui::Text(text.c_str());

			label = "##" + element->getUniqueName() + "Amplitude" + std::to_string(couplingIndex);
			ImGui::SetNextItemWidth(150.0f * ui);
			ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, 0.0f, 100.0f);
			text = "Amplitude " + std::to_string(couplingIndex);
			ImGui::SameLine(); ImGui::Text(text.c_str());

			label = "##" + element->getUniqueName() + "Width" + std::to_string(couplingIndex);
			ImGui::SetNextItemWidth(150.0f * ui);
			ImGui::DragFloat(label.c_str(), &width, 0.1f,1.0f, 30.0f);
			text = "Width " + std::to_string(couplingIndex);
			ImGui::SameLine(); ImGui::Text(text.c_str());

			static constexpr double epsilon = 1e-6;
			if (std::abs(x_i - static_cast<float>(coupling.x_i)) > epsilon ||
				std::abs(x_j - static_cast<float>(coupling.x_j)) > epsilon ||
				std::abs(amplitude - static_cast<float>(coupling.amplitude)) > epsilon ||
				std::abs(width - static_cast<float>(coupling.width)) > epsilon ||
				normalized != gfcp.normalized ||
				circular != gfcp.circular)
			{
				gfcp.normalized = normalized;
				gfcp.circular = circular;
				coupling.x_i = x_i;
				coupling.x_j = x_j;
				coupling.amplitude = amplitude;
				coupling.width = width;
				gfc->setParameters(gfcp);
			}
		}

		// Button to open the modal
		if (ImGui::Button("Add new coupling"))
		{
			ImGui::OpenPopup("Add Coupling Modal");
		}

		// Modal dialog for adding a coupling
		if (ImGui::BeginPopupModal("Add Coupling Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static float new_x_i = 0.0f, new_x_j = 0.0f, new_amplitude = 1.0f, new_width = 1.0f;

			ImGui::Text("Specify new coupling parameters:");
			ImGui::Separator();

			std::string label_new_params = "##New_x_i";
			ImGui::SetNextItemWidth(250);
			ImGui::SliderFloat(label_new_params.c_str(), &new_x_i, 0, static_cast<float>(other_size));
			ImGui::SameLine(); ImGui::Text("New x_i");

			label_new_params = "##New_x_j";
			ImGui::SetNextItemWidth(250);
			ImGui::SliderFloat(label_new_params.c_str(), &new_x_j, 0, static_cast<float>(size));
			ImGui::SameLine(); ImGui::Text("New x_j");

			label_new_params = "##NewAmplitude";
			ImGui::SetNextItemWidth(250);
			ImGui::SliderFloat(label_new_params.c_str(), &new_amplitude, 0, 100);
			ImGui::SameLine(); ImGui::Text("New Amplitude");

			label_new_params = "##NewWidth";
			ImGui::SetNextItemWidth(250);
			ImGui::SliderFloat(label_new_params.c_str(), &new_width, 1, 30);
			ImGui::SameLine(); ImGui::Text("New Width");

			if (ImGui::Button("Add Coupling", ImVec2(120, 0)))
			{
				element::GaussCoupling newCoupling{
					static_cast<double>(new_x_i),
					static_cast<double>(new_x_j),
					static_cast<double>(new_amplitude),
					static_cast<double>(new_width)
				};
				gfc->addCoupling(newCoupling);
				gfc->init();

				// Reset parameters
				new_x_i = 0.0f;
				new_x_j = 0.0f;
				new_amplitude = 1.0f;
				new_width = 1.0f;

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ElementWindow::modifyElementOscillatoryKernel(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto kernel = std::dynamic_pointer_cast<element::OscillatoryKernel>(element);
		element::OscillatoryKernelParameters okp = kernel->getParameters();

		auto amplitude = static_cast<float>(okp.amplitude);
		auto decay = static_cast<float>(okp.decay);
		auto zeroCrossings = static_cast<float>(okp.zeroCrossings);
		auto amplitudeGlobal = static_cast<float>(okp.amplitudeGlobal);
		bool circular = okp.circular;
		bool normalized = okp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, 0.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Decay";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &decay, 0.005f, 0.001f, 10.0f);
		ImGui::SameLine(); ImGui::Text("Decay");

		label = "##" + element->getUniqueName() + "Zero crossings";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &zeroCrossings, 0.005f, 0.0f, 1.0f);
		ImGui::SameLine(); ImGui::Text("Zero crossings");

		label = "##" + element->getUniqueName() + "Amp. global";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.01f, -10.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Amp. global");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");

		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::SameLine(); ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(okp.amplitude)) > epsilon ||
			std::abs(decay - static_cast<float>(okp.decay)) > epsilon ||
			std::abs(zeroCrossings - static_cast<float>(okp.zeroCrossings)) > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(okp.amplitudeGlobal)) > epsilon ||
			circular != okp.circular ||
			normalized != okp.normalized)
		{
			okp.amplitude = amplitude;
			okp.decay = decay;
			okp.zeroCrossings = zeroCrossings;
			okp.amplitudeGlobal = amplitudeGlobal;
			okp.circular = circular;
			okp.normalized = normalized;
			kernel->setParameters(okp);
		}
	}

	void ElementWindow::modifyElementAsymmetricGaussKernel(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto kernel = std::dynamic_pointer_cast<element::AsymmetricGaussKernel>(element);
		element::AsymmetricGaussKernelParameters agkp = kernel->getParameters();

		auto amplitude = static_cast<float>(agkp.amplitude);
		auto width = static_cast<float>(agkp.width);
		auto amplitudeGlobal = static_cast<float>(agkp.amplitudeGlobal);
		auto timeShift = static_cast<float>(agkp.timeShift);
		bool circular = agkp.circular;
		bool normalized = agkp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.05f, -30.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Width";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &width, 0.005f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Amp. global";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.005f, -10.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Amp. global");

		label = "##" + element->getUniqueName() + "Time shift";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &timeShift, 0.01f, -10, 10);
		ImGui::SameLine(); ImGui::Text("Time shift");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");

		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::SameLine(); ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(agkp.amplitude)) > epsilon ||
			std::abs(width - static_cast<float>(agkp.width)) > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(agkp.amplitudeGlobal)) > epsilon ||
			std::abs(timeShift - static_cast<float>(agkp.timeShift)) > epsilon ||
			circular != agkp.circular ||
			normalized != agkp.normalized)
		{
			agkp.amplitude = amplitude;
			agkp.width = width;
			agkp.amplitudeGlobal = amplitudeGlobal;
			agkp.timeShift = timeShift;
			agkp.circular = circular;
			agkp.normalized = normalized;
			kernel->setParameters(agkp);
		}
	}

	void ElementWindow::modifyElementBoostStimulus(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto boostStimulus = std::dynamic_pointer_cast<element::BoostStimulus>(element);
		element::BoostStimulusParameters bsp = boostStimulus->getParameters();

		auto amplitude = static_cast<float>(bsp.amplitude);
		bool isActive = bsp.isActive;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, -30.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "IsActive";
		ImGui::Checkbox(label.c_str(), &isActive);
		ImGui::SameLine(); ImGui::Text("Active");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(bsp.amplitude)) > epsilon ||
			isActive != bsp.isActive)
		{
			bsp.amplitude = amplitude;
			bsp.isActive = isActive;
			boostStimulus->setParameters(bsp);
		}
	}

	void ElementWindow::modifyElementMemoryTrace(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto memoryTrace = std::dynamic_pointer_cast<element::MemoryTrace>(element);
		element::MemoryTraceParameters mtp = memoryTrace->getParameters();

		auto tauBuild  = static_cast<float>(mtp.tauBuild);
		auto tauDecay  = static_cast<float>(mtp.tauDecay);
		auto threshold = static_cast<float>(mtp.threshold);

		std::string label = "##" + element->getUniqueName() + "TauBuild";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &tauBuild, 1.0f, 1.0f, 10000.0f);
		ImGui::SameLine(); ImGui::Text("Tau build");

		label = "##" + element->getUniqueName() + "TauDecay";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &tauDecay, 5.0f, 1.0f, 100000.0f);
		ImGui::SameLine(); ImGui::Text("Tau decay");

		label = "##" + element->getUniqueName() + "Threshold";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &threshold, 0.01f, -2.0f, 2.0f);
		ImGui::SameLine(); ImGui::Text("Threshold");

		static constexpr double epsilon = 1e-6;
		if (std::abs(tauBuild  - static_cast<float>(mtp.tauBuild))  > epsilon ||
			std::abs(tauDecay  - static_cast<float>(mtp.tauDecay))  > epsilon ||
			std::abs(threshold - static_cast<float>(mtp.threshold)) > epsilon)
		{
			mtp.tauBuild  = tauBuild;
			mtp.tauDecay  = tauDecay;
			mtp.threshold = threshold;
			memoryTrace->setParameters(mtp);
		}
	}

	void ElementWindow::modifyElementNeuralField2D(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto nf = std::dynamic_pointer_cast<element::NeuralField2D>(element);
		element::NeuralField2DParameters p = nf->getParameters();

		auto tau           = static_cast<float>(p.tau);
		auto restingLevel  = static_cast<float>(p.startingRestingLevel);

		std::string label = "##" + element->getUniqueName() + "Tau";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &tau, 0.5f, 1.0f, 1000.0f);
		ImGui::SameLine(); ImGui::Text("Tau");

		label = "##" + element->getUniqueName() + "RestingLevel";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &restingLevel, 0.1f, -30.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Resting level");

		static constexpr double epsilon = 1e-6;
		if (std::abs(tau - static_cast<float>(p.tau)) > epsilon ||
			std::abs(restingLevel - static_cast<float>(p.startingRestingLevel)) > epsilon)
		{
			p.tau = tau;
			p.startingRestingLevel = restingLevel;
			nf->setParameters(p);
		}
	}

	void ElementWindow::modifyElementGaussStimulus2D(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto gs = std::dynamic_pointer_cast<element::GaussStimulus2D>(element);
		element::GaussStimulusParameters2D p = gs->getParameters();

		auto width      = static_cast<float>(p.width);
		auto amplitude  = static_cast<float>(p.amplitude);
		auto posX       = static_cast<float>(p.position_x);
		auto posY       = static_cast<float>(p.position_y);
		bool circular   = p.circular;
		bool normalized = p.normalized;

		std::string label = "##" + element->getUniqueName() + "Width";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &width, 0.1f, 0.1f, 100.0f);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.5f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "PosX";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &posX, 0.5f, 0.0f, 1000.0f);
		ImGui::SameLine(); ImGui::Text("Position x");

		label = "##" + element->getUniqueName() + "PosY";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &posY, 0.5f, 0.0f, 1000.0f);
		ImGui::SameLine(); ImGui::Text("Position y");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");
		ImGui::SameLine();
		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(width - static_cast<float>(p.width)) > epsilon ||
			std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon ||
			std::abs(posX - static_cast<float>(p.position_x)) > epsilon ||
			std::abs(posY - static_cast<float>(p.position_y)) > epsilon ||
			circular != p.circular || normalized != p.normalized)
		{
			p.width = width; p.amplitude = amplitude;
			p.position_x = posX; p.position_y = posY;
			p.circular = circular; p.normalized = normalized;
			gs->setParameters(p);
		}
	}

	void ElementWindow::modifyElementGaussKernel2D(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto gk = std::dynamic_pointer_cast<element::GaussKernel2D>(element);
		element::GaussKernel2DParameters p = gk->getParameters();

		auto width           = static_cast<float>(p.width);
		auto amplitude       = static_cast<float>(p.amplitude);
		auto amplitudeGlobal = static_cast<float>(p.amplitudeGlobal);
		bool circular   = p.circular;
		bool normalized = p.normalized;

		std::string label = "##" + element->getUniqueName() + "Width";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &width, 0.1f, 0.1f, 100.0f);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "AmplitudeGlobal";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.001f, -1.0f, 1.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude global");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");
		ImGui::SameLine();
		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(width - static_cast<float>(p.width)) > epsilon ||
			std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(p.amplitudeGlobal)) > epsilon ||
			circular != p.circular || normalized != p.normalized)
		{
			p.width = width; p.amplitude = amplitude;
			p.amplitudeGlobal = amplitudeGlobal;
			p.circular = circular; p.normalized = normalized;
			gk->setParameters(p);
		}
	}

	void ElementWindow::modifyElementMexicanHatKernel2D(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto mhk = std::dynamic_pointer_cast<element::MexicanHatKernel2D>(element);
		element::MexicanHatKernel2DParameters p = mhk->getParameters();

		auto widthExc        = static_cast<float>(p.widthExc);
		auto amplitudeExc    = static_cast<float>(p.amplitudeExc);
		auto widthInh        = static_cast<float>(p.widthInh);
		auto amplitudeInh    = static_cast<float>(p.amplitudeInh);
		auto amplitudeGlobal = static_cast<float>(p.amplitudeGlobal);
		bool circular   = p.circular;
		bool normalized = p.normalized;

		std::string label = "##" + element->getUniqueName() + "WidthExc";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &widthExc, 0.1f, 0.1f, 100.0f);
		ImGui::SameLine(); ImGui::Text("Width exc");

		label = "##" + element->getUniqueName() + "AmpExc";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeExc, 0.1f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude exc");

		label = "##" + element->getUniqueName() + "WidthInh";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &widthInh, 0.1f, 0.1f, 100.0f);
		ImGui::SameLine(); ImGui::Text("Width inh");

		label = "##" + element->getUniqueName() + "AmpInh";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeInh, 0.1f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude inh");

		label = "##" + element->getUniqueName() + "AmpGlobal";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.001f, -1.0f, 1.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude global");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");
		ImGui::SameLine();
		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(widthExc - static_cast<float>(p.widthExc)) > epsilon ||
			std::abs(amplitudeExc - static_cast<float>(p.amplitudeExc)) > epsilon ||
			std::abs(widthInh - static_cast<float>(p.widthInh)) > epsilon ||
			std::abs(amplitudeInh - static_cast<float>(p.amplitudeInh)) > epsilon ||
			std::abs(amplitudeGlobal - static_cast<float>(p.amplitudeGlobal)) > epsilon ||
			circular != p.circular || normalized != p.normalized)
		{
			p.widthExc = widthExc; p.amplitudeExc = amplitudeExc;
			p.widthInh = widthInh; p.amplitudeInh = amplitudeInh;
			p.amplitudeGlobal = amplitudeGlobal;
			p.circular = circular; p.normalized = normalized;
			mhk->setParameters(p);
		}
	}

	void ElementWindow::modifyElementNormalNoise2D(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;
		const auto nn = std::dynamic_pointer_cast<element::NormalNoise2D>(element);
		element::NormalNoise2DParameters p = nn->getParameters();

		auto amplitude = static_cast<float>(p.amplitude);

		const std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.001f, 0.0f, 10.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude - static_cast<float>(p.amplitude)) > epsilon)
		{
			p.amplitude = amplitude;
			nn->setParameters(p);
		}
	}

	void ElementWindow::modifyElementOscillatoryKernel2D(const std::shared_ptr<element::Element>& element)
	{
		const float ui = ImGui::GetIO().FontGlobalScale;

		const auto kernel = std::dynamic_pointer_cast<element::OscillatoryKernel2D>(element);
		element::OscillatoryKernel2DParameters p = kernel->getParameters();

		auto amplitude      = static_cast<float>(p.amplitude);
		auto decay          = static_cast<float>(p.decay);
		auto zeroCrossings  = static_cast<float>(p.zeroCrossings);
		auto amplitudeGlobal = static_cast<float>(p.amplitudeGlobal);
		bool circular       = p.circular;
		bool normalized     = p.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, 0.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Decay";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &decay, 0.005f, 0.001f, 10.0f);
		ImGui::SameLine(); ImGui::Text("Decay");

		label = "##" + element->getUniqueName() + "Zero crossings";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &zeroCrossings, 0.005f, 0.0f, 1.0f);
		ImGui::SameLine(); ImGui::Text("Zero crossings");

		label = "##" + element->getUniqueName() + "Amp. global";
		ImGui::SetNextItemWidth(150.0f * ui);
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.01f, -10.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Amp. global");

		label = "##" + element->getUniqueName() + "Circular";
		ImGui::Checkbox(label.c_str(), &circular);
		ImGui::SameLine(); ImGui::Text("Circular");

		label = "##" + element->getUniqueName() + "Normalized";
		ImGui::SameLine(); ImGui::Checkbox(label.c_str(), &normalized);
		ImGui::SameLine(); ImGui::Text("Normalized");

		static constexpr double epsilon = 1e-6;
		if (std::abs(amplitude     - static_cast<float>(p.amplitude))     > epsilon ||
		    std::abs(decay         - static_cast<float>(p.decay))         > epsilon ||
		    std::abs(zeroCrossings - static_cast<float>(p.zeroCrossings)) > epsilon ||
		    std::abs(amplitudeGlobal - static_cast<float>(p.amplitudeGlobal)) > epsilon ||
		    circular != p.circular || normalized != p.normalized)
		{
			p.amplitude      = amplitude;
			p.decay          = decay;
			p.zeroCrossings  = zeroCrossings;
			p.amplitudeGlobal = amplitudeGlobal;
			p.circular       = circular;
			p.normalized     = normalized;
			kernel->setParameters(p);
		}
	}

	ImVec4 ElementWindow::getColorForElementType(const element::ElementLabel label)
	{
		switch (label)
		{
		case element::ElementLabel::NEURAL_FIELD:
			return ImVec4(0.337f, 0.502f, 0.749f, 1.0f);  // Soft Blue
		case element::ElementLabel::NORMAL_NOISE:
			return ImVec4(0.875f, 0.580f, 0.329f, 1.0f);  // Warm Orange
		case element::ElementLabel::GAUSS_KERNEL:
			return ImVec4(0.749f, 0.247f, 0.247f, 1.0f);  // Muted Red
		case element::ElementLabel::GAUSS_STIMULUS:
			return ImVec4(0.498f, 0.749f, 0.498f, 1.0f);  // Sage Green
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
			return ImVec4(0.604f, 0.475f, 0.749f, 1.0f);  // Lavender
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			return ImVec4(0.647f, 0.400f, 0.278f, 1.0f);  // Warm Brown
		case element::ElementLabel::FIELD_COUPLING:
			return ImVec4(0.831f, 0.753f, 0.475f, 1.0f);  // Cream Gold
		case element::ElementLabel::OSCILLATORY_KERNEL:
			return ImVec4(0.686f, 0.522f, 0.733f, 1.0f);  // Dusty Rose
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
			return ImVec4(0.580f, 0.698f, 0.714f, 1.0f);  // Soft Teal
		case element::ElementLabel::BOOST_STIMULUS:
			return ImVec4(0.949f, 0.820f, 0.325f, 1.0f);  // Warm Yellow
		case element::ElementLabel::MEMORY_TRACE:
			return ImVec4(0.431f, 0.627f, 0.549f, 1.0f);  // Sage Green
		case element::ElementLabel::NEURAL_FIELD_2D:
			return ImVec4(0.365f, 0.529f, 0.718f, 1.0f);  // Steel Blue
		case element::ElementLabel::GAUSS_STIMULUS_2D:
			return ImVec4(0.690f, 0.525f, 0.380f, 1.0f);  // Warm Tan
		case element::ElementLabel::GAUSS_KERNEL_2D:
			return ImVec4(0.420f, 0.667f, 0.510f, 1.0f);  // Mint Green
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
			return ImVec4(0.706f, 0.494f, 0.576f, 1.0f);  // Dusty Mauve
		case element::ElementLabel::NORMAL_NOISE_2D:
			return ImVec4(0.600f, 0.600f, 0.600f, 1.0f);  // Light Gray
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
			return ImVec4(0.710f, 0.545f, 0.753f, 1.0f);  // Dusty Violet
		default:
			return ImVec4(0.498f, 0.498f, 0.498f, 1.0f);  // Neutral Gray
		}
	}

	std::string ElementWindow::getElementTypeDisplayName(const element::ElementLabel label)
	{
		switch (label)
		{
		case element::ElementLabel::NEURAL_FIELD: return "Neural Fields";
		case element::ElementLabel::GAUSS_STIMULUS: return "Gaussian Stimuli";
		case element::ElementLabel::FIELD_COUPLING: return "Field Couplings";
		case element::ElementLabel::GAUSS_KERNEL: return "Gaussian Kernels";
		case element::ElementLabel::MEXICAN_HAT_KERNEL: return "Mexican Hat Kernels";
		case element::ElementLabel::NORMAL_NOISE: return "Normal Noise";
		case element::ElementLabel::GAUSS_FIELD_COUPLING: return "Gaussian Field Couplings";
		case element::ElementLabel::OSCILLATORY_KERNEL: return "Oscillatory Kernels";
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL: return "Asymmetric Gaussian Kernels";
		case element::ElementLabel::BOOST_STIMULUS: return "Boost Stimuli";
		case element::ElementLabel::MEMORY_TRACE:  return "Memory Traces";
		case element::ElementLabel::NEURAL_FIELD_2D: return "Neural Fields 2D";
		case element::ElementLabel::GAUSS_STIMULUS_2D: return "Gauss Stimuli 2D";
		case element::ElementLabel::GAUSS_KERNEL_2D: return "Gauss Kernels 2D";
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D: return "Mexican Hat Kernels 2D";
		case element::ElementLabel::NORMAL_NOISE_2D: return "Normal Noise 2D";
		case element::ElementLabel::OSCILLATORY_KERNEL_2D: return "Oscillatory Kernels 2D";
		default: return "Unknown Elements";
		}
	}

	PanelScope ElementWindow::beginElementPanel(const ImVec4& baseColor, const ImVec2& size)
	{
		// Draw background first, then place the cursor inside for content

		PanelScope p{};
		p.ui       = ImGui::GetIO().FontGlobalScale;
		p.rounding = 12.0f * p.ui;
		p.pad      = ImVec2(10.0f * p.ui, 8.0f * p.ui);

		// soft fill/border from base color
		p.fill   = ImGui::GetColorU32(ImVec4(baseColor.x, baseColor.y, baseColor.z, 0.18f));
		p.border = ImGui::GetColorU32(ImVec4(baseColor.x, baseColor.y, baseColor.z, 0.35f));

		// fixed rect from the current cursor (screen coords)
		const ImVec2 topLeft = ImGui::GetCursorScreenPos();
		const auto bottomRight = ImVec2(topLeft.x + size.x, topLeft.y + size.y);
		p.rect = ImRect(topLeft, bottomRight);

		// draw the panel *behind* content
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled(p.rect.Min, p.rect.Max, p.fill, p.rounding);
		dl->AddRect      (p.rect.Min, p.rect.Max, p.border, p.rounding, 0, 1.5f * p.ui);

		// move the cursor inside, honoring padding, then start the content group
		const auto innerPos = ImVec2(p.rect.Min.x + p.pad.x, p.rect.Min.y + p.pad.y);
		ImGui::SetCursorScreenPos(innerPos);
		ImGui::BeginGroup();
		return p;
	}

	void ElementWindow::endElementPanel(const PanelScope& p)
	{
		ImGui::EndGroup();
		// advance cursor to just below the panel, keeping normal flow
		ImGui::SetCursorScreenPos(ImVec2(p.rect.Min.x, p.rect.Max.y + 6.0f * p.ui));
	}
}
