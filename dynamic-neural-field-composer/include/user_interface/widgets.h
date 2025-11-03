#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "tools/math.h"

namespace dnf_composer::user_interface::widgets
{
	void renderHelpMarker(const char* desc);
	bool renderSidebarTab(const char* icon, const char* label, bool selected);
	bool renderIconTileButton(const char* id, const char* icon, const char* label,
						   const float tile, const float uiScale,
						   const ImU32 colBg, const ImU32 colHover, const ImU32 colActive,
						   const ImU32 colText, const ImU32 colLabel);
	inline void Spring(float weight = 1.0f, const float spacing = 0.0f)
	{
		ImGui::Dummy(ImVec2(spacing, 0));
		ImGui::SameLine(0, 0);
	}
	inline bool BeginHorizontal(const void* id = nullptr, float spacing = 0.0f)
	{
		if (id) ImGui::PushID(id);
		ImGui::BeginGroup();
		return true;
	}

	inline void EndHorizontal()
	{
		ImGui::EndGroup();
		ImGui::PopID();
	}
	inline bool BeginVertical(const void* id = nullptr, float spacing = 0.0f)
	{
		if (id) ImGui::PushID(id);
		ImGui::BeginGroup();
		return true;
	}

	inline void EndVertical()
	{
		ImGui::EndGroup();
		if (ImGui::GetID(NULL) != 0) ImGui::PopID();
	}
	class Card
	{
	private:
		std::string id;
		ImVec2 topLeftPosition;
		ImVec2 size;
		std::string title;
	public:
		Card(std::string  id, const ImVec2& topLeftPosition, const ImVec2& size, std::string  title);
		bool beginCard(const float& uiScale) const; // remember to end the card
		static void endCard();
	};
}
