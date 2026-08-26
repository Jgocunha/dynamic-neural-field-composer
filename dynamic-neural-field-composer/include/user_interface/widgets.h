#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "tools/math.h"

namespace dnf_composer::user_interface::widgets
{
	/// @brief Draw a "(?)" marker that shows @p desc as a tooltip on hover.
	/// @param desc Tooltip text.
	void renderHelpMarker(const char* desc);
	/// @brief Draw one icon+label tab in a vertical sidebar.
	/// @param icon     Icon glyph/text.
	/// @param label    Tab label.
	/// @param selected Whether the tab is currently the active one.
	/// @return True if the tab was clicked this frame.
	bool renderSidebarTab(const char* icon, const char* label, bool selected);
	/// @brief Draw a square icon button with a label underneath, used in card-style toolbars.
	/// @param id       Unique ImGui id for the button.
	/// @param icon     Icon glyph/text.
	/// @param label    Label drawn below the icon.
	/// @param tile     Side length of the square tile.
	/// @param uiScale  Current UI scale factor.
	/// @param colBg    Background colour.
	/// @param colHover Background colour on hover.
	/// @param colActive Background colour while pressed.
	/// @param colText  Icon colour.
	/// @param colLabel Label text colour.
	/// @return True if the button was clicked this frame.
	bool renderIconTileButton(const char* id, const char* icon, const char* label,
						   float tile, float uiScale,
						   ImU32 colBg, ImU32 colHover, ImU32 colActive,
						   ImU32 colText, ImU32 colLabel);
	/// @brief Insert spacing between widgets laid out on the same line, then continue the line.
	/// @param weight  Not currently used.
	/// @param spacing Fixed spacing, in pixels.
	inline void Spring(float weight = 1.0F, const float spacing = 0.0F)
	{
		ImGui::Dummy(ImVec2(spacing, 0));
		ImGui::SameLine(0, 0);
	}
	/// @brief Begin a horizontal layout group; pair with EndHorizontal().
	/// @param id      Optional id pushed onto the ImGui id stack for the group's contents.
	/// @param spacing Not currently used.
	/// @return Always true.
	inline bool BeginHorizontal(const void* id = nullptr, float spacing = 0.0F)
	{
		if (id != nullptr) { ImGui::PushID(id);
}
		ImGui::BeginGroup();
		return true;
	}

	/// @brief End a layout group started with BeginHorizontal().
	inline void EndHorizontal()
	{
		ImGui::EndGroup();
		ImGui::PopID();
	}
	/// @brief Begin a vertical layout group; pair with EndVertical().
	/// @param id      Optional id pushed onto the ImGui id stack for the group's contents.
	/// @param spacing Not currently used.
	/// @return Always true.
	inline bool BeginVertical(const void* id = nullptr, float spacing = 0.0F)
	{
		if (id != nullptr) { ImGui::PushID(id);
}
		ImGui::BeginGroup();
		return true;
	}

	/// @brief End a layout group started with BeginVertical().
	inline void EndVertical()
	{
		ImGui::EndGroup();
		if (ImGui::GetID(static_cast<const void*>(nullptr)) != 0) { ImGui::PopID();
}
	}
	/// @brief Fixed-rectangle titled panel drawn as a child window.
	class Card
	{
	private:
		std::string id;
		ImVec2 topLeftPosition;
		ImVec2 size;
		std::string title;
	public:
		/// @brief Construct a card at a fixed position and size.
		/// @param id              Unique ImGui id for the card's child window.
		/// @param topLeftPosition Top-left corner of the card (screen coords).
		/// @param size            Size of the card.
		/// @param title           Title drawn in the card's header.
		Card(std::string  id, const ImVec2& topLeftPosition, const ImVec2& size, std::string  title);
		/// @brief Begin the card's child window; must be paired with endCard() if this returns true.
		/// @param uiScale Current UI scale factor.
		/// @return True if the card is visible and endCard() must be called.
		[[nodiscard]] bool beginCard(const float& uiScale) const; // remember to end the card
		/// @brief End a card window started with beginCard().
		static void endCard();
	};
}
