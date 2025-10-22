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
				if (window->SkipItems)
					return false;

				ImGuiContext& g = *GImGui;
				const ImGuiStyle& style = g.Style;
				const ImGuiID id = window->GetID(std::string(icon + std::string(label)).c_str());
				const ImVec2 label_size = ImGui::CalcTextSize(icon);

				ImVec2 pos = window->DC.CursorPos;
				ImVec2 size = { 160, 30 };

				const ImRect bb(pos, ImVec2(pos.x + 162, pos.y + 30));
				ImGui::ItemSize(size, 0);
				if (!ImGui::ItemAdd(bb, id))
					return false;

				bool hovered, held;
				bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

				if (hovered || held)
					ImGui::SetMouseCursor(7);

				float t = selected ? 1.0f : 0.0f;
				float deltatime = 1.5f * ImGui::GetIO().DeltaTime;
				static std::map<ImGuiID, float> hover_animation;
				auto it_hover = hover_animation.find(id);
				if (it_hover == hover_animation.end())
				{
					hover_animation.insert({ id, 0.f });
					it_hover = hover_animation.find(id);
				}
				it_hover->second = ImClamp(it_hover->second + (0.2f * ImGui::GetIO().DeltaTime * (hovered || ImGui::IsItemActive() ? 1.f : -1.f)), 0.0f, 0.15f);
				it_hover->second *= min(ImGui::GetStyle().Alpha * 1.2, 1.f);

				static std::map<ImGuiID, float> filled_animation;
				auto it_filled = filled_animation.find(id);
				if (it_filled == filled_animation.end())
				{
					filled_animation.insert({ id, 0.f });
					it_filled = filled_animation.find(id);
				}
				it_filled->second = ImClamp(it_filled->second + (2.55f * ImGui::GetIO().DeltaTime * (selected ? 1.f : -1.0f)), it_hover->second, 1.f);
				it_filled->second *= min(ImGui::GetStyle().Alpha * 1.2, 1.f);

				static std::map<ImGuiID, float> fill_animation;
				auto it_fill = fill_animation.find(id);
				if (it_fill == fill_animation.end())
				{
					fill_animation.insert({ id, 0.f });
					it_fill = fill_animation.find(id);
				}

				it_fill->second = ImClamp(it_filled->second + (1.75f * ImGui::GetIO().DeltaTime * (selected ? 1.f : -1.0f)), it_hover->second, 1.f);
				it_fill->second *= min( ImGui::GetStyle().Alpha * 1.2, 1.f);

				constexpr ImVec4 highlight_color = ImVec4(64.0f / 255.0f, 163.0f / 255.0f, 130.0f / 255.0f, 1.0f);

				ImVec4 icon_color = ImLerp(
					imgui_kit::colours::Gray, // not-selected
					highlight_color, //selected
					it_filled->second);
				ImVec4 text_color = ImLerp(
					imgui_kit::colours::Gray, // not-selected
					highlight_color, //selected
					it_filled->second);

				ImGui::GetForegroundDrawList()->AddRectFilled(
					ImVec2(bb.Min.x + 5, bb.Min.y - 5),
					ImVec2(bb.Min.x + 5 + it_filled->second * 6, bb.Max.y),
					ImColor((int)(highlight_color.x * 255.0f), (int)(highlight_color.y * 255.0f), (int)(highlight_color.z * 255.0f), (int)(it_filled->second * 255.0f)),
//					ImColor(105, 125, 203, int(255 * it_filled->second)),
					7);
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);// bold font
				ImGui::GetForegroundDrawList()->AddText(
					ImVec2(bb.Min.x + (32 - label_size.x / 2), bb.Min.y + (15 - label_size.y / 2)),
					ImColor(icon_color),
					icon);

				ImGui::GetForegroundDrawList()->AddText(ImVec2(bb.Min.x + 60, bb.Min.y + (15 -  ImGui::CalcTextSize(label).y / 2)),
					ImColor(text_color),
					label);
				ImGui::PopFont();

				return pressed;
			}

			bool settingsButton(const char* label)
			{
				ImGuiWindow* window = ImGui::GetCurrentWindow();
				if (window->SkipItems)
					return false;

				ImGuiContext& g = *GImGui;
				const ImGuiStyle& style = g.Style;
				const ImGuiID id = window->GetID(label);
				const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);


				ImVec2 pos = window->DC.CursorPos;
				ImVec2 size = ImGui::CalcItemSize(ImVec2(15, 15), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

				ImVec2 bb_min = pos;
				ImVec2 bb_max(pos.x + size.x, pos.y + size.y);
				const ImRect bb(bb_min, bb_max);
				ImGui::ItemSize(size, style.FramePadding.y);
				if (!ImGui::ItemAdd(bb, id))
					return false;

				bool hovered, held;
				bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

				if (hovered || held)
					ImGui::SetMouseCursor(7);
				//ImGui::PushFont(iconfont);
				ImGui::RenderText(ImVec2(bb.Min.x + size.x / 2 - label_size.x / 2, bb.Min.y + size.y / 2 - label_size.y / 2), label);
				//ImGui::PopFont();
				return pressed;
			}

			bool checkbox(const char* label, bool* value)
			{
				ImGuiWindow* window = ImGui::GetCurrentWindow();
			    if (window->SkipItems)
			        return false;

			    ImGuiContext& g = *GImGui;
			    const ImGuiStyle& style = g.Style;
			    const ImGuiID id = window->GetID(label);
			    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

				const ImVec2 cur = window->DC.CursorPos;
				ImVec2 check_min(cur.x - 0.0f, cur.y - 0.0f);
				ImVec2 check_max(
					cur.x - 5.0f + (label_size.y + style.FramePadding.y * 2.0f),
					cur.y - 5.0f + (label_size.y + style.FramePadding.y * 2.0f)
				);
				ImRect check_bb(check_min, check_max);
				ImGui::ItemSize(check_bb, style.FramePadding.y);

			    ImRect total_bb = check_bb;
			    if (label_size.x > 0)
			        ImGui::SameLine(0, style.ItemInnerSpacing.x);
				ImVec2 text_min(cur.x,               cur.y + style.FramePadding.y);
				ImVec2 text_max(cur.x + label_size.x, cur.y + style.FramePadding.y + label_size.y);
				ImRect text_bb(text_min, text_max);
			    //const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y));
			    if (label_size.x > 0)
			    {
			        ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
			        total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
			    }

			    if (!ImGui::ItemAdd(total_bb, id))
			        return false;

			    bool hovered, held;
			    bool pressed = ImGui::ButtonBehavior(check_bb, id, &hovered, &held, 0);
			    if (pressed)
			    {
			        *value = !(*value);
			        ImGui::MarkItemEdited(id);
			    }
			    float deltatime = 4.5f * ImGui::GetIO().DeltaTime;

			    const ImVec4 text_dis = ImVec4(189 / 255.f, 189 / 255.f, 194 / 255.f, 1.f);;
			    const ImVec4 text_act = ImVec4(50 / 255.f, 50 / 255.f, 55 / 255.f, 1.f);
			    static std::map<ImGuiID, ImVec4> text_animation;
			    auto it_text = text_animation.find(id);
			    if (it_text == text_animation.end())
			    {
			        text_animation.insert({ id, text_dis });
			        it_text = text_animation.find(id);
			    }
			    if (*value)
			    {
			        ImVec4 to = text_act;
			        if (it_text->second.x != to.x)
			        {
			            if (it_text->second.x < to.x)
			                it_text->second.x = ImMin(it_text->second.x + deltatime, to.x);
			            else if (it_text->second.x > to.x)
			                it_text->second.x = ImMax(to.x, it_text->second.x - deltatime);
			        }

			        if (it_text->second.y != to.y)
			        {
			            if (it_text->second.y < to.y)
			                it_text->second.y = ImMin(it_text->second.y + deltatime, to.y);
			            else if (it_text->second.y > to.y)
			                it_text->second.y = ImMax(to.y, it_text->second.y - deltatime);
			        }

			        if (it_text->second.z != to.z)
			        {
			            if (it_text->second.z < to.z)
			                it_text->second.z = ImMin(it_text->second.z + deltatime, to.z);
			            else if (it_text->second.z > to.z)
			                it_text->second.z = ImMax(to.z, it_text->second.z - deltatime);
			        }
			    }
			    else
			    {
			        ImVec4 to = text_dis;
			        if (it_text->second.x != to.x)
			        {
			            if (it_text->second.x < to.x)
			                it_text->second.x = ImMin(it_text->second.x + deltatime, to.x);
			            else if (it_text->second.x > to.x)
			                it_text->second.x = ImMax(to.x, it_text->second.x - deltatime);
			        }

			        if (it_text->second.y != to.y)
			        {
			            if (it_text->second.y < to.y)
			                it_text->second.y = ImMin(it_text->second.y + deltatime, to.y);
			            else if (it_text->second.y > to.y)
			                it_text->second.y = ImMax(to.y, it_text->second.y - deltatime);
			        }

			        if (it_text->second.z != to.z)
			        {
			            if (it_text->second.z < to.z)
			                it_text->second.z = ImMin(it_text->second.z + deltatime, to.z);
			            else if (it_text->second.z > to.z)
			                it_text->second.z = ImMax(to.z, it_text->second.z - deltatime);
			        }
			    }

			    static std::map<ImGuiID, float> filled_animation;
			    auto it_filled = filled_animation.find(id);
			    if (it_filled == filled_animation.end())
			    {
			        filled_animation.insert({ id, 0.f });
			        it_filled = filled_animation.find(id);
			    }
			    if (*value && it_filled->second >= 1.f)
			        it_filled->second = ImClamp(it_filled->second - 2.2f * ImGui::GetIO().DeltaTime, 0.5f, 1.f);
			    else
			        it_filled->second = ImClamp(it_filled->second + (13.2f * ImGui::GetIO().DeltaTime * ((*value) ? 1.f : -1.f)), 0.0f, 1.f);

				ImDrawList* draw = ImGui::GetWindowDrawList();
			    draw->AddRect(check_bb.Min, check_bb.Max, ImColor(206, 206, 211), 2);

			    ImGui::RenderFrame(ImVec2(check_bb.Min.x + 3.5f, check_bb.Min.y + 3.5f), ImVec2(check_bb.Max.x - 2.5f, check_bb.Max.y - 2.5f),
			    	ImGui::GetColorU32(ImVec4(50 / 255.f, 50 / 255.f, 55 / 255.f, it_filled->second)), true, 2);

			    ImGui::PushStyleColor(ImGuiCol_Text, it_text->second);
			    if (label_size.x > 0.0f)
			        ImGui::RenderText(ImVec2(check_bb.Min.x + 25, check_bb.Min.y + 1), label);
			    ImGui::PopStyleColor();
			    // ImGui::Spacing(30);
				ImGui::Dummy(ImVec2(0.0f, 30.0f)); // adds 30px vertical space
			    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
			    return pressed;
			}

		}
	}
}
