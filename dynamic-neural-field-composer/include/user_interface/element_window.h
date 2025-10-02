#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/gauss_stimulus.h"
#include "elements/neural_field.h"
#include "elements/field_coupling.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/gauss_field_coupling.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"

namespace dnf_composer::user_interface
{
	class ElementWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
	public:
		explicit ElementWindow(const std::shared_ptr<Simulation>& simulation);

		ElementWindow(const ElementWindow&) = delete;
		ElementWindow& operator=(const ElementWindow&) = delete;
		ElementWindow(ElementWindow&&) = delete;
		ElementWindow& operator=(ElementWindow&&) = delete;

		void render() override;
		~ElementWindow() override = default;
	private:
		void renderModifyElementParameters() const;
		static void switchElementToModify(const std::shared_ptr<element::Element>& element);
		static void modifyElementNeuralField(const std::shared_ptr<element::Element>& element) ;
		static void modifyElementGaussStimulus(const std::shared_ptr<element::Element>& element);
		static void modifyElementFieldCoupling(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementMexicanHatKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementNormalNoise(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussFieldCoupling(const std::shared_ptr<element::Element>& element);
		static void modifyElementOscillatoryKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementAsymmetricGaussKernel(const std::shared_ptr<element::Element>& element);
		static ImVec4 getColorForElementType(element::ElementLabel label);
		static std::string getIconForElementType(element::ElementLabel label);
		static std::string getElementTypeDisplayName(element::ElementLabel label);
	};
}
