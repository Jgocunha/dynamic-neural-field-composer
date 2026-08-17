// Regression guard for the shared GUI colour registry (#28).
//
// There is no behaviour to assert about a table of named constants -- what this
// pins down is that the sweep that moved every ImVec4()/IM_COL32() literal in the
// first-party user-interface layer into the registry did not typo a hex digit or a
// component along the way. Expected values below are copied verbatim from the
// literals on origin/main before the sweep (or, for the element-type palette, from
// the pre-sweep node-graph literals -- see ElementPaletteIsSingleSourceOfTruth).

#include <gtest/gtest.h>

#include "user_interface/colour_registry.h"
#include "user_interface/node_graph_window.h"
#include "user_interface/element_window.h"
#include "element_parameters/element_parameters.h"

using namespace dnf_composer::user_interface::colour;
using dnf_composer::element::ElementLabel;

namespace
{
	void expectImVec4Eq(const ImVec4& actual, const float x, const float y, const float z, const float w)
	{
		EXPECT_FLOAT_EQ(actual.x, x);
		EXPECT_FLOAT_EQ(actual.y, y);
		EXPECT_FLOAT_EQ(actual.z, z);
		EXPECT_FLOAT_EQ(actual.w, w);
	}

	void expectImVec4Near(const ImVec4& actual, const ImVec4& expected, const float tol)
	{
		EXPECT_NEAR(actual.x, expected.x, tol);
		EXPECT_NEAR(actual.y, expected.y, tol);
		EXPECT_NEAR(actual.z, expected.z, tol);
		EXPECT_NEAR(actual.w, expected.w, tol);
	}
}

TEST(ColourRegistry, ElementPaletteMatchesPreSweepNodeGraphLiterals)
{
	// These 25 constants back both getHeaderColorForElementType() (ImU32) and
	// getColorForElementType() (ImVec4, via toImVec4()). Values are the pre-sweep
	// node-graph literals -- the single source of truth this PR establishes.
	EXPECT_EQ(kNeuralFieldElement, IM_COL32(86, 128, 191, 255));
	EXPECT_EQ(kNormalNoiseElement, IM_COL32(223, 148, 84, 255));
	EXPECT_EQ(kCorrelatedNormalNoiseElement, IM_COL32(210, 110, 60, 255));
	EXPECT_EQ(kGaussKernelElement, IM_COL32(191, 63, 63, 255));
	EXPECT_EQ(kGaussStimulusElement, IM_COL32(127, 191, 127, 255));
	EXPECT_EQ(kMexicanHatKernelElement, IM_COL32(154, 121, 191, 255));
	EXPECT_EQ(kGaussFieldCouplingElement, IM_COL32(165, 102, 71, 255));
	EXPECT_EQ(kFieldCouplingElement, IM_COL32(212, 192, 121, 255));
	EXPECT_EQ(kOscillatoryKernelElement, IM_COL32(175, 133, 187, 255));
	EXPECT_EQ(kAsymmetricGaussKernelElement, IM_COL32(148, 178, 182, 255));
	EXPECT_EQ(kBoostStimulusElement, IM_COL32(242, 209, 83, 255));
	EXPECT_EQ(kMemoryTraceElement, IM_COL32(110, 160, 140, 255));
	EXPECT_EQ(kNeuralField2DElement, IM_COL32(70, 110, 175, 255));
	EXPECT_EQ(kGaussStimulus2DElement, IM_COL32(105, 175, 105, 255));
	EXPECT_EQ(kGaussKernel2DElement, IM_COL32(175, 48, 48, 255));
	EXPECT_EQ(kMexicanHatKernel2DElement, IM_COL32(138, 105, 175, 255));
	EXPECT_EQ(kNormalNoise2DElement, IM_COL32(207, 132, 68, 255));
	EXPECT_EQ(kOscillatoryKernel2DElement, IM_COL32(152, 116, 163, 255));
	EXPECT_EQ(kTimedGaussStimulusElement, IM_COL32(97, 161, 97, 255));
	EXPECT_EQ(kTimedGaussStimulus2DElement, IM_COL32(80, 133, 80, 255));
	EXPECT_EQ(kBoostStimulus2DElement, IM_COL32(210, 182, 72, 255));
	EXPECT_EQ(kCorrelatedNormalNoise2DElement, IM_COL32(182, 109, 44, 255));
	EXPECT_EQ(kAsymmetricGaussKernel2DElement, IM_COL32(129, 155, 159, 255));
	EXPECT_EQ(kMemoryTrace2DElement, IM_COL32(96, 139, 122, 255));
	EXPECT_EQ(kUnknownElement, IM_COL32(127, 127, 127, 255));

	// RESIZE/RESIZE_2D/COLLAPSE/EXPAND had no node-graph header case pre-sweep; these
	// four constants are new, carried over from element_window's pre-sweep values.
	EXPECT_EQ(kResizeElement, IM_COL32(128, 153, 179, 255));
	EXPECT_EQ(kResize2DElement, IM_COL32(107, 130, 153, 255));
	EXPECT_EQ(kCollapseElement, IM_COL32(115, 166, 158, 255));
	EXPECT_EQ(kExpandElement, IM_COL32(97, 143, 135, 255));
}

TEST(ColourRegistry, ToImVec4RoundTripsIM_COL32)
{
	expectImVec4Eq(toImVec4(IM_COL32(255, 0, 128, 64)), 1.0F, 0.0F, 128.0F / 255.0F, 64.0F / 255.0F);
	expectImVec4Eq(toImVec4(IM_COL32(0, 0, 0, 0)), 0.0F, 0.0F, 0.0F, 0.0F);
}

TEST(ColourRegistry, ElementPaletteIsSingleSourceOfTruth)
{
	// The node graph (ImU32 header) and the element window (ImVec4 card) must always
	// resolve every label with a header case to the same colour -- this is the assertion
	// that actually prevents the two views re-drifting the way they had before this PR (#28).
	constexpr ElementLabel kLabelsWithHeaderCase[] = {
		ElementLabel::NEURAL_FIELD, ElementLabel::NORMAL_NOISE, ElementLabel::CORRELATED_NORMAL_NOISE,
		ElementLabel::GAUSS_KERNEL, ElementLabel::GAUSS_STIMULUS, ElementLabel::MEXICAN_HAT_KERNEL,
		ElementLabel::GAUSS_FIELD_COUPLING, ElementLabel::FIELD_COUPLING, ElementLabel::OSCILLATORY_KERNEL,
		ElementLabel::ASYMMETRIC_GAUSS_KERNEL, ElementLabel::BOOST_STIMULUS, ElementLabel::MEMORY_TRACE,
		ElementLabel::NEURAL_FIELD_2D, ElementLabel::GAUSS_STIMULUS_2D, ElementLabel::GAUSS_KERNEL_2D,
		ElementLabel::MEXICAN_HAT_KERNEL_2D, ElementLabel::NORMAL_NOISE_2D, ElementLabel::OSCILLATORY_KERNEL_2D,
		ElementLabel::TIMED_GAUSS_STIMULUS, ElementLabel::TIMED_GAUSS_STIMULUS_2D, ElementLabel::BOOST_STIMULUS_2D,
		ElementLabel::CORRELATED_NORMAL_NOISE_2D, ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D, ElementLabel::MEMORY_TRACE_2D,
	};

	for (const ElementLabel label : kLabelsWithHeaderCase)
	{
		const ImVec4 headerAsVec4 = toImVec4(dnf_composer::user_interface::getHeaderColorForElementType(label));
		const ImVec4 cardColour = dnf_composer::user_interface::ElementWindow::getColorForElementType(label);
		expectImVec4Near(cardColour, headerAsVec4, 1.0F / 255.0F);
	}
}

TEST(ColourRegistry, ReshapeLabelsHaveNoNodeGraphHeaderCaseYet)
{
	// RESIZE/RESIZE_2D/COLLAPSE/EXPAND have an element-window colour (kResizeElement etc.)
	// but deliberately no node-graph header case -- wiring them up is a visible change
	// (currently gray) left as a follow-up (#28). This pins that gap so it's a deliberate
	// decision, not a silent omission, if the node-graph switch grows those cases later.
	for (const ElementLabel label : { ElementLabel::RESIZE, ElementLabel::RESIZE_2D,
			ElementLabel::COLLAPSE, ElementLabel::EXPAND })
	{
		EXPECT_EQ(dnf_composer::user_interface::getHeaderColorForElementType(label), kUnknownElement);
	}
	expectImVec4Near(dnf_composer::user_interface::ElementWindow::getColorForElementType(ElementLabel::RESIZE),
		toImVec4(kResizeElement), 1.0F / 255.0F);
}

TEST(ColourRegistry, NodeGraphPinAndLinkColoursMatchPreRegistryLiterals)
{
	expectImVec4Eq(kInputPinColour, 1.0F, 1.0F, 1.0F, 0.90F);
	expectImVec4Eq(kTargetPinColour, 0.83F, 0.75F, 0.47F, 0.95F);
	expectImVec4Eq(kActivationPinColour, 0.55F, 0.75F, 0.90F, 0.95F);
	expectImVec4Eq(kPinIconInnerFill, 0.0F, 0.0F, 0.0F, 0.0F);
	expectImVec4Eq(kLinkColour, 0.08F, 0.08F, 0.08F, 0.85F);
	expectImVec4Eq(kTargetLinkColour, 0.60F, 0.50F, 0.15F, 0.85F);
	expectImVec4Eq(kActivationLinkColour, 0.30F, 0.50F, 0.65F, 0.85F);
	EXPECT_FLOAT_EQ(kMultiSeriesLineAlpha, 0.86F);
}

TEST(ColourRegistry, NodeGraphCanvasAndNavColoursMatchPreRegistryLiterals)
{
	expectImVec4Eq(kCanvasBackground, 0.94F, 0.95F, 0.96F, 1.00F);
	expectImVec4Eq(kCanvasGridLine, 0.80F, 0.82F, 0.85F, 0.60F);
	expectImVec4Eq(kFloatingPanelBackground, 0.95F, 0.97F, 0.98F, 1.0F);
	expectImVec4Eq(kNavButtonBackground, 0.92F, 0.92F, 0.93F, 1.0F);
	expectImVec4Eq(kNavButtonHoveredBackground, 0.80F, 0.84F, 0.95F, 1.0F);
	expectImVec4Eq(kNavButtonActiveBackground, 0.65F, 0.72F, 0.92F, 1.0F);
}

TEST(ColourRegistry, HeatmapAndMinimapColoursMatchPreRegistryLiterals)
{
	EXPECT_EQ(kHeatmapOverlayFill, IM_COL32(255, 255, 255, 40));
	EXPECT_EQ(kHeatmapOverlayBorder, IM_COL32(0, 0, 0, 30));
	EXPECT_EQ(kHeatmapAxisText, IM_COL32(40, 40, 40, 230));
	EXPECT_EQ(kHeatmapAxisTick, IM_COL32(80, 80, 80, 180));
	EXPECT_EQ(kMinimapNodeBorder, IM_COL32(255, 255, 255, 60));
	EXPECT_EQ(kMinimapViewportBorder, IM_COL32(255, 255, 255, 200));
}

TEST(ColourRegistry, SharedButtonStateColoursMatchPreRegistryLiterals)
{
	EXPECT_FLOAT_EQ(kAccentHoverDarken, 0.9F);
	EXPECT_FLOAT_EQ(kAccentActiveDarken, 0.8F);
	EXPECT_FLOAT_EQ(kAccentHoverDarkenStrong, 0.85F);
	EXPECT_FLOAT_EQ(kAccentActiveDarkenStrong, 0.70F);
	expectImVec4Eq(kButtonTextOnAccent, 1, 1, 1, 1);
	EXPECT_EQ(kIconEnabled, IM_COL32(255, 255, 255, 255));
	EXPECT_EQ(kIconDisabled, IM_COL32(255, 255, 255, 100));
	expectImVec4Eq(kFlatButtonBackground, 0, 0, 0, 0);
	expectImVec4Eq(kFlatButtonHoverOverlay, 0, 0, 0, 0.06F);
	expectImVec4Eq(kFlatButtonActiveOverlay, 0, 0, 0, 0.12F);
	expectImVec4Eq(kTransparentChildBackground, 0, 0, 0, 0);
}

TEST(ColourRegistry, DestructiveAndRecordingColoursMatchPreRegistryLiterals)
{
	expectImVec4Eq(kDestructiveText, 0.85F, 0.25F, 0.25F, 1.0F);
	expectImVec4Eq(kValidationErrorText, 0.90F, 0.35F, 0.35F, 1.0F);
	expectImVec4Eq(kDestructiveButtonHoverOverlay, 1.0F, 0.0F, 0.0F, 0.12F);
	expectImVec4Eq(kDestructiveButtonActiveOverlay, 1.0F, 0.0F, 0.0F, 0.22F);
	expectImVec4Eq(kStopButtonText, 0.85F, 0.15F, 0.15F, 1.0F);
	expectImVec4Eq(kRecordButtonBase, 0.72F, 0.13F, 0.13F, 1.0F);
}

TEST(ColourRegistry, WindowAndPanelFactorsMatchPreRegistryLiterals)
{
	EXPECT_FLOAT_EQ(kSidebarBackgroundDarkenFactor, 0.96F);
	EXPECT_FLOAT_EQ(kCategoryHeaderHoverBrighten, 1.1F);
	EXPECT_FLOAT_EQ(kElementPanelFillAlpha, 0.18F);
	EXPECT_FLOAT_EQ(kElementPanelBorderAlpha, 0.35F);
}

TEST(ColourRegistry, ElementCardColoursMatchPreRegistryLiterals)
{
	expectImVec4Eq(kCardBackground, 1.0F, 1.0F, 1.0F, 1.0F);
	expectImVec4Eq(kCardBorder, 0.82F, 0.85F, 0.89F, 1.0F);
}

TEST(ColourRegistry, ControlBarColoursMatchPreRegistryLiterals)
{
	expectImVec4Eq(kToolbarButtonHovered, 0.878F, 0.878F, 0.878F, 1.0F);
	expectImVec4Eq(kToolbarButtonActive, 0.835F, 0.835F, 0.835F, 1.0F);
	expectImVec4Eq(kStopControlText, 0.8F, 0.1F, 0.1F, 1.0F);
}

TEST(ColourRegistry, FieldMetricsColoursMatchPreRegistryLiterals)
{
	EXPECT_EQ(kMetricCardDot, IM_COL32(74, 144, 217, 255));
	expectImVec4Eq(kMetricStableText, 0.22F, 0.75F, 0.35F, 1.0F);
	expectImVec4Eq(kMetricUnstableText, 0.90F, 0.55F, 0.10F, 1.0F);
	EXPECT_EQ(kMetricBarTrack, IM_COL32(60, 60, 60, 80));
	EXPECT_EQ(kMetricBarStableFill, IM_COL32(56, 200, 90, 180));
	EXPECT_EQ(kMetricBarUnstableFill, IM_COL32(230, 140, 25, 180));
	EXPECT_EQ(kMetricBarZeroTick, IM_COL32(255, 255, 255, 150));
	EXPECT_EQ(kMetricBarNegativeTrack, IM_COL32(180, 60, 60, 80));
}

TEST(ColourRegistry, StatusBarColoursMatchPreRegistryLiterals)
{
	expectImVec4Eq(kStatusDotStopped, 0.75F, 0.20F, 0.20F, 1.0F);
	expectImVec4Eq(kStatusDotRunning, 0.20F, 0.75F, 0.20F, 1.0F);
	expectImVec4Eq(kStatusDotPaused, 0.90F, 0.70F, 0.10F, 1.0F);
}

TEST(ColourRegistry, LogConsoleColourMatchesPreRegistryLiteral)
{
	expectImVec4Eq(kLogConsoleBackground, 0.07F, 0.07F, 0.07F, 1.0F);
}
