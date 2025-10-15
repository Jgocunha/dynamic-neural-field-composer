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
		if (ImGui::Begin("Element Control", nullptr, imgui_kit::getGlobalWindowFlags()))
		{
			renderModifyElementParameters();
		}
		ImGui::End();
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

		ImGui::SeparatorText( ("Element " + elementId).c_str() );

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
		const auto neuralField = std::dynamic_pointer_cast<element::NeuralField>(element);
		element::NeuralFieldParameters nfp = neuralField->getParameters();

		auto restingLevel = static_cast<float>(nfp.startingRestingLevel);
		auto tau = static_cast<float>(nfp.tau);
		auto stabilityThreshold = static_cast<float>(neuralField->getStabilityThreshold());

		std::string label = "##" + element->getUniqueName() + "Resting level";
		ImGui::DragFloat(label.c_str(), &restingLevel, 0.1f, -30.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Resting level");

		label = "##" + element->getUniqueName() + "Tau";
		ImGui::DragFloat(label.c_str(), &tau, 0.5f, 1.0f, 300.0f);
		ImGui::SameLine(); ImGui::Text("Tau");

		// stability threshold
		label = "##" + element->getUniqueName() + "Stability threshold";
		ImGui::DragFloat(label.c_str(), &stabilityThreshold, 0.01f, 0.0f, 2.0f);
		ImGui::SameLine(); ImGui::Text("Stability threshold");


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

	if (std::abs(stabilityThreshold - static_cast<float>(neuralField->getStabilityThreshold())) > epsilon)
	{
		neuralField->setThresholdForStability(stabilityThreshold);
	}
}

	void ElementWindow::modifyElementGaussStimulus(const std::shared_ptr<element::Element>& element)
	{
		const auto stimulus = std::dynamic_pointer_cast<element::GaussStimulus>(element);
		element::GaussStimulusParameters gsp = stimulus->getParameters();

		auto amplitude = static_cast<float>(gsp.amplitude);
		auto width = static_cast<float>(gsp.width);
		auto position = static_cast<float>(gsp.position);
		bool circular = gsp.circular;
		bool normalized = gsp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, 0, 30);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Width";
		ImGui::DragFloat(label.c_str(), &width, 0.01f, 0, 30);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Position";
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
		const auto fieldCoupling = std::dynamic_pointer_cast<element::FieldCoupling>(element);
		element::FieldCouplingParameters fcp = fieldCoupling->getParameters();

		auto scalar = static_cast<float>(fcp.scalar);
		auto learningRate = static_cast<float>(fcp.learningRate);
		bool activateLearning = fcp.isLearningActive;

		std::string label = "##" + element->getUniqueName() + "Learning rule";
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

		label = "##" + element->getUniqueName() + "Learning rate";
		ImGui::DragFloat(label.c_str(), &learningRate, 0.01f, 0, 10);
		ImGui::SameLine(); ImGui::Text("Learning rate");

		label = "##" + element->getUniqueName() + "Scalar";
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

		if (ImGui::Button("Read weights"))
		{
			fieldCoupling->readWeights();
		}
		ImGui::SameLine();

		if (ImGui::Button("Save weights"))
		{
			fieldCoupling->writeWeights();
		}
		ImGui::SameLine();

		if (ImGui::Button("Clear weights"))
		{
			fieldCoupling->clearWeights();
		}

		ImGui::PopID(); // End unique ID scope
	}

	void ElementWindow::modifyElementGaussKernel(const std::shared_ptr<element::Element>& element)
	{
		const auto kernel = std::dynamic_pointer_cast<element::GaussKernel>(element);
		element::GaussKernelParameters gkp = kernel->getParameters();

		auto amplitude = static_cast<float>(gkp.amplitude);
		auto width = static_cast<float>(gkp.width);
		auto amplitudeGlobal = static_cast<float>(gkp.amplitudeGlobal);
		bool circular = gkp.circular;
		bool normalized = gkp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Width";
		ImGui::DragFloat(label.c_str(), &width, 0.1f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Amplitude global";
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.1f, -10, 10);
		ImGui::SameLine(); ImGui::Text("Amplitude global");

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
		const auto kernel = std::dynamic_pointer_cast<element::MexicanHatKernel>(element);
		element::MexicanHatKernelParameters mhkp = kernel->getParameters();

		auto amplitudeExc = static_cast<float>(mhkp.amplitudeExc);
		auto widthExc = static_cast<float>(mhkp.widthExc);
		auto amplitudeInh = static_cast<float>(mhkp.amplitudeInh);
		auto widthInh = static_cast<float>(mhkp.widthInh);
		auto amplitudeGlobal = static_cast<float>(mhkp.amplitudeGlobal);
		bool circular = mhkp.circular;
		bool normalized = mhkp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude exc.";
		ImGui::DragFloat(label.c_str(), &amplitudeExc, 0.1f, -50.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude exc.");

		label = "##" + element->getUniqueName() + "Width exc.";
		ImGui::DragFloat(label.c_str(), &widthExc, 0.1f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width exc.");

		label = "##" + element->getUniqueName() + "Amplitude inh.";
		ImGui::DragFloat(label.c_str(), &amplitudeInh, 0.1f, 0.0f, 100.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude inh.");

		label = "##" + element->getUniqueName() + "Width inh.";
		ImGui::DragFloat(label.c_str(), &widthInh, 0.1f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width inh.");

		label = "##" + element->getUniqueName() + "Amplitude global";
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.01f, -10.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude global");

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
		const auto normalNoise = std::dynamic_pointer_cast<element::NormalNoise>(element);
		element::NormalNoiseParameters nnp = normalNoise->getParameters();

		auto amplitude = static_cast<float>(nnp.amplitude);

		const std::string label = "##" + element->getUniqueName() + "Amplitude";
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

			label = "##" + element->getUniqueName() + "x_i" + std::to_string(couplingIndex);
			ImGui::DragFloat(label.c_str(), &x_i, 0.05f, 0.0f, static_cast<float>(other_size));
			text = "x_i " + std::to_string(couplingIndex);
			ImGui::SameLine(); ImGui::Text(text.c_str());

			label = "##" + element->getUniqueName() + "x_j" + std::to_string(couplingIndex);
			ImGui::DragFloat(label.c_str(), &x_j, 0.05f, 0.0f, static_cast<float>(size));
			text = "x_j " + std::to_string(couplingIndex);
			ImGui::SameLine(); ImGui::Text(text.c_str());

			label = "##" + element->getUniqueName() + "Amplitude" + std::to_string(couplingIndex);
			ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, 0.0f, 100.0f);
			text = "Amplitude " + std::to_string(couplingIndex);
			ImGui::SameLine(); ImGui::Text(text.c_str());

			label = "##" + element->getUniqueName() + "Width" + std::to_string(couplingIndex);
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

		// Section: Add New Coupling
		ImGui::Separator();

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
		const auto kernel = std::dynamic_pointer_cast<element::OscillatoryKernel>(element);
		element::OscillatoryKernelParameters okp = kernel->getParameters();

		auto amplitude = static_cast<float>(okp.amplitude);
		auto decay = static_cast<float>(okp.decay);
		auto zeroCrossings = static_cast<float>(okp.zeroCrossings);
		auto amplitudeGlobal = static_cast<float>(okp.amplitudeGlobal);
		bool circular = okp.circular;
		bool normalized = okp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::DragFloat(label.c_str(), &amplitude, 0.1f, 0.0f, 50.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Decay";
		ImGui::DragFloat(label.c_str(), &decay, 0.005f, 0.001f, 10.0f);
		ImGui::SameLine(); ImGui::Text("Decay");

		label = "##" + element->getUniqueName() + "Zero crossings";
		ImGui::DragFloat(label.c_str(), &zeroCrossings, 0.005f, 0.0f, 1.0f);
		ImGui::SameLine(); ImGui::Text("Zero crossings");

		label = "##" + element->getUniqueName() + "Amplitude global";
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.01f, -10.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude global");

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
		const auto kernel = std::dynamic_pointer_cast<element::AsymmetricGaussKernel>(element);
		element::AsymmetricGaussKernelParameters agkp = kernel->getParameters();

		auto amplitude = static_cast<float>(agkp.amplitude);
		auto width = static_cast<float>(agkp.width);
		auto amplitudeGlobal = static_cast<float>(agkp.amplitudeGlobal);
		auto timeShift = static_cast<float>(agkp.timeShift);
		bool circular = agkp.circular;
		bool normalized = agkp.normalized;

		std::string label = "##" + element->getUniqueName() + "Amplitude";
		ImGui::DragFloat(label.c_str(), &amplitude, 0.05f, -30.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude");

		label = "##" + element->getUniqueName() + "Width";
		ImGui::DragFloat(label.c_str(), &width, 0.005f, 0.0f, 30.0f);
		ImGui::SameLine(); ImGui::Text("Width");

		label = "##" + element->getUniqueName() + "Amplitude global";
		ImGui::DragFloat(label.c_str(), &amplitudeGlobal, 0.005f, -10.0f, 0.0f);
		ImGui::SameLine(); ImGui::Text("Amplitude global");

		label = "##" + element->getUniqueName() + "Time shift";
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

	ImVec4 ElementWindow::getColorForElementType(element::ElementLabel label)
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
		default:
			return ImVec4(0.498f, 0.498f, 0.498f, 1.0f);  // Neutral Gray
		}
	}

	std::string ElementWindow::getIconForElementType(element::ElementLabel label)
	{
		switch (label)
		{
		case element::ElementLabel::NEURAL_FIELD:
			return " ";
		case element::ElementLabel::GAUSS_STIMULUS:
			return " ";
		case element::ElementLabel::FIELD_COUPLING:
			return " ";
		case element::ElementLabel::GAUSS_KERNEL:
			return " ";
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
			return " ";
		case element::ElementLabel::NORMAL_NOISE:
			return " ";
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			return " ";
		case element::ElementLabel::OSCILLATORY_KERNEL:
			return " ";
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
			return " ";
		default:
			return "? ";  // Question mark
		}
	}

	std::string ElementWindow::getElementTypeDisplayName(element::ElementLabel label)
	{
		switch (label)
		{
		case element::ElementLabel::NEURAL_FIELD: return "Neural Fields";
		case element::ElementLabel::GAUSS_STIMULUS: return "Gauss Stimuli";
		case element::ElementLabel::FIELD_COUPLING: return "Field Couplings";
		case element::ElementLabel::GAUSS_KERNEL: return "Gauss Kernels";
		case element::ElementLabel::MEXICAN_HAT_KERNEL: return "Mexican Hat Kernels";
		case element::ElementLabel::NORMAL_NOISE: return "Normal Noise";
		case element::ElementLabel::GAUSS_FIELD_COUPLING: return "Gauss Field Couplings";
		case element::ElementLabel::OSCILLATORY_KERNEL: return "Oscillatory Kernels";
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL: return "Asymmetric Gauss Kernels";
		default: return "Unknown Elements";
		}
	}
}
