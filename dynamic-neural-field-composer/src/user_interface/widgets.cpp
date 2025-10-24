#include "user_interface/widgets.h"

#include "application/application.h"


namespace dnf_composer
{
	namespace user_interface
	{
		namespace widgets
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

			bool tab(const char* icon, const char* label, bool selected)
			{
			    ImGuiWindow* window = ImGui::GetCurrentWindow();
			    if (window->SkipItems) return false;

			    const ImGuiID id = window->GetID(std::string(icon + std::string(label)).c_str());

			    // --- Layout (scaled by current font size so it looks good at any DPI)
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
			    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

			    if (hovered || held) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

			    // --- Animations (unchanged from your version)
				static std::map<ImGuiID,float> hover_animation, filled_animation, fill_animation;
				ImGuiIO& io = ImGui::GetIO();
				float& a_hover = hover_animation[id];
				float& a_fill  = filled_animation[id]; // drives the left bar
				float& a_mix   = fill_animation[id];   // drives text/icon color
				a_hover = ImClamp(a_hover + (0.20f * io.DeltaTime * (hovered || ImGui::IsItemActive() ? 1.f : -1.f)), 0.0f, 0.15f);
				// IMPORTANT: no lower-bound to hover — let it reach 0 when not selected
				a_fill  = ImClamp(a_fill  + (2.55f * io.DeltaTime * (selected ? 1.f : -1.f)), 0.0f, 1.0f);
				a_mix   = ImClamp(a_mix   + (1.75f * io.DeltaTime * (selected ? 1.f : -1.f)), 0.0f, 1.0f);

			    constexpr ImVec4 highlight = ImVec4(64/255.f, 163/255.f, 130/255.f, 1.0f);
			    constexpr ImVec4 text_sel  = highlight;
			    constexpr ImVec4 text_uns  = ImVec4(0.58f, 0.58f, 0.58f, 1.0f);

			    ImVec4 icon_col = ImLerp(text_uns, text_sel, a_fill);
			    ImVec4 text_col = ImLerp(text_uns, text_sel, a_fill);

				if (a_fill > 0.001f)
				{
					ImU32 col = ImColor(64/255.f, 163/255.f, 130/255.f, a_fill); // alpha scales with a_fill
					ImGui::GetForegroundDrawList()->AddRectFilled(
						ImVec2(bb.Min.x, bb.Min.y),
						ImVec2(bb.Min.x + a_fill * 6.0f, bb.Max.y),
						col, 7.0f);
				}

			    // --- Icon: center using icon font metrics
			    ImFont* iconFont  = ImGui::GetIO().Fonts->Fonts[3]; // your bold+icons font
			    ImFont* labelFont = ImGui::GetIO().Fonts->Fonts[1]; // your label font
			    ImGui::PushFont(iconFont);
			    float icon_y = centerTextY(iconFont, bb.Min.y, line_h);
			    // center icon within its reserved box horizontally
			    ImVec2 icon_size = ImGui::CalcTextSize(icon);
			    float icon_x = bb.Min.x + pad_x + (icon_box - icon_size.x) * 0.5f;
			    ImGui::GetForegroundDrawList()->AddText(ImVec2(icon_x, icon_y), ImColor(icon_col), icon);
			    ImGui::PopFont();

			    // --- Label: center using label font metrics
			    ImGui::PushFont(labelFont);
			    const float text_y = centerTextY(labelFont, bb.Min.y, line_h);
			    const float text_x = bb.Min.x + pad_x + icon_box + gap_x;
			    ImGui::GetForegroundDrawList()->AddText(ImVec2(text_x, text_y), ImColor(text_col), label);
			    ImGui::PopFont();

			    return pressed;
			}
		}
	}
}
