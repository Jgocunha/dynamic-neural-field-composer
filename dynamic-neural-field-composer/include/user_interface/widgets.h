#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "tools/math.h"

namespace dnf_composer::user_interface::widgets
{
	void renderHelpMarker(const char* desc);
	bool renderSidebarTab(const char* icon, const char* label, bool selected);

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
