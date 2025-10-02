#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/gauss_kernel.h"
#include "elements/gauss_stimulus.h"
#include "elements/neural_field.h"
#include "elements/normal_noise.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/gauss_field_coupling.h"
#include "elements/field_coupling.h"
#include "elements/oscillatory_kernel.h"
#include "widgets.h"

namespace dnf_composer::user_interface
{
	inline ImU32 getHeaderColorForElementType(element::ElementLabel label)
	{
		switch (label) {
		case element::ElementLabel::NEURAL_FIELD:
			return IM_COL32(86, 128, 191, 255);   // Soft Blue
		case element::ElementLabel::NORMAL_NOISE:
			return IM_COL32(223, 148, 84, 255);   // Warm Orange
		case element::ElementLabel::GAUSS_KERNEL:
			return IM_COL32(191, 63, 63, 255);    // Muted Red
		case element::ElementLabel::GAUSS_STIMULUS:
			return IM_COL32(127, 191, 127, 255);  // Sage Green
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
			return IM_COL32(154, 121, 191, 255);  // Lavender
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			return IM_COL32(165, 102, 71, 255);   // Warm Brown
		case element::ElementLabel::FIELD_COUPLING:
			return IM_COL32(212, 192, 121, 255);  // Cream Gold
		case element::ElementLabel::OSCILLATORY_KERNEL:
			return IM_COL32(175, 133, 187, 255);  // Dusty Rose
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
			return IM_COL32(148, 178, 182, 255);  // Soft Teal
		default:
			return IM_COL32(127, 127, 127, 255);  // Neutral Gray
		}
	}

	class NodeGraphWindow : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		ImNodeEditor::Config config;
		ImNodeEditor::EditorContext* context;
		static constexpr uint16_t startingInputPinId = 1000;
		static constexpr uint16_t startingOutputPinId = 2000;
		static constexpr uint16_t startingLinkId = 3000;
	public:
		explicit NodeGraphWindow(const std::shared_ptr<Simulation>& simulation);

		NodeGraphWindow(const NodeGraphWindow&) = delete;
		NodeGraphWindow& operator=(const NodeGraphWindow&) = delete;
		NodeGraphWindow(NodeGraphWindow&&) = delete;
		NodeGraphWindow& operator=(NodeGraphWindow&&) = delete;

		void render() override;
		~NodeGraphWindow() override = default;
	private:
		void renderElementNodes() const;
		static void setNodeStyle(const std::shared_ptr<element::Element>& element);
		static void renderElementNode(const std::shared_ptr<element::Element>& element);
		static void renderElementNodeHeader(const std::shared_ptr<element::Element>& element);
		static void renderElementCommonParameters(const std::shared_ptr<element::Element>& element);
		static void renderElementSpecificParameters(const std::shared_ptr<element::Element>& element);
		static void renderElementPins(const std::shared_ptr<element::Element>& element);
		static void renderElementNodeConnections(const std::shared_ptr<element::Element>& element);
		void handleInteractions() const;
		void handlePinInteractions() const;
		void handleLinkInteractions() const;
		static size_t getNodeId(const std::shared_ptr<element::Element>& element);
	};
}
