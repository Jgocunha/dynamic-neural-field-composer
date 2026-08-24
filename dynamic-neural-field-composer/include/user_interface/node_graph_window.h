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
#include "user_interface/colour_registry.h"


namespace dnf_composer::user_interface
{
	inline ImU32 getHeaderColorForElementType(const element::ElementLabel label)
	{
		switch (label) {
		case element::ElementLabel::NEURAL_FIELD:
			return colour::kNeuralFieldElement;
		case element::ElementLabel::NORMAL_NOISE:
			return colour::kNormalNoiseElement;
		case element::ElementLabel::CORRELATED_NORMAL_NOISE:
			return colour::kCorrelatedNormalNoiseElement;
		case element::ElementLabel::GAUSS_KERNEL:
			return colour::kGaussKernelElement;
		case element::ElementLabel::GAUSS_STIMULUS:
			return colour::kGaussStimulusElement;
		case element::ElementLabel::MEXICAN_HAT_KERNEL:
			return colour::kMexicanHatKernelElement;
		case element::ElementLabel::GAUSS_FIELD_COUPLING:
			return colour::kGaussFieldCouplingElement;
		case element::ElementLabel::FIELD_COUPLING:
			return colour::kFieldCouplingElement;
		case element::ElementLabel::OSCILLATORY_KERNEL:
			return colour::kOscillatoryKernelElement;
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL:
			return colour::kAsymmetricGaussKernelElement;
		case element::ElementLabel::BOOST_STIMULUS:
			return colour::kBoostStimulusElement;
		case element::ElementLabel::MEMORY_TRACE:
			return colour::kMemoryTraceElement;
		case element::ElementLabel::NEURAL_FIELD_2D:
			return colour::kNeuralField2DElement;
		case element::ElementLabel::GAUSS_STIMULUS_2D:
			return colour::kGaussStimulus2DElement;
		case element::ElementLabel::GAUSS_KERNEL_2D:
			return colour::kGaussKernel2DElement;
		case element::ElementLabel::MEXICAN_HAT_KERNEL_2D:
			return colour::kMexicanHatKernel2DElement;
		case element::ElementLabel::NORMAL_NOISE_2D:
			return colour::kNormalNoise2DElement;
		case element::ElementLabel::OSCILLATORY_KERNEL_2D:
			return colour::kOscillatoryKernel2DElement;
		case element::ElementLabel::TIMED_GAUSS_STIMULUS:
			return colour::kTimedGaussStimulusElement;
		case element::ElementLabel::TIMED_GAUSS_STIMULUS_2D:
			return colour::kTimedGaussStimulus2DElement;
		case element::ElementLabel::BOOST_STIMULUS_2D:
			return colour::kBoostStimulus2DElement;
		case element::ElementLabel::CORRELATED_NORMAL_NOISE_2D:
			return colour::kCorrelatedNormalNoise2DElement;
		case element::ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D:
			return colour::kAsymmetricGaussKernel2DElement;
		case element::ElementLabel::MEMORY_TRACE_2D:
			return colour::kMemoryTrace2DElement;
		default:
			return colour::kUnknownElement;
		}
	}

	/// @brief Per-node floating plot card state (position, size, axis limits, heatmap options).
	struct PlotCardState
	{
		bool   isFirstFrame   = true; ///< True until the card's initial position has been applied once.
		ImVec2 initialPos; ///< Position the card is placed at on its first frame.
		ImVec2 size           = { 540.0F, 620.0F }; ///< Current card size.
		bool   autoFit        = true; ///< Whether axis limits auto-fit the plotted data.
		float  xMin = 0.F;    ///< Lower x axis limit, used when @c autoFit is false.
		float  xMax = 100.F;  ///< Upper x axis limit, used when @c autoFit is false.
		float  yMin = -20.F;  ///< Lower y axis limit, used when @c autoFit is false.
		float  yMax = 20.F;   ///< Upper y axis limit, used when @c autoFit is false.
		float  xStep          = 1.0F; ///< Spatial step between samples along the x axis.
		float  lineThickness  = 2.5F; ///< Line plot stroke width.
		std::array<char, 128> title  = {""}; ///< Editable plot title buffer.
		std::array<char, 64>  xLabel = {"Spatial location"}; ///< Editable x-axis label buffer.
		std::array<char, 64>  yLabel = {"Amplitude"}; ///< Editable y-axis label buffer.
		// 2D heatmap options
		int    colormap       = ImPlotColormap_Deep; ///< ImPlot colormap used for 2D field/weight heatmaps.
		float  scaleMin       = -20.0F; ///< Heatmap colour scale lower bound, used when @c autoScale is false.
		float  scaleMax       =  20.0F; ///< Heatmap colour scale upper bound, used when @c autoScale is false.
		bool   autoScale      = true; ///< Whether the heatmap colour scale auto-fits the plotted data.
		std::array<char, 64>  selectedComponent  = {""}; ///< Name of the field component currently plotted.
		std::array<char, 64>  autoTitleComponent = {""}; ///< Component name the auto-generated title was last derived from.
	};

	/// @brief Deleter that returns an imgui-node-editor context to the library.
	///
	/// Lets the context be held in a unique_ptr so it is released on every exit
	/// path, rather than relying on a destructor body remembering to do it (#115).
	struct EditorContextDeleter
	{
		/// @brief Destroy the editor context, if any.
		/// @param ctx Context to destroy, or @c nullptr.
		void operator()(ImNodeEditor::EditorContext* ctx) const noexcept
		{
			if (ctx != nullptr) { ImNodeEditor::DestroyEditor(ctx); }
		}
	};

	/// @brief Node-editor window: renders a simulation's elements and connections as a graph.
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
			/// @brief Which side/role of an element a pin id encodes.
			enum class Kind { Input, Target, Output, Activation, Invalid };

			static constexpr uint64_t pinKindStride  = 1'000'000ULL; ///< Size of each kind's disjoint id band; must exceed any element uid.
			static constexpr uint64_t inputPinBase      = 1ULL * pinKindStride; ///< First id in the Input-pin band.
			static constexpr uint64_t targetPinBase     = 2ULL * pinKindStride; ///< First id in the Target-pin band.
			static constexpr uint64_t outputPinBase     = 3ULL * pinKindStride; ///< First id in the Output-pin band.
			static constexpr uint64_t activationPinBase = 4ULL * pinKindStride; ///< First id in the Activation-pin band.
			static constexpr uint64_t linkIdBase        = 5ULL * pinKindStride; ///< First id in the link-id band.

			/// @brief Encode the input pin id for an element.
			/// @param uid Element uid.
			/// @return The pin id.
			static uint64_t inputPin(int uid)      { return inputPinBase      + static_cast<uint64_t>(uid); }
			/// @brief Encode the target pin id for an element.
			/// @param uid Element uid.
			/// @return The pin id.
			static uint64_t targetPin(int uid)     { return targetPinBase     + static_cast<uint64_t>(uid); }
			/// @brief Encode the output pin id for an element.
			/// @param uid Element uid.
			/// @return The pin id.
			static uint64_t outputPin(int uid)     { return outputPinBase     + static_cast<uint64_t>(uid); }
			/// @brief Encode the activation pin id for an element.
			/// @param uid Element uid.
			/// @return The pin id.
			static uint64_t activationPin(int uid) { return activationPinBase + static_cast<uint64_t>(uid); }

			/// @brief Distinguishes links that differ only in which pin they start/end
			/// at (e.g. Output-to-Input vs Activation-to-Input between the same pair
			/// of elements), so they never share an id.
			/// @param srcUid           Uid of the source element.
			/// @param dstUid           Uid of the destination element.
			/// @param isTargetSlot     True if the link ends at a Target pin (vs an Input pin).
			/// @param isFromActivation True if the link starts at an Activation pin (vs an Output pin).
			/// @return The link id.
			static uint64_t linkId(int srcUid, int dstUid, bool isTargetSlot, bool isFromActivation = false)
			{
				return linkIdBase + (static_cast<uint64_t>(srcUid) << 32)
					+ (static_cast<uint64_t>(dstUid) << 2)
					+ (isTargetSlot ? 1ULL : 0ULL) + (isFromActivation ? 2ULL : 0ULL);
			}

			/// @brief Result of decoding a raw pin id.
			struct Decoded
			{
				Kind kind; ///< Pin kind the id belongs to (or @c Kind::Invalid).
				int  uid;  ///< Element uid encoded in the id (or -1 if invalid).
			};

			/// @brief Decode a raw pin id back into its kind and element uid.
			/// Returns Kind::Invalid for an id outside any known band (e.g. a
			/// stale id from a since-removed encoding).
			/// @param rawPinId Pin id to decode.
			/// @return The decoded kind and element uid.
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
		/// @brief Construct the node graph window for a simulation.
		/// @param simulation Simulation whose elements and connections are graphed.
		explicit NodeGraphWindow(const std::shared_ptr<Simulation>& simulation);

		NodeGraphWindow(const NodeGraphWindow&) = delete;
		NodeGraphWindow& operator=(const NodeGraphWindow&) = delete;
		NodeGraphWindow(NodeGraphWindow&&) = delete;
		NodeGraphWindow& operator=(NodeGraphWindow&&) = delete;

		/// @brief Draw the node graph window for this frame.
		void render() override;
		/// @brief Draw the node graph inside the caller's own ImGui window/child region,
		/// instead of opening its own top-level window.
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

		/// @brief Stops newly constructed windows persisting their node layout to disk.
		///
		/// The node editor writes its layout to `imnode-window.json` resolved against the
		/// process working directory, so a test binary constructing this window drops that
		/// file into whatever directory it was run from. Call this once before constructing
		/// any window in a test process; it affects only windows created afterwards, and
		/// has no use in application code, where persisting the layout is the point.
		static void disableLayoutPersistenceForTesting();

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
