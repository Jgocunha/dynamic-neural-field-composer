#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "tools/math.h"

namespace dnf_composer::user_interface
{
	namespace widgets
	{
		void renderHelpMarker(const char* desc);
		bool tab(const char* icon, const char* label, bool selected);
	}

	// helper functions for imgui
	static float centerTextY(const ImFont* font, const float& y0, const float& h)
	{
		// In Dear ImGui: height = Ascent - Descent, AddText expects pos.y == top
		const float text_h = font->Ascent - font->Descent;
		return y0 + (h - text_h) * 0.5f;
	}
}
