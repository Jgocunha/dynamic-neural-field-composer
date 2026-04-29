// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "user_interface/element_window.h"

namespace dnf_composer::user_interface
{
	std::shared_ptr<element::Element> ElementWindow::s_focusedElement_ = nullptr;

	void ElementWindow::setFocusedElement(const std::shared_ptr<element::Element>& element)
	{
		s_focusedElement_ = element;
	}

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
			renderElementControlCard();
		ImGui::End();
	}

	void ElementWindow::renderElementControlCard() const
	{
		constexpr ImGuiWindowFlags childFlags =
        ImGuiWindowFlags_NoSavedSettings;

		widgets::renderHelpMarker("Left click and drag or double-click to change element parameter values.");
	    ImGui::BeginChild("##element_scroll", ImVec2(0, 0), false, childFlags);

		// Layout metrics (must be computed before the Selected elements section uses them)
	    const float innerW    = ImGui::GetContentRegionAvail().x;
		const float ui        = ImGui::GetIO().FontGlobalScale;
		const float dragW     = 130.0f * ui;
		const float panelPadX = 10.0f * ui;
		const float spacingX  = ImGui::GetStyle().ItemSpacing.x;

		auto PanelHeightFor = [&](const std::shared_ptr<element::Element>& e) -> float
		{
			const float frameH   = ImGui::GetFrameHeight();
			const float spacingY = ImGui::GetStyle().ItemSpacing.y;
			const float rowH     = frameH + spacingY;
			const float panelPad = 3.0f * 8.0f * ui;

			auto h = [&](const int rows) {
				return rows * rowH - spacingY + panelPad;
			};

			constexpr int dimRows  = 2; // Size + Step
			constexpr int kDimRows = 4; // Size + Step + Output Size + Output Step

			switch (e->getLabel())
			{
				case element::ElementLabel::NORMAL_NOISE:            return h(1  + dimRows);
				case element::ElementLabel::NEURAL_FIELD:            return h(2  + dimRows);
				case element::ElementLabel::GAUSS_STIMULUS:          return h(4  + dimRows);
				case element::ElementLabel::GAUSS_KERNEL:            return h(4  + kDimRows);
				case element::ElementLabel::FIELD_COUPLING:          return h(7  + dimRows);
				case element::ElementLabel::MEXICAN_HAT_KERNEL:      return h(6  + kDimRows);
				case element::ElementLabel::OSCILLATORY_KERNEL:      return h(5  + kDimRows);
				case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL: return h(5  + kDimRows);
				case element::ElementLabel::BOOST_STIMULUS:          return h(2  + dimRows);
				case element::ElementLabel::MEMORY_TRACE:            return h(3  + dimRows);
				case element::ElementLabel::GAUSS_FIELD_COUPLING:
				{
					const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(e);
					const int numCouplings = static_cast<int>(gfc->getParameters().couplings.size());
					return h(4 + 5 * numCouplings + dimRows);
				}
				default: return h(4 + dimRows);
			}
		};

		const float maxNaturalW = 1.0f * panelPadX + dragW + spacingX + ImGui::CalcTextSize("circular + normalized").x;
		const float panelW = ImMin(maxNaturalW, innerW);

		// Validate a focused element still belongs to this simulation
		if (s_focusedElement_)
		{
			bool stillValid = false;
			for (const auto& e : simulation->getElements())
				if (e == s_focusedElement_) { stillValid = true; break; }
			if (!stillValid) s_focusedElement_ = nullptr;
		}

		// Selected elements section — shows the most recently single-clicked node
		if (s_focusedElement_)
		{
			ImGui::PushID("##sel_focused");
			ImGui::PushFont(g_BoldLargeFont);
			ImGui::TextUnformatted("Selected Element");
			ImGui::PopFont();
			ImGui::Spacing();

			ImGui::TextUnformatted(s_focusedElement_->getUniqueName().c_str());
			ImGui::Spacing();
			const ImVec4 tint = getColorForElementType(s_focusedElement_->getLabel());
			const ImVec2 selSize(panelW, PanelHeightFor(s_focusedElement_));
			PanelScope scope = beginElementPanel(tint, selSize);
			{
				ImGui::PushItemWidth(dragW);
				switchElementToModify(s_focusedElement_);
				ImGui::PopItemWidth();
				renderDimensionControls(s_focusedElement_);
			}
			endElementPanel(scope);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::PopID();
		}

	    // group elements by type
	    std::map<element::ElementLabel, std::vector<std::shared_ptr<element::Element>>> byType;
	    for (const auto& e : simulation->getElements())
	        byType[e->getLabel()].push_back(e);

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

	            PanelScope scope = beginElementPanel(tint, size);
	            {
	                switchElementToModify(e);
	                renderDimensionControls(e);
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

	void ElementWindow::renderDimensionControls(const std::shared_ptr<element::Element>& element) const
	{
		const float ui     = ImGui::GetIO().FontGlobalScale;
		const float inputW = 150.0f * ui;
		const std::string elemId = element->getUniqueName();
		const element::ElementLabel label = element->getLabel();
		const bool isCoupling = label == element::ElementLabel::FIELD_COUPLING ||
		                        label == element::ElementLabel::GAUSS_FIELD_COUPLING;
		const bool isKernel   = label == element::ElementLabel::GAUSS_KERNEL ||
		                        label == element::ElementLabel::MEXICAN_HAT_KERNEL ||
		                        label == element::ElementLabel::OSCILLATORY_KERNEL ||
		                        label == element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL;

		static std::unordered_map<int, std::pair<float, float>> staged;
		static std::unordered_map<int, std::pair<float, float>> stagedIn;
		static std::unordered_map<int, std::pair<float, float>> stagedOut;
		const int id = element->getUniqueIdentifier();
		auto& [stagedXmax, stagedDx] = staged[id];

		const auto& dim = element->getElementCommonParameters().dimensionParameters;
		if (stagedXmax == 0.0f && stagedDx == 0.0f)
		{
			stagedXmax = static_cast<float>(dim.x_max);
			stagedDx   = static_cast<float>(dim.d_x);
		}

		ImGui::Separator();
		ImGui::PushID(("##dim_" + elemId).c_str());

		ImGui::SetNextItemWidth(inputW);
		ImGui::InputFloat("##x_max", &stagedXmax, 0.0f, 0.0f, "%.1f");
		if (ImGui::IsItemDeactivatedAfterEdit() && stagedXmax > 0.0f && stagedDx > 0.0f)
		{
			const element::ElementDimensions newDim(static_cast<int>(stagedXmax), static_cast<double>(stagedDx));
			simulation->changeDimensions(elemId, newDim);
			stagedXmax = static_cast<float>(newDim.x_max);
			stagedDx   = static_cast<float>(newDim.d_x);
		}
		ImGui::SameLine(); ImGui::TextUnformatted(isCoupling ? "Out size" : "Size");
		ImGui::SameLine();
		widgets::renderHelpMarker(
			"Changing the field size will disconnect all existing connections\n"
			"to and from this element. Press Enter to commit the new size."
		);

		ImGui::SetNextItemWidth(inputW);
		ImGui::InputFloat("##dx", &stagedDx, 0.0f, 0.0f, "%.2f");
		if (ImGui::IsItemDeactivatedAfterEdit() && stagedXmax > 0.0f && stagedDx > 0.0f)
		{
			const element::ElementDimensions newDim(static_cast<int>(stagedXmax), static_cast<double>(stagedDx));
			simulation->changeDimensions(elemId, newDim);
			stagedXmax = static_cast<float>(newDim.x_max);
			stagedDx   = static_cast<float>(newDim.d_x);
		}
		ImGui::SameLine(); ImGui::TextUnformatted(isCoupling ? "Out step" : "Step");

		if (isKernel)
		{
			auto& [stagedOutXmax, stagedOutDx] = stagedOut[id];

			auto getKernelOutputDims = [&]() -> std::optional<element::ElementDimensions> {
				if (label == element::ElementLabel::GAUSS_KERNEL)
					return std::dynamic_pointer_cast<element::GaussKernel>(element)->getParameters().outputFieldDimensions;
				if (label == element::ElementLabel::MEXICAN_HAT_KERNEL)
					return std::dynamic_pointer_cast<element::MexicanHatKernel>(element)->getParameters().outputFieldDimensions;
				if (label == element::ElementLabel::OSCILLATORY_KERNEL)
					return std::dynamic_pointer_cast<element::OscillatoryKernel>(element)->getParameters().outputFieldDimensions;
				if (label == element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL)
					return std::dynamic_pointer_cast<element::AsymmetricGaussKernel>(element)->getParameters().outputFieldDimensions;
				return std::nullopt;
			};

			if (stagedOutXmax == 0.0f && stagedOutDx == 0.0f)
			{
				const auto currentOut = getKernelOutputDims();
				if (currentOut.has_value())
				{
					stagedOutXmax = static_cast<float>(currentOut->x_max);
					stagedOutDx   = static_cast<float>(currentOut->d_x);
				}
				else
				{
					stagedOutXmax = static_cast<float>(dim.x_max);
					stagedOutDx   = static_cast<float>(dim.d_x);
				}
			}

			auto applyKernelOutputDim = [&]() {
				if (stagedOutXmax <= 0.0f || stagedOutDx <= 0.0f) return;
				const element::ElementDimensions newOutDim(static_cast<int>(stagedOutXmax), static_cast<double>(stagedOutDx));
				const bool squareMode = (newOutDim.x_max == element->getMaxSpatialDimension() &&
				                        std::abs(newOutDim.d_x - element->getStepSize()) < 1e-9);
				const std::optional<element::ElementDimensions> newOptOut =
					squareMode ? std::nullopt : std::make_optional(newOutDim);
				element->removeInputs();
				element->removeOutputs();
				if (label == element::ElementLabel::GAUSS_KERNEL)
				{
					const auto k = std::dynamic_pointer_cast<element::GaussKernel>(element);
					auto p = k->getParameters();
					p.outputFieldDimensions = newOptOut;
					k->setParameters(p);
				}
				else if (label == element::ElementLabel::MEXICAN_HAT_KERNEL)
				{
					const auto k = std::dynamic_pointer_cast<element::MexicanHatKernel>(element);
					auto p = k->getParameters();
					p.outputFieldDimensions = newOptOut;
					k->setParameters(p);
				}
				else if (label == element::ElementLabel::OSCILLATORY_KERNEL)
				{
					const auto k = std::dynamic_pointer_cast<element::OscillatoryKernel>(element);
					auto p = k->getParameters();
					p.outputFieldDimensions = newOptOut;
					k->setParameters(p);
				}
				else if (label == element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL)
				{
					const auto k = std::dynamic_pointer_cast<element::AsymmetricGaussKernel>(element);
					auto p = k->getParameters();
					p.outputFieldDimensions = newOptOut;
					k->setParameters(p);
				}
				const auto finalOut = getKernelOutputDims();
				if (finalOut.has_value())
				{
					stagedOutXmax = static_cast<float>(finalOut->x_max);
					stagedOutDx   = static_cast<float>(finalOut->d_x);
				}
				else
				{
					stagedOutXmax = static_cast<float>(element->getElementCommonParameters().dimensionParameters.x_max);
					stagedOutDx   = static_cast<float>(element->getElementCommonParameters().dimensionParameters.d_x);
				}
			};

			ImGui::SetNextItemWidth(inputW);
			ImGui::InputFloat("##out_x_max", &stagedOutXmax, 0.0f, 0.0f, "%.1f");
			if (ImGui::IsItemDeactivatedAfterEdit()) applyKernelOutputDim();
			ImGui::SameLine(); ImGui::TextUnformatted("Output Size");

			ImGui::SetNextItemWidth(inputW);
			ImGui::InputFloat("##out_dx", &stagedOutDx, 0.0f, 0.0f, "%.2f");
			if (ImGui::IsItemDeactivatedAfterEdit()) applyKernelOutputDim();
			ImGui::SameLine(); ImGui::TextUnformatted("Output Step");
		}

		if (isCoupling)
		{
			auto& [stagedInXmax, stagedInDx] = stagedIn[id];

			element::ElementDimensions currentInputDim{};
			if (label == element::ElementLabel::FIELD_COUPLING)
			{
				const auto fc = std::dynamic_pointer_cast<element::FieldCoupling>(element);
				currentInputDim = fc->getParameters().inputFieldDimensions;
			}
			else
			{
				const auto gfc = std::dynamic_pointer_cast<element::GaussFieldCoupling>(element);
				currentInputDim = gfc->getInputFieldDimensions();
			}

			if (stagedInXmax == 0.0f && stagedInDx == 0.0f)
			{
				stagedInXmax = static_cast<float>(currentInputDim.x_max);
				stagedInDx   = static_cast<float>(currentInputDim.d_x);
			}

			auto applyInputDim = [&]() {
				if (stagedInXmax <= 0.0f || stagedInDx <= 0.0f) return;
				const element::ElementDimensions newInDim(static_cast<int>(stagedInXmax), static_cast<double>(stagedInDx));
				element->removeInputs();
				element->removeOutputs();
				if (label == element::ElementLabel::FIELD_COUPLING)
					std::dynamic_pointer_cast<element::FieldCoupling>(element)->changeInputDimensions(newInDim);
				else
					std::dynamic_pointer_cast<element::GaussFieldCoupling>(element)->changeInputDimensions(newInDim);
				stagedInXmax = static_cast<float>(newInDim.x_max);
				stagedInDx   = static_cast<float>(newInDim.d_x);
			};

			ImGui::SetNextItemWidth(inputW);
			ImGui::InputFloat("##in_x_max", &stagedInXmax, 0.0f, 0.0f, "%.1f");
			if (ImGui::IsItemDeactivatedAfterEdit()) applyInputDim();
			ImGui::SameLine(); ImGui::TextUnformatted("In size");

			ImGui::SetNextItemWidth(inputW);
			ImGui::InputFloat("##in_dx", &stagedInDx, 0.0f, 0.0f, "%.2f");
			if (ImGui::IsItemDeactivatedAfterEdit()) applyInputDim();
			ImGui::SameLine(); ImGui::TextUnformatted("In step");
		}

		ImGui::PopID();
	}

	void ElementWindow::switchElementToModify(const std::shared_ptr<element::Element>& element)
	{
		const std::string elementId = element->getUniqueName();
		static bool missingElementAcknowledged = false;

		switch (const element::ElementLabel label = element->getLabel())
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
		case element::ElementLabel::UNINITIALIZED:
			break;
		default:
			if (!missingElementAcknowledged)
			{
				log(tools::logger::LogLevel::ERROR, "There is a missing element in the "
					"TreeNode in simulation window.");
				missingElementAcknowledged = true;
			}
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
