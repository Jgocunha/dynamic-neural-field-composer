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
#include "user_interface/widgets.h"

namespace dnf_composer::user_interface
{
	struct PanelScope {
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
		 // Visual helpers for the per-element rounded panel

	public:
		explicit ElementWindow(const std::shared_ptr<Simulation>& simulation);

		ElementWindow(const ElementWindow&) = delete;
		ElementWindow& operator=(const ElementWindow&) = delete;
		ElementWindow(ElementWindow&&) = delete;
		ElementWindow& operator=(ElementWindow&&) = delete;

		void render() override;
		void renderElementControlCard() const;
		void renderModifyElementParameters() const;
		~ElementWindow() override = default;
	private:
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

		// Draw background first, then place the cursor inside for content
		static PanelScope beginElementPanel(const ImVec4& baseColor, const ImVec2& size)
		{
			PanelScope p{};
			p.ui       = ImGui::GetIO().FontGlobalScale;
			p.rounding = 12.0f * p.ui;
			p.pad      = ImVec2(10.0f * p.ui, 8.0f * p.ui);

			// soft fill/border from base color
			p.fill   = ImGui::GetColorU32(ImVec4(baseColor.x, baseColor.y, baseColor.z, 0.18f));
			p.border = ImGui::GetColorU32(ImVec4(baseColor.x, baseColor.y, baseColor.z, 0.35f));

			// fixed rect from current cursor (screen coords)
			const ImVec2 topLeft = ImGui::GetCursorScreenPos();
			const ImVec2 bottomRight = ImVec2(topLeft.x + size.x, topLeft.y + size.y);
			p.rect = ImRect(topLeft, bottomRight);

			// draw the panel *behind* content
			ImDrawList* dl = ImGui::GetWindowDrawList();
			dl->AddRectFilled(p.rect.Min, p.rect.Max, p.fill, p.rounding);
			dl->AddRect      (p.rect.Min, p.rect.Max, p.border, p.rounding, 0, 1.5f * p.ui);

			// move cursor inside, honoring padding, then start the content group
			const ImVec2 innerPos = ImVec2(p.rect.Min.x + p.pad.x, p.rect.Min.y + p.pad.y);
			ImGui::SetCursorScreenPos(innerPos);
			ImGui::BeginGroup();
			return p;
		}

		static void endElementPanel(const PanelScope& p)
		{
			ImGui::EndGroup();
			// advance cursor to just below the panel, keeping normal flow
			ImGui::SetCursorScreenPos(ImVec2(p.rect.Min.x, p.rect.Max.y + 6.0f * p.ui));
		}
	};
}
