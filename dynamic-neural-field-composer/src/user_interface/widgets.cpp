#include "user_interface/widgets.h"


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
		}
	}
}
