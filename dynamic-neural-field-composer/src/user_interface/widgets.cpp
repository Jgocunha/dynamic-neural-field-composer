#include <utility>

#include "user_interface/widgets.h"

#include "application/application.h"

extern ImFont* g_LightFont;
extern ImFont* g_MediumFont;
extern ImFont* g_BoldFont;
extern ImFont* g_BlackFont;
extern ImFont* g_MonoFont;
extern ImFont* g_IconsFont;

namespace dnf_composer::user_interface::widgets
{
	void renderHelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	bool renderSidebarTab(const char* icon, const char* label, bool selected)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		const ImGuiID id = window->GetID(std::string(icon + std::string(label)).c_str());

		//  Layout (scaled by current font size so it looks good at any DPI)
		constexpr float line_h    = 30.0f;                  // row height
		constexpr float pad_x     = 10.0f;                  // left padding
		constexpr float icon_box  = 28.0f;                  // width reserved for icon
		constexpr float gap_x     = 12.0f;                  // gap between icon and label
		constexpr float total_w   = pad_x + icon_box + gap_x + 110.0f; // item width

		const ImVec2 pos  = window->DC.CursorPos;
		const ImRect bb(pos, ImVec2(pos.x + total_w, pos.y + line_h));
		ImGui::ItemSize(ImVec2(total_w, line_h), 0);
		if (!ImGui::ItemAdd(bb, id)) return false;

		bool hovered, held;
		const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		if (hovered || held) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

		// Animations
		static std::map<ImGuiID,float> hover_animation, filled_animation, fill_animation;
		const ImGuiIO& io = ImGui::GetIO();
		float& a_hover = hover_animation[id];
		float& a_fill  = filled_animation[id]; // drives the left bar
		float& a_mix   = fill_animation[id];   // drives text/icon color
		a_hover = ImClamp(a_hover + (0.20f * io.DeltaTime * (hovered || ImGui::IsItemActive() ? 1.f : -1.f)), 0.0f, 0.15f);
		// IMPORTANT: no lower-bound to hover — let it reach 0 when not selected
		a_fill  = ImClamp(a_fill  + (2.55f * io.DeltaTime * (selected ? 1.f : -1.f)), 0.0f, 1.0f);
		a_mix   = ImClamp(a_mix   + (1.75f * io.DeltaTime * (selected ? 1.f : -1.f)), 0.0f, 1.0f);

		const ImVec4 text_sel  = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight); // pure accent from theme
		const ImVec4 text_uns  = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

		const ImVec4 icon_col = ImLerp(text_uns, text_sel, a_fill);
		const ImVec4 text_col = ImLerp(text_uns, text_sel, a_fill);

		if (a_fill > 0.001f)
		{
			ImVec4 a = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
			a.w = a_fill; // keep dynamic alpha
			const ImU32 col = ImGui::GetColorU32(a);
			ImGui::GetForegroundDrawList()->AddRectFilled(
				ImVec2(bb.Min.x, bb.Min.y),
				ImVec2(bb.Min.x + a_fill * 6.0f, bb.Max.y),
				col, 7.0f);
		}

		// lamba to center text vertically within a bounding box
		auto centerTextY = [](const ImFont* font, const float y0, const float h) noexcept -> float
		{
			// in ImGui: height = Ascent - Descent, AddText expects pos.y == top
			if (!font) return y0;
			const float text_h = font->Ascent - font->Descent;
			return y0 + (h - text_h) * 0.5f;
		};

		// Icon: center using icon font metrics
		ImGui::PushFont(g_IconsFont);
		const float icon_y = centerTextY(g_IconsFont, bb.Min.y, line_h);
		// center icon within its reserved box horizontally
		const ImVec2 icon_size = ImGui::CalcTextSize(icon);
		const float icon_x = bb.Min.x + pad_x + (icon_box - icon_size.x) * 0.5f;
		ImGui::GetForegroundDrawList()->AddText(ImVec2(icon_x, icon_y), ImColor(icon_col), icon);
		ImGui::PopFont();

		// Label: center using label font metrics
		ImGui::PushFont(g_MediumFont);
		const float text_y = centerTextY(g_MediumFont, bb.Min.y, line_h);
		const float text_x = bb.Min.x + pad_x + icon_box + gap_x;
		ImGui::GetForegroundDrawList()->AddText(ImVec2(text_x, text_y), ImColor(text_col), label);
		ImGui::PopFont();

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

		const float r   = 10.0f * uiScale;
		const float pad = 12.0f * uiScale;
		const float th  = 32.0f * uiScale;     // title bar height
		const float titleGap = 30.0f  * uiScale;  // extra space below title

		// background
		dl->AddRectFilled(a, b, ImGui::GetColorU32(ImGuiCol_FrameBg), r);
		// border
		dl->AddRect(a, b, ImGui::GetColorU32(ImGuiCol_Border), r);

		// title text
		ImGui::PushFont(g_BoldFont);
		ImGui::SetCursorPosX(30);
		ImGui::SetCursorPosY(a.y + (th - ImGui::GetTextLineHeight())*0.5f);
		ImGui::Text(title.c_str());
		ImGui::PopFont();

		// set the cursor to the card’s inner body area and start a child
		const auto body_pos  = ImVec2(a.x + pad, a.y + th + pad + titleGap);
		const auto body_size = ImVec2(size.x - 2*pad, size.y - th - 2*pad - titleGap);
		ImGui::SetCursorScreenPos(body_pos);
		return ImGui::BeginChild(id.c_str(), body_size, false, ImGuiWindowFlags_NoSavedSettings);

	}

	void Card::endCard()
	{
		ImGui::EndChild();
	}
}
