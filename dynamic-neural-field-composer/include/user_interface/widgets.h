#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "tools/math.h"

namespace dnf_composer
{
	namespace user_interface
	{
		namespace widgets
		{
			void renderHelpMarker(const char* desc);
			bool tab(const char* label, bool selected, ImVec2 size = ImVec2(15, 15));
			bool settingsButton(const char* label);
			bool sub(const char* label, bool selected);
			bool checkbox(const char* label, bool* value);
		}
	}
}