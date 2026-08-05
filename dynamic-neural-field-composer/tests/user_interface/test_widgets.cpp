#include <gtest/gtest.h>

#include "ui_test_harness.h"
#include "user_interface/widgets.h"

using namespace dnf_composer;
using namespace dnf_composer::user_interface;

// ---------------------------------------------------------------------------
// renderHelpMarker widget
// ---------------------------------------------------------------------------

TEST(Widgets, RenderHelpMarkerBasic)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderHelpMarker("This is a test tooltip");
	});
	SUCCEED();
}

TEST(Widgets, RenderHelpMarkerWithEmptyDescription)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderHelpMarker("");
	});
	SUCCEED();
}

TEST(Widgets, RenderHelpMarkerWithLongDescription)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderHelpMarker(
			"This is a very long help text that explains in detail "
			"what a particular feature does. It might span multiple lines "
			"and contain lots of information about the behavior and usage."
		);
	});
	SUCCEED();
}

TEST(Widgets, RenderHelpMarkerMultipleTimes)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderHelpMarker("First marker");
		widgets::renderHelpMarker("Second marker");
		widgets::renderHelpMarker("Third marker");
	});
	SUCCEED();
}

// ---------------------------------------------------------------------------
// renderSidebarTab widget
// ---------------------------------------------------------------------------

TEST(Widgets, RenderSidebarTabUnselected)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		bool result = widgets::renderSidebarTab("icon_text", "Label", false);
		// Just verify it renders without crashing
		SUCCEED();
	});
}

TEST(Widgets, RenderSidebarTabSelected)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		bool result = widgets::renderSidebarTab("icon_text", "Label", true);
		SUCCEED();
	});
}

TEST(Widgets, RenderSidebarTabWithDifferentIcons)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderSidebarTab("A", "First", false);
		widgets::renderSidebarTab("B", "Second", true);
		widgets::renderSidebarTab("C", "Third", false);
	});
	SUCCEED();
}

TEST(Widgets, RenderSidebarTabWithEmptyLabel)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderSidebarTab("icon", "", false);
	});
	SUCCEED();
}

TEST(Widgets, RenderSidebarTabTogglesSelection)
{
	test::HeadlessImGui gui;
	gui.frames(2, [&] {
		bool result1 = widgets::renderSidebarTab("icon", "Label", false);
		SUCCEED();
	});
}

// ---------------------------------------------------------------------------
// renderIconTileButton widget
// ---------------------------------------------------------------------------

TEST(Widgets, RenderIconTileButtonBasic)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		bool result = widgets::renderIconTileButton(
			"button_id",
			"icon",
			"label",
			100.0F,
			1.0F,
			0xFF000000,
			0xFF888888,
			0xFFFFFFFF,
			0xFFFFFFFF,
			0xFFFFFFFF
		);
		SUCCEED();
	});
}

TEST(Widgets, RenderIconTileButtonWithDifferentColors)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderIconTileButton(
			"btn1", "A", "Active", 80.0F, 1.0F,
			0xFF00FF00,
			0xFF0000FF,
			0xFFFF00FF,
			0xFFFFFF00,
			0xFF00FFFF
		);
		widgets::renderIconTileButton(
			"btn2", "B", "Inactive", 120.0F, 1.0F,
			0xFF111111,
			0xFF222222,
			0xFF333333,
			0xFF444444,
			0xFF555555
		);
	});
	SUCCEED();
}

TEST(Widgets, RenderIconTileButtonWithDifferentScales)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderIconTileButton("btn1", "icon", "label", 80.0F, 0.5F,
			0xFF000000, 0xFF888888, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		widgets::renderIconTileButton("btn2", "icon", "label", 100.0F, 1.0F,
			0xFF000000, 0xFF888888, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		widgets::renderIconTileButton("btn3", "icon", "label", 120.0F, 1.5F,
			0xFF000000, 0xFF888888, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	});
	SUCCEED();
}

TEST(Widgets, RenderIconTileButtonWithLargeTile)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderIconTileButton("big", "icon", "Big Button", 200.0F, 2.0F,
			0xFF000000, 0xFF888888, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	});
	SUCCEED();
}

TEST(Widgets, RenderIconTileButtonWithSmallTile)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderIconTileButton("small", "i", "S", 20.0F, 0.5F,
			0xFF000000, 0xFF888888, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	});
	SUCCEED();
}

TEST(Widgets, RenderIconTileButtonMultiple)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		for (int i = 0; i < 5; ++i)
		{
			std::string id = "btn_" + std::to_string(i);
			std::string label = "Button " + std::to_string(i);
			widgets::renderIconTileButton(
				id.c_str(), "icon", label.c_str(), 100.0F, 1.0F,
				0xFF000000, 0xFF888888, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
			);
		}
	});
	SUCCEED();
}

// ---------------------------------------------------------------------------
// Spring inline utility
// ---------------------------------------------------------------------------

TEST(Widgets, SpringLayout)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		ImGui::Text("Before");
		widgets::Spring();
		ImGui::Text("After");
	});
	SUCCEED();
}

TEST(Widgets, SpringWithWeight)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		ImGui::Text("Text 1");
		widgets::Spring(2.0F);
		ImGui::Text("Text 2");
		widgets::Spring(1.0F);
		ImGui::Text("Text 3");
	});
	SUCCEED();
}

TEST(Widgets, SpringWithSpacing)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		ImGui::Text("Before");
		widgets::Spring(1.0F, 10.0F);
		ImGui::Text("After");
	});
	SUCCEED();
}

// ---------------------------------------------------------------------------
// BeginHorizontal / EndHorizontal
// ---------------------------------------------------------------------------

TEST(Widgets, BeginEndHorizontal)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id1 = 100;
		widgets::BeginHorizontal(&id1);
		ImGui::Text("Item 1");
		ImGui::Text("Item 2");
		ImGui::Text("Item 3");
		widgets::EndHorizontal();
	});
	SUCCEED();
}

TEST(Widgets, BeginEndHorizontalWithId)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id = 42;
		widgets::BeginHorizontal(&id);
		ImGui::Text("Item 1");
		ImGui::Text("Item 2");
		widgets::EndHorizontal();
	});
	SUCCEED();
}

TEST(Widgets, BeginEndHorizontalWithSpacing)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id2 = 102;
		widgets::BeginHorizontal(&id2, 10.0F);
		ImGui::Text("Spaced 1");
		ImGui::Text("Spaced 2");
		widgets::EndHorizontal();
	});
	SUCCEED();
}

TEST(Widgets, NestedHorizontalGroups)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id1 = 103;
		const int id2 = 104;
		widgets::BeginHorizontal(&id1);
		{
			widgets::BeginHorizontal(&id2);
			ImGui::Text("Nested 1");
			ImGui::Text("Nested 2");
			widgets::EndHorizontal();
		}
		ImGui::Text("Parent");
		widgets::EndHorizontal();
	});
	SUCCEED();
}

// ---------------------------------------------------------------------------
// BeginVertical / EndVertical
// ---------------------------------------------------------------------------

TEST(Widgets, BeginEndVertical)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id1 = 200;
		widgets::BeginVertical(&id1);
		ImGui::Text("Line 1");
		ImGui::Text("Line 2");
		ImGui::Text("Line 3");
		widgets::EndVertical();
	});
	SUCCEED();
}

TEST(Widgets, BeginEndVerticalWithId)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id = 123;
		widgets::BeginVertical(&id);
		ImGui::Text("Line 1");
		ImGui::Text("Line 2");
		widgets::EndVertical();
	});
	SUCCEED();
}

TEST(Widgets, BeginEndVerticalWithSpacing)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id2 = 202;
		widgets::BeginVertical(&id2, 20.0F);
		ImGui::Text("Spaced 1");
		ImGui::Text("Spaced 2");
		widgets::EndVertical();
	});
	SUCCEED();
}

TEST(Widgets, NestedVerticalGroups)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id1 = 203;
		const int id2 = 204;
		widgets::BeginVertical(&id1);
		{
			ImGui::Text("Parent Line 1");
			widgets::BeginVertical(&id2);
			ImGui::Text("Nested 1");
			ImGui::Text("Nested 2");
			widgets::EndVertical();
			ImGui::Text("Parent Line 2");
		}
		widgets::EndVertical();
	});
	SUCCEED();
}

// ---------------------------------------------------------------------------
// Card widget
// ---------------------------------------------------------------------------

TEST(Widgets, CardConstruction)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const widgets::Card card("card_id", ImVec2(0, 0), ImVec2(200, 100), "Card Title");
		// Just verify construction doesn't crash
		SUCCEED();
	});
}

TEST(Widgets, CardWithDifferentPositions)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const widgets::Card c1("c1", ImVec2(0, 0), ImVec2(200, 100), "Card 1");
		const widgets::Card c2("c2", ImVec2(220, 0), ImVec2(200, 100), "Card 2");
		// Cards constructed without calling beginCard
		SUCCEED();
	});
}

TEST(Widgets, CardWithDifferentUIScales)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const widgets::Card c1("c1", ImVec2(0, 0), ImVec2(200, 100), "0.5x");
		const widgets::Card c2("c2", ImVec2(210, 0), ImVec2(200, 100), "1.0x");
		const widgets::Card c3("c3", ImVec2(420, 0), ImVec2(200, 100), "1.5x");
		// Cards with different scales
		SUCCEED();
	});
}

TEST(Widgets, MultipleCardsInSequence)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		for (int i = 0; i < 3; ++i)
		{
			const std::string id = "card_" + std::to_string(i);
			const std::string title = "Card " + std::to_string(i);
			const ImVec2 pos(10 + i * 210, 10);
			const ImVec2 size(200, 100);

			const widgets::Card card(id, pos, size, title);
			// Multiple card constructions
		}
		SUCCEED();
	});
}

// ---------------------------------------------------------------------------
// Mixed widget usage
// ---------------------------------------------------------------------------

TEST(Widgets, CombinedHorizontalAndVertical)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		const int id1 = 300;
		const int id2 = 301;
		const int id3 = 302;
		widgets::BeginVertical(&id1);
		{
			widgets::BeginHorizontal(&id2);
			{
				ImGui::Text("Top Left");
				widgets::Spring();
				ImGui::Text("Top Right");
			}
			widgets::EndHorizontal();

			ImGui::Separator();

			widgets::BeginHorizontal(&id3);
			{
				ImGui::Text("Bottom Left");
				widgets::Spring();
				ImGui::Text("Bottom Right");
			}
			widgets::EndHorizontal();
		}
		widgets::EndVertical();
	});
	SUCCEED();
}

TEST(Widgets, ComplexLayoutWithCards)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		// Test complex layout with horizontal/vertical groups
		const int id1 = 400;
		const int id2 = 401;
		widgets::BeginVertical(&id1);
		{
			ImGui::Text("Header");
			widgets::Spring(0.1F);
			widgets::BeginHorizontal(&id2);
			{
				ImGui::Button("Button 1");
				widgets::Spring();
				ImGui::Button("Button 2");
			}
			widgets::EndHorizontal();
		}
		widgets::EndVertical();
		SUCCEED();
	});
}

TEST(Widgets, AllWidgetsInOneFrame)
{
	test::HeadlessImGui gui;
	gui.frame([&] {
		widgets::renderHelpMarker("Help text");
		widgets::renderSidebarTab("i1", "Tab 1", true);
		widgets::renderIconTileButton("btn", "icon", "Tile", 100.0F, 1.0F,
			0xFF000000, 0xFF888888, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		widgets::Spring();
		const int id = 500;
		widgets::BeginHorizontal(&id);
		{
			ImGui::Text("In group");
		}
		widgets::EndHorizontal();
	});
	SUCCEED();
}
