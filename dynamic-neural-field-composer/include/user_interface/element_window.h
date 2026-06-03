#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/gauss_stimulus.h"
#include "user_interface/widgets.h"
#include "application/application.h"

namespace dnf_composer::user_interface
{
	struct PanelScope
	{
		ImRect rect;          // fixed rectangle of the panel (screen coords)
		float  ui{};
		ImU32  fill{}, border{};
		float  rounding{};
		ImVec2 pad{};
	};

	class ElementWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		static std::shared_ptr<element::Element> focusedElement;
	public:
		explicit ElementWindow(const std::shared_ptr<Simulation>& simulation);

		ElementWindow(const ElementWindow&) = delete;
		ElementWindow& operator=(const ElementWindow&) = delete;
		ElementWindow(ElementWindow&&) = delete;
		ElementWindow& operator=(ElementWindow&&) = delete;

		void render() override;
		void renderElementControlCard();
		void renderModifyElementParameters();
		static void switchElementToModify(const std::shared_ptr<element::Element>& element, const std::string& simId = {});
		static void setFocusedElement(const std::shared_ptr<element::Element>& element);
		~ElementWindow() override = default;
	private:
		void renderIdentifiersSection(const std::shared_ptr<element::Element>& element) const;
		void renderDimensionControls(const std::shared_ptr<element::Element>& element) const;
		void renderDimensionControls2D(const std::shared_ptr<element::Element>& element) const;
		static void modifyElementNeuralField(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussStimulus(const std::shared_ptr<element::Element>& element);
		static void modifyElementFieldCoupling(const std::shared_ptr<element::Element>& element, const std::string& simId = {});
		static void modifyElementGaussKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementMexicanHatKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementNormalNoise(const std::shared_ptr<element::Element>& element);
		static void modifyElementCorrelatedNormalNoise(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussFieldCoupling(const std::shared_ptr<element::Element>& element);
		static void modifyElementOscillatoryKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementAsymmetricGaussKernel(const std::shared_ptr<element::Element>& element);
		static void modifyElementBoostStimulus(const std::shared_ptr<element::Element>& element);
		static void modifyElementMemoryTrace(const std::shared_ptr<element::Element>& element);
		static void modifyElementNeuralField2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussStimulus2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementGaussKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementMexicanHatKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementNormalNoise2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementOscillatoryKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementTimedGaussStimulus(const std::shared_ptr<element::Element>& element);
		static void modifyElementTimedGaussStimulus2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementBoostStimulus2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementCorrelatedNormalNoise2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementAsymmetricGaussKernel2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementMemoryTrace2D(const std::shared_ptr<element::Element>& element);
		static void modifyElementResize(const std::shared_ptr<element::Element>& element);
		static void modifyElementResize2D(const std::shared_ptr<element::Element>& element);
		static ImVec4 getColorForElementType(element::ElementLabel label);
		static std::string getElementTypeDisplayName(element::ElementLabel label);
		static PanelScope beginElementPanel(const ImVec4& baseColor, const ImVec2& size);
		static void endElementPanel(const PanelScope& p);
	};
}
