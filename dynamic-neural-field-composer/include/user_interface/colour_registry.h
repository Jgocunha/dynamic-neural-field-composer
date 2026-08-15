#pragma once

#include <imgui.h>

/// @file colour_registry.h
/// @brief Named, semantic GUI colour constants shared across the user-interface layer.
///
/// Before this registry, colour literals (`ImVec4(...)`, `IM_COL32(...)`) were re-spelled
/// at every call site across `node_graph_window`, `simulation_window`, and `element_window`,
/// so the "same" colour could silently drift between windows and a theme change meant
/// hunting literals file by file (#28).
///
/// Constants are named by **role**, not by hue (`kDestructiveText`, not `kRed`) -- a registry
/// of `kOrange`/`kLightBlue` would just move the magic-number problem, not remove it. Group
/// comments below describe where each family of constants is used; the call sites themselves
/// are unchanged in behaviour, only in where the literal lives.
namespace dnf_composer::user_interface::colour
{
	// ---------------------------------------------------------------------------------------
	// Node graph: per-element-type header colours (NodeGraphWindow::getHeaderColorForElementType)
	// ---------------------------------------------------------------------------------------
	inline constexpr ImU32 kNeuralFieldHeader              = IM_COL32(86,  128, 191, 255); // Soft Blue
	inline constexpr ImU32 kNormalNoiseHeader               = IM_COL32(223, 148, 84,  255); // Warm Orange
	inline constexpr ImU32 kCorrelatedNormalNoiseHeader     = IM_COL32(210, 110, 60,  255); // Deep Orange
	inline constexpr ImU32 kGaussKernelHeader                = IM_COL32(191, 63,  63,  255); // Muted Red
	inline constexpr ImU32 kGaussStimulusHeader              = IM_COL32(127, 191, 127, 255); // Sage Green
	inline constexpr ImU32 kMexicanHatKernelHeader           = IM_COL32(154, 121, 191, 255); // Lavender
	inline constexpr ImU32 kGaussFieldCouplingHeader         = IM_COL32(165, 102, 71,  255); // Warm Brown
	inline constexpr ImU32 kFieldCouplingHeader              = IM_COL32(212, 192, 121, 255); // Cream Gold
	inline constexpr ImU32 kOscillatoryKernelHeader          = IM_COL32(175, 133, 187, 255); // Dusty Rose
	inline constexpr ImU32 kAsymmetricGaussKernelHeader      = IM_COL32(148, 178, 182, 255); // Soft Teal
	inline constexpr ImU32 kBoostStimulusHeader              = IM_COL32(242, 209, 83,  255); // Warm Yellow
	inline constexpr ImU32 kMemoryTraceHeader                = IM_COL32(110, 160, 140, 255); // Sage Green
	inline constexpr ImU32 kNeuralField2DHeader              = IM_COL32(70,  110, 175, 255); // Deeper Blue
	inline constexpr ImU32 kGaussStimulus2DHeader            = IM_COL32(105, 175, 105, 255); // Deeper Sage Green
	inline constexpr ImU32 kGaussKernel2DHeader              = IM_COL32(175, 48,  48,  255); // Deeper Muted Red
	inline constexpr ImU32 kMexicanHatKernel2DHeader         = IM_COL32(138, 105, 175, 255); // Deeper Lavender
	inline constexpr ImU32 kNormalNoise2DHeader              = IM_COL32(207, 132, 68,  255); // Deeper Warm Orange
	inline constexpr ImU32 kOscillatoryKernel2DHeader        = IM_COL32(152, 116, 163, 255); // Deeper Dusty Rose
	inline constexpr ImU32 kTimedGaussStimulusHeader         = IM_COL32(97,  161, 97,  255); // Darker Sage Green
	inline constexpr ImU32 kTimedGaussStimulus2DHeader       = IM_COL32(80,  133, 80,  255); // Deepest Sage Green
	inline constexpr ImU32 kBoostStimulus2DHeader            = IM_COL32(210, 182, 72,  255); // Deeper Warm Yellow
	inline constexpr ImU32 kCorrelatedNormalNoise2DHeader    = IM_COL32(182, 109, 44,  255); // Deeper Deep Orange
	inline constexpr ImU32 kAsymmetricGaussKernel2DHeader    = IM_COL32(129, 155, 159, 255); // Deeper Soft Teal
	inline constexpr ImU32 kMemoryTrace2DHeader              = IM_COL32(96,  139, 122, 255); // Deeper Sage Green
	inline constexpr ImU32 kUnknownElementHeader              = IM_COL32(127, 127, 127, 255); // Neutral Gray

	// ---------------------------------------------------------------------------------------
	// Node graph: canvas chrome, navigation overlay, and pin/link colours
	// ---------------------------------------------------------------------------------------
	inline constexpr ImVec4 kCanvasBackground      = ImVec4(0.94F, 0.95F, 0.96F, 1.00F);
	inline constexpr ImVec4 kCanvasGridLine        = ImVec4(0.80F, 0.82F, 0.85F, 0.60F);
	inline constexpr ImVec4 kFloatingPanelBackground = ImVec4(0.95F, 0.97F, 0.98F, 1.0F);

	inline constexpr ImVec4 kNavButtonBackground        = ImVec4(0.92F, 0.92F, 0.93F, 1.0F);
	inline constexpr ImVec4 kNavButtonHoveredBackground = ImVec4(0.80F, 0.84F, 0.95F, 1.0F);
	inline constexpr ImVec4 kNavButtonActiveBackground  = ImVec4(0.65F, 0.72F, 0.92F, 1.0F);

	/// @brief Element-side ("input") pin colour.
	inline constexpr ImVec4 kInputPinColour      = ImVec4(1.0F,  1.0F,  1.0F,  0.90F);
	/// @brief Cream-gold "target" pin colour -- matches FieldCoupling's header colour.
	inline constexpr ImVec4 kTargetPinColour     = ImVec4(0.83F, 0.75F, 0.47F, 0.95F);
	/// @brief Cool-blue "activation" pin colour -- contrasts with the target pin.
	inline constexpr ImVec4 kActivationPinColour = ImVec4(0.55F, 0.75F, 0.90F, 0.95F);
	/// @brief Inner fill for a pin's icon outline; transparent so only the outline shows.
	inline constexpr ImVec4 kPinIconInnerFill = ImVec4(0.0F, 0.0F, 0.0F, 0.0F);

	inline constexpr ImVec4 kLinkColour           = ImVec4(0.08F, 0.08F, 0.08F, 0.85F); // near-black
	inline constexpr ImVec4 kTargetLinkColour     = ImVec4(0.60F, 0.50F, 0.15F, 0.85F); // matches the Target pin
	inline constexpr ImVec4 kActivationLinkColour = ImVec4(0.30F, 0.50F, 0.65F, 0.85F); // matches the Activation pin

	/// @brief Alpha applied to per-series colormap colours in the multi-line plot preview.
	inline constexpr float kMultiSeriesLineAlpha = 0.86F;

	// ---------------------------------------------------------------------------------------
	// Node graph: inline heatmap preview (fill/border overlay, axis text/ticks) and minimap
	// ---------------------------------------------------------------------------------------
	inline constexpr ImU32 kHeatmapOverlayFill   = IM_COL32(255, 255, 255, 40);
	inline constexpr ImU32 kHeatmapOverlayBorder = IM_COL32(0,   0,   0,   30);
	inline constexpr ImU32 kHeatmapAxisText      = IM_COL32(40,  40,  40,  230);
	inline constexpr ImU32 kHeatmapAxisTick      = IM_COL32(80,  80,  80,  180);

	inline constexpr ImU32 kMinimapNodeBorder      = IM_COL32(255, 255, 255, 60);
	inline constexpr ImU32 kMinimapViewportBorder  = IM_COL32(255, 255, 255, 200);

	// ---------------------------------------------------------------------------------------
	// Shared accent-derived button states (element_window + simulation_window action buttons)
	// ---------------------------------------------------------------------------------------
	/// @brief Multiply an accent colour's RGB by this on hover (the common case).
	inline constexpr float kAccentHoverDarken  = 0.9F;
	/// @brief Multiply an accent colour's RGB by this while active/pressed (the common case).
	inline constexpr float kAccentActiveDarken = 0.8F;
	/// @brief Stronger darken used by the recording Start/Stop buttons on hover.
	inline constexpr float kAccentHoverDarkenStrong  = 0.85F;
	/// @brief Stronger darken used by the recording Start/Stop buttons while active/pressed.
	inline constexpr float kAccentActiveDarkenStrong = 0.70F;
	/// @brief Text colour on a solid accent-coloured button.
	inline constexpr ImVec4 kButtonTextOnAccent = ImVec4(1, 1, 1, 1);

	/// @brief Icon colour when its action is available.
	inline constexpr ImU32 kIconEnabled  = IM_COL32(255, 255, 255, 255);
	/// @brief Icon colour when its action is disabled (~39% opacity).
	inline constexpr ImU32 kIconDisabled = IM_COL32(255, 255, 255, 100);

	/// @brief Transparent background for a flat (borderless) icon button.
	inline constexpr ImVec4 kFlatButtonBackground     = ImVec4(0, 0, 0, 0);
	inline constexpr ImVec4 kFlatButtonHoverOverlay   = ImVec4(0, 0, 0, 0.06F);
	inline constexpr ImVec4 kFlatButtonActiveOverlay  = ImVec4(0, 0, 0, 0.12F);

	/// @brief Transparent child-window background, used to inset a panel without a visible fill.
	inline constexpr ImVec4 kTransparentChildBackground = ImVec4(0, 0, 0, 0);

	// ---------------------------------------------------------------------------------------
	// Destructive / error / recording-state colours
	// ---------------------------------------------------------------------------------------
	/// @brief Text colour for a destructive (remove/delete) icon button, element_window.
	inline constexpr ImVec4 kDestructiveText = ImVec4(0.85F, 0.25F, 0.25F, 1.0F);
	/// @brief Text colour for the add-element validation error message, simulation_window (#146).
	inline constexpr ImVec4 kValidationErrorText = ImVec4(0.90F, 0.35F, 0.35F, 1.0F);

	inline constexpr ImVec4 kDestructiveButtonHoverOverlay  = ImVec4(1.0F, 0.0F, 0.0F, 0.12F);
	inline constexpr ImVec4 kDestructiveButtonActiveOverlay = ImVec4(1.0F, 0.0F, 0.0F, 0.22F);
	/// @brief Text colour for the recording Stop button.
	inline constexpr ImVec4 kStopButtonText = ImVec4(0.85F, 0.15F, 0.15F, 1.0F);

	/// @brief Base colour of the recording Start button's red circle icon; alpha is applied
	/// separately at the call site to reflect whether recording can currently be started.
	inline constexpr ImVec4 kRecordButtonBase = ImVec4(0.72F, 0.13F, 0.13F, 1.0F);

	// ---------------------------------------------------------------------------------------
	// Simulation window specifics
	// ---------------------------------------------------------------------------------------
	/// @brief Darken factor applied to the theme's window background for the sidebar strip.
	inline constexpr float kSidebarBackgroundDarkenFactor = 0.96F;

	// ---------------------------------------------------------------------------------------
	// Element window specifics
	// ---------------------------------------------------------------------------------------
	/// @brief Brighten factor for a category header's background on hover.
	inline constexpr float kCategoryHeaderHoverBrighten = 1.1F;
	/// @brief Fill alpha for an element panel's soft background, derived from its category colour.
	inline constexpr float kElementPanelFillAlpha = 0.18F;
	/// @brief Border alpha for an element panel's outline, derived from its category colour.
	inline constexpr float kElementPanelBorderAlpha = 0.35F;
}
