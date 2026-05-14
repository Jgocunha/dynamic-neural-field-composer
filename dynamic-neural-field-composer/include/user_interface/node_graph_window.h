#pragma once

#include <unordered_map>
#include <unordered_set>
#include <implot.h>
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
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/boost_stimulus.h"
#include "elements/memory_trace.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/normal_noise_2d.h"
#include "elements/oscillatory_kernel_2d.h"
#include "elements/timed_gauss_stimulus.h"
#include "elements/timed_gauss_stimulus_2d.h"
#include "widgets.h"
#include "user_interface/node_utilities/builders.h"
#include "user_interface/node_utilities/node_widgets.h"
#include "application/application.h"
#include "user_interface/element_window.h"


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
		case element::ElementLabel::BOOST_STIMULUS:
			return IM_COL32(242, 209, 83, 255);   // Warm Yellow
		case element::ElementLabel::MEMORY_TRACE:
			return IM_COL32(110, 160, 140, 255);  // Sage Green
		case element::ElementLabel::NEURAL_FIELD_2D:
			return IM_COL32(70, 110, 175, 255);   // Deeper Blue
		case element::ElementLabel::GAUSS_STIMULUS_2D:
			return IM_COL32(105, 175, 105, 255);  // Deeper Sage Green
		case element::ElementLabel::GAUSS_KERNEL_2D:
			return IM_COL32(175, 48, 48, 255);    // Deeper Muted Red
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
			return IM_COL32(138, 105, 175, 255);  // Deeper Lavender
		case element::ElementLabel::NORMAL_NOISE_2D:
			return IM_COL32(207, 132, 68, 255);   // Deeper Warm Orange
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
			return IM_COL32(152, 116, 163, 255);  // Deeper Dusty Rose
		case element::ElementLabel::TIMED_GAUSS_STIMULUS:
			return IM_COL32(97, 161, 97, 255);    // Darker Sage Green
		case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:
			return IM_COL32(80, 133, 80, 255);    // Deepest Sage Green
		default:
			return IM_COL32(127, 127, 127, 255);  // Neutral Gray
		}
	}

	class NodeGraphWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation> simulation;
		ImNodeEditor::Config config;
		ImNodeEditor::EditorContext* context;
		static constexpr uint16_t startingInputPinId = 1000;
		static constexpr uint16_t startingOutputPinId = 2000;
		static constexpr uint16_t startingLinkId = 3000;

		// Initial-layout state: nodes not yet seen in this session get a grid position
		// on the frame after their first render (when we can read their actual position).
		mutable std::unordered_set<size_t>          positionedNodeIds_;
		mutable std::unordered_map<size_t, ImVec2>  pendingInitialPositions_;

		// Node inspector panel state.
		mutable size_t  selectedNodeId_ = 0;
		mutable ImVec2  inspectorPos_   = {};
		mutable ImVec2  inspectorSize_  = {};
	public:
		explicit NodeGraphWindow(const std::shared_ptr<Simulation>& simulation);

		NodeGraphWindow(const NodeGraphWindow&) = delete;
		NodeGraphWindow& operator=(const NodeGraphWindow&) = delete;
		NodeGraphWindow(NodeGraphWindow&&) = delete;
		NodeGraphWindow& operator=(NodeGraphWindow&&) = delete;

		void render() override;
		void renderGraph() const;   // embedded variant (no Begin/End window)
		~NodeGraphWindow() override = default;
	private:
		void renderElementNodes() const;
		static void renderElementNode(const std::shared_ptr<element::Element>& element);
		static void renderElementNodeConnections(const std::shared_ptr<element::Element>& element);
		void handleInteractions() const;
		void handlePinInteractions() const;
		void handleLinkInteractions() const;
		void handleNodeSelection() const;
		void renderNodeInspectorPopup() const;
		static size_t getNodeId(const std::shared_ptr<element::Element>& element);
		static int    getColumnForElement(element::ElementLabel label);
		static void applyCanvasStyle();
		static void restoreCanvasStyle();
		static void renderElementTooltip(const std::shared_ptr<element::Element>& element);
	};
}
