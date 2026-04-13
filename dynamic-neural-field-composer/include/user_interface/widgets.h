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
						   float tile, float uiScale,
						   ImU32 colBg, ImU32 colHover, ImU32 colActive,
						   ImU32 colText, ImU32 colLabel);
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
		[[nodiscard]] bool beginCard(const float& uiScale) const; // remember to end the card
		static void endCard();
	};
}
