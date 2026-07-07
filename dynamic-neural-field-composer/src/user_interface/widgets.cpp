#include <utility>

#include "user_interface/widgets.h"

#include "application/application.h"
#include "user_interface/fonts/IconsFontAwesome6.h"

extern ImFont* g_LightMediumFont;
extern ImFont* g_MediumMediumFont;
extern ImFont* g_BoldLargeFont;
extern ImFont* g_BlackLargeFont;
extern ImFont* g_MonoMediumFont;
extern ImFont* g_MediumIconsFont;
extern ImFont* g_LargeIconsFont;

namespace dnf_composer::user_interface::widgets
{
	void renderHelpMarker(const char* desc)
	{
		ImGui::PushFont(g_MediumIconsFont);
		ImGui::Text(ICON_FA_CIRCLE_QUESTION);
		ImGui::PopFont();
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0F);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	bool renderSidebarTab(const char* icon, const char* /*label*/, bool selected)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) { return false;
}

		const ImGuiID id = window->GetID(icon);

		constexpr float line_h  = 48.0F;  // row height — more vertical breathing room
		constexpr float total_w = 52.0F;  // icon-only strip width

		const ImVec2 pos = window->DC.CursorPos;
		const ImRect bb(pos, ImVec2(pos.x + total_w, pos.y + line_h));
		ImGui::ItemSize(ImVec2(total_w, line_h), 0);
		if (!ImGui::ItemAdd(bb, id)) { return false;
}

		bool hovered;
		bool held;
		const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		if (hovered || held) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
}

		// Animations
		static std::map<ImGuiID,float> hover_animation;
		static std::map<ImGuiID,float> filled_animation;
		static std::map<ImGuiID,float> fill_animation;
		const ImGuiIO& io = ImGui::GetIO();
		float& a_hover = hover_animation[id];
		float& a_fill  = filled_animation[id]; // drives the left bar
		float& a_mix   = fill_animation[id];   // drives icon color
		a_hover = ImClamp(a_hover + (0.20F * io.DeltaTime * (hovered || ImGui::IsItemActive() ? 1.F : -1.F)), 0.0F, 0.15F);
		a_fill  = ImClamp(a_fill  + (2.55F * io.DeltaTime * (selected ? 1.F : -1.F)), 0.0F, 1.0F);
		a_mix   = ImClamp(a_mix   + (1.75F * io.DeltaTime * (selected ? 1.F : -1.F)), 0.0F, 1.0F);

		const ImVec4 text_sel = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
		const ImVec4 text_uns = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
		const ImVec4 icon_col = ImLerp(text_uns, text_sel, a_fill);

		ImDrawList* dl = ImGui::GetWindowDrawList();

		if (a_fill > 0.001F)
		{
			ImVec4 a = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
			a.w = a_fill;
			const ImU32 col = ImGui::GetColorU32(a);
			dl->AddRectFilled(
				ImVec2(bb.Min.x, bb.Min.y),
				ImVec2(bb.Min.x + a_fill * 6.0F, bb.Max.y),
				col, 7.0F);
		}

		auto centerTextY = [](const ImFont* font, const float y0, const float h) -> float
		{
			if (font == nullptr) { return y0;
}
			const ImFontBaked* baked = const_cast<ImFont*>(font)->GetFontBaked(font->LegacySize);
			const float text_h = baked != nullptr ? (baked->Ascent - baked->Descent) : font->LegacySize;
			return y0 + (h - text_h) * 0.5F;
		};

		ImGui::PushFont(g_LargeIconsFont);
		const float icon_y = centerTextY(g_LargeIconsFont, bb.Min.y, line_h);
		const ImVec2 icon_size = ImGui::CalcTextSize(icon);
		const float icon_x = window->Pos.x + (window->Size.x - icon_size.x) * 0.5F;
		dl->AddText(ImVec2(icon_x, icon_y), ImColor(icon_col), icon);
		ImGui::PopFont();

		return pressed;
	}

	bool renderIconTileButton(const char* id, const char* icon, const char* label,
						   const float tile, const float uiScale,
						   const ImU32 colBg, const ImU32 colHover, const ImU32 colActive,
						   const ImU32 colText, const ImU32 colLabel)
	{
		ImGui::PushID(id);
		ImGui::BeginGroup();

		// Button styling (rounded square, soft bg)
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0F * uiScale);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button,        colBg);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  colActive);
		ImGui::PushStyleColor(ImGuiCol_Text,          colText);

		// Center icon inside the square by padding based on glyph size
		const ImVec2 iconBox(tile, tile);

		ImGui::PushFont(g_LargeIconsFont);
		const bool pressed = ImGui::Button(icon, iconBox);
		ImGui::PopFont();

		//ImGui::PopStyleVar(); // FramePadding
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar(2);

		// Label under the tile, centered to the button width
		const ImVec2 lbl = ImGui::CalcTextSize(label);
		const auto btn = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		// X: center within the tile using button's left edge
		const float labelX = btn.Min.x + (tile - lbl.x) * 0.5F;
		// Y: current cursor is already below the button; use it
		ImVec2 labelPos = ImGui::GetCursorScreenPos();
		labelPos.x = labelX;

		ImGui::SetCursorScreenPos(labelPos);
		ImGui::TextUnformatted(label);

		ImGui::EndGroup();
		ImGui::PopID();
		return pressed;
	}

	Card::Card(std::string  id, const ImVec2& topLeftPosition, const ImVec2& size, std::string  title)
		: id(std::move(id)), topLeftPosition(topLeftPosition), size(size), title(std::move(title))
	{}

	bool Card::beginCard(const float& uiScale) const
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 a = topLeftPosition;
		const auto b = ImVec2(topLeftPosition.x + size.x, topLeftPosition.y + size.y);

		const float r   = 10.0F * uiScale;
		const float pad = 22.0F * uiScale;
		const float th  = 32.0F * uiScale;     // title bar height
		const float titleGap = 10.0F  * uiScale;  // extra space below title

		// background
		dl->AddRectFilled(a, b, ImGui::GetColorU32(ImGuiCol_FrameBg), r);
		// border
		dl->AddRect(a, b, ImGui::GetColorU32(ImGuiCol_Border), r);

		// title text
		ImGui::PushFont(g_BlackLargeFont);
		// place title using screen coordinates, aligned with the card's inner padding
		const float yOffset = 20.0F * ImGui::GetIO().FontGlobalScale;  // tweak value to taste
		const ImVec2 title_pos(a.x + pad, a.y + (th - ImGui::GetTextLineHeight()) * 0.5F + yOffset);
		ImGui::SetCursorScreenPos(title_pos);
		ImGui::TextUnformatted(title.c_str());
		ImGui::PopFont();

		// set the cursor to the card’s inner body area and start a child
		const auto body_pos  = ImVec2(a.x + pad, a.y + th + pad + titleGap);
		const auto body_size = ImVec2(size.x - 2*pad, size.y - th - 2*pad - titleGap);
		ImGui::SetCursorScreenPos(body_pos);

		return ImGui::BeginChild(id.c_str(), body_size, 0, ImGuiWindowFlags_NoSavedSettings);

	}

	void Card::endCard()
	{
		ImGui::EndChild();
	}
}
