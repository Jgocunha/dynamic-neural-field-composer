#pragma once

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <implot.h>
#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/gauss_kernel.h"
#include "elements/gauss_stimulus.h"
#include "elements/neural_field.h"
#include "elements/normal_noise.h"
#include "elements/correlated_normal_noise.h"
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
#include "elements/boost_stimulus_2d.h"
#include "elements/memory_trace_2d.h"
#include "elements/asymmetric_gauss_kernel_2d.h"
#include "widgets.h"
#include "user_interface/node_utilities/builders.h"
#include "user_interface/node_utilities/node_widgets.h"
#include "application/application.h"
#include "user_interface/element_window.h"


namespace dnf_composer::user_interface
{
	inline ImU32 getHeaderColorForElementType(const element::ElementLabel label)
	{
		switch (label) {
		case element::ElementLabel::NEURAL_FIELD:
			return IM_COL32(86, 128, 191, 255);   // Soft Blue
		case element::ElementLabel::NORMAL_NOISE:
			return IM_COL32(223, 148, 84, 255);   // Warm Orange
		case element::ElementLabel::CORRELATED_NORMAL_NOISE:
			return IM_COL32(210, 110, 60, 255);   // Deep Orange
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
		case element::ElementLabel::BOOST_STIMULUS_2D:
			return IM_COL32(210, 182, 72, 255);   // Deeper Warm Yellow
		case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D:
			return IM_COL32(182, 109, 44, 255);   // Deeper Deep Orange
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D:
			return IM_COL32(129, 155, 159, 255);  // Deeper Soft Teal
		case element::ElementLabel::MEMORY_TRACE_2D:
			return IM_COL32(96, 139, 122, 255);   // Deeper Sage Green
		default:
			return IM_COL32(127, 127, 127, 255);  // Neutral Gray
		}
	}

	struct PlotCardState
	{
		bool   isFirstFrame   = true;
		ImVec2 initialPos;
		ImVec2 size           = { 540.0F, 620.0F };
		bool   autoFit        = true;
		float  xMin = 0.F, xMax = 100.F, yMin = -20.F, yMax = 20.F;
		float  xStep          = 1.0F;
		float  lineThickness  = 2.5F;
		std::array<char, 128> title  = {""};
		std::array<char, 64>  xLabel = {"Spatial location"};
		std::array<char, 64>  yLabel = {"Amplitude"};
		// 2D heatmap options
		int    colormap       = ImPlotColormap_Deep;
		float  scaleMin       = -20.0F;
		float  scaleMax       =  20.0F;
		bool   autoScale      = true;
		std::array<char, 64>  selectedComponent  = {""};
		std::array<char, 64>  autoTitleComponent = {""};
	};

	/// @brief Deleter that returns an imgui-node-editor context to the library.
	///
	/// Lets the context be held in a unique_ptr so it is released on every exit
	/// path, rather than relying on a destructor body remembering to do it (#115).
	struct EditorContextDeleter
	{
		void operator()(ImNodeEditor::EditorContext* ctx) const noexcept
		{
			if (ctx != nullptr) { ImNodeEditor::DestroyEditor(ctx); }
		}
	};

	class NodeGraphWindow final : public imgui_kit::UserInterfaceWindow
	{
	public:
		/// @brief Encodes/decodes node-editor pin and link ids.
		///
		/// Pin ids used to be additive (`1000 + uid`, `2000 + uid`, ...) with a
		/// never-reset global element uid counter -- so any element reaching
		/// uid >= 1000 collided with a pin of a lower-uid element (e.g. input pin
		/// of uid 1000 == output pin of uid 0). The link id formula
		/// (`stoull(to_string(dst)+to_string(src))`) had the same problem for
		/// multi-digit uids: `stoull("11"+"1") == stoull("1"+"11")`.
		///
		/// Replaced with a multiplicative kind stride: each pin kind owns a
		/// disjoint band `[kind * pinKindStride, (kind+1) * pinKindStride)`, so
		/// any uid below the stride can never collide across kinds, and link ids
		/// are packed by bit-shifting the two uids rather than string-concatenating
		/// their decimal digits.
		///
		/// Exposed publicly (not just as private statics) so this encoding can be
		/// unit-tested directly -- see tests/user_interface/test_node_graph_window_pin_ids.cpp.
		struct PinIdEncoding
		{
			// Input/Target are input-side (sink) pins; Output/Activation are
			// output-side (source) pins. A link's destination component name is
			// derived from the sink pin's kind ("target" vs "output"); its source
			// component name is derived from the source pin's kind ("activation"
			// vs "output") -- see NodeGraphWindow::handlePinInteractions().
			enum class Kind { Input, Target, Output, Activation, Invalid };

			static constexpr uint64_t pinKindStride  = 1'000'000ULL;
			static constexpr uint64_t inputPinBase      = 1ULL * pinKindStride;
			static constexpr uint64_t targetPinBase     = 2ULL * pinKindStride;
			static constexpr uint64_t outputPinBase     = 3ULL * pinKindStride;
			static constexpr uint64_t activationPinBase = 4ULL * pinKindStride;
			static constexpr uint64_t linkIdBase        = 5ULL * pinKindStride;

			static uint64_t inputPin(int uid)      { return inputPinBase      + static_cast<uint64_t>(uid); }
			static uint64_t targetPin(int uid)     { return targetPinBase     + static_cast<uint64_t>(uid); }
			static uint64_t outputPin(int uid)     { return outputPinBase     + static_cast<uint64_t>(uid); }
			static uint64_t activationPin(int uid) { return activationPinBase + static_cast<uint64_t>(uid); }

			/// @brief Distinguishes links that differ only in which pin they start/end
			/// at (e.g. Output-to-Input vs Activation-to-Input between the same pair
			/// of elements), so they never share an id.
			static uint64_t linkId(int srcUid, int dstUid, bool isTargetSlot, bool isFromActivation = false)
			{
				return linkIdBase + (static_cast<uint64_t>(srcUid) << 32)
					+ (static_cast<uint64_t>(dstUid) << 2)
					+ (isTargetSlot ? 1ULL : 0ULL) + (isFromActivation ? 2ULL : 0ULL);
			}

			struct Decoded { Kind kind; int uid; };

			/// @brief Decode a raw pin id back into its kind and element uid.
			/// Returns Kind::Invalid for an id outside any known band (e.g. a
			/// stale id from a since-removed encoding).
			static Decoded decode(uint64_t rawPinId)
			{
				const uint64_t band = (rawPinId / pinKindStride) * pinKindStride;
				const int uid = static_cast<int>(rawPinId % pinKindStride);
				if (band == inputPinBase)      { return { Kind::Input,      uid }; }
				if (band == targetPinBase)     { return { Kind::Target,     uid }; }
				if (band == outputPinBase)     { return { Kind::Output,     uid }; }
				if (band == activationPinBase) { return { Kind::Activation, uid }; }
				return { Kind::Invalid, -1 };
			}
		};

	private:
		std::shared_ptr<Simulation> simulation;
		// config must outlive context: CreateEditor() keeps the pointer we hand it,
		// and members are destroyed in reverse declaration order, so config must be
		// declared first.
		ImNodeEditor::Config config;
		std::unique_ptr<ImNodeEditor::EditorContext, EditorContextDeleter> context;

		// Initial-layout state: nodes not yet seen in this session get a grid position
		// on the frame after their first render (when we can read their actual position).
		mutable std::unordered_set<size_t>          positionedNodeIds;
		mutable std::unordered_map<size_t, ImVec2>  pendingInitialPositions;

		// Per-node floating plot cards (toggled by double-click).
		mutable std::unordered_map<size_t, PlotCardState> plotCards;

		// Node graph panel bounds (updated every frame) for plot card clamping.
		mutable ImVec2 ngBoundsMin;
		mutable ImVec2 ngBoundsMax;

		// Mini-map cache (filled each frame inside Begin/End while editor context is active).
		mutable std::vector<std::pair<ImVec2, ImVec2>> cachedNodeRects;
		mutable std::vector<element::ElementLabel>     cachedNodeLabels;
		mutable std::vector<size_t>                    cachedNodeIds;
		mutable ImVec2                                 cachedVpMin;
		mutable ImVec2                                 cachedVpMax;

		// Overlap prevention: baseline positions and drag-start positions for snap-on-drop.
		mutable std::unordered_map<size_t, ImVec2> prevNodePositions;
		mutable std::unordered_map<size_t, ImVec2> dragStartPositions;
	public:
		explicit NodeGraphWindow(const std::shared_ptr<Simulation>& simulation);

		NodeGraphWindow(const NodeGraphWindow&) = delete;
		NodeGraphWindow& operator=(const NodeGraphWindow&) = delete;
		NodeGraphWindow(NodeGraphWindow&&) = delete;
		NodeGraphWindow& operator=(NodeGraphWindow&&) = delete;

		void render() override;
		void renderEmbedded() const;

		/// @brief Clear the window's cross-frame UI caches (hover timers, EMA-smoothed
		/// colormap ranges, and any half-finished click-to-click connection).
		///
		/// These caches are keyed by node id and element name and intentionally
		/// outlive a single frame, so in a test process two tests that reuse an
		/// element name would otherwise share smoothed state and become
		/// order-dependent. Call this between tests; it has no use in application
		/// code, where the caches are meant to persist.
		static void resetTransientStateForTesting();

		~NodeGraphWindow() override = default;
	private:
		void renderGraphContent() const;
		void renderElementNodes() const;
		static void renderElementNode(const std::shared_ptr<element::Element>& element);
		static void renderElementNodeConnections(const std::shared_ptr<element::Element>& element);
		void handleInteractions() const;
		void handlePinInteractions() const;
		void handleLinkInteractions() const;
		void handleNodeSelection() const;
		void renderNodePlotCards() const;
		static size_t getNodeId(const std::shared_ptr<element::Element>& element);
		static int    getColumnForElement(element::ElementLabel label);
		static void applyCanvasStyle();
		static void restoreCanvasStyle();
		static void renderElementTooltip(const std::shared_ptr<element::Element>& element);
		static bool isWeightMapElement(element::ElementLabel label);
		static void drawWeightHeatmap(ImDrawList* dl, ImRect rect, const std::vector<double>& weights,
			int rows, int cols);
		static void draw2DFieldHeatmap(ImDrawList* dl, ImRect rect, const std::vector<double>& data,
			int rows, int cols, double wMin, double wMax, int colormap = ImPlotColormap_Deep);
		static void drawInlineHeatmapAxes(ImDrawList* dl, const ImRect& hmRect, int rows, int cols,
			double dMin, double dMax, int colormap = ImPlotColormap_Deep);
		/// @brief Space (px) to reserve to the right of the inline node preview's
		/// heatmap square for its colorbar strip, ticks, and value labels -- sized
		/// from the widest label selectHeatmapTickFormat() will draw for [dMin,
		/// dMax], so a narrow (e.g. DELTA weight matrix) range doesn't clip its
		/// more precise labels the way a fixed constant did.
		static float inlineColorbarWidth(double dMin, double dMax);
		static void renderNodeScrollingName(const std::shared_ptr<element::Element>& element, float minNodeSize);
		static void renderNodeInlinePreview(const std::shared_ptr<element::Element>& element, float minNodeSize);
		static void renderNodePins(const std::shared_ptr<element::Element>& element, float minNodeSize);
		static void renderPlotCardMenuBar(PlotCardState& state, bool is2DField,
			const std::shared_ptr<element::Element>& element, bool isWM = false);
		static void renderPlotCardContent(const std::shared_ptr<element::Element>& element, PlotCardState& state,
			bool isWM, bool is2DField);
		void renderNavigationControls(ImVec2 winPos, ImVec2 winSize) const;
		void renderMiniMap(ImVec2 winPos, ImVec2 winSize) const;
	};
}
