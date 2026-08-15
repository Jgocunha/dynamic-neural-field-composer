// Regression guard for the shared GUI colour registry (#28).
//
// There is no behaviour to assert about a table of named constants -- what this
// pins down is that the sweep that moved ~120 scattered ImVec4()/IM_COL32() literals
// out of node_graph_window.h/.cpp, element_window.cpp, and simulation_window.cpp did
// not typo a hex digit or a component along the way. Expected values below are copied
// verbatim from the literals on origin/main before the sweep.

#include <gtest/gtest.h>

#include "user_interface/colour_registry.h"

using namespace dnf_composer::user_interface::colour;

namespace
{
	void expectImVec4Eq(const ImVec4& actual, const float x, const float y, const float z, const float w)
	{
		EXPECT_FLOAT_EQ(actual.x, x);
		EXPECT_FLOAT_EQ(actual.y, y);
		EXPECT_FLOAT_EQ(actual.z, z);
		EXPECT_FLOAT_EQ(actual.w, w);
	}
}

TEST(ColourRegistry, NodeGraphHeaderColoursMatchPreRegistryLiterals)
{
	EXPECT_EQ(kNeuralFieldHeader, IM_COL32(86, 128, 191, 255));
	EXPECT_EQ(kGaussKernelHeader, IM_COL32(191, 63, 63, 255));
	EXPECT_EQ(kFieldCouplingHeader, IM_COL32(212, 192, 121, 255));
	EXPECT_EQ(kMemoryTrace2DHeader, IM_COL32(96, 139, 122, 255));
	EXPECT_EQ(kUnknownElementHeader, IM_COL32(127, 127, 127, 255));
}

TEST(ColourRegistry, NodeGraphPinAndLinkColoursMatchPreRegistryLiterals)
{
	expectImVec4Eq(kInputPinColour, 1.0F, 1.0F, 1.0F, 0.90F);
	expectImVec4Eq(kTargetPinColour, 0.83F, 0.75F, 0.47F, 0.95F);
	expectImVec4Eq(kActivationPinColour, 0.55F, 0.75F, 0.90F, 0.95F);
	expectImVec4Eq(kLinkColour, 0.08F, 0.08F, 0.08F, 0.85F);
	expectImVec4Eq(kTargetLinkColour, 0.60F, 0.50F, 0.15F, 0.85F);
	expectImVec4Eq(kActivationLinkColour, 0.30F, 0.50F, 0.65F, 0.85F);
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
	EXPECT_FLOAT_EQ(kMultiSeriesLineAlpha, 0.86F);
}
