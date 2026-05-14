#include "user_interface/log_window.h"


namespace dnf_composer::user_interface
{
	LogWindow::LogWindow()
	{
   		isWindowActive = true;
		clean();
	}

	void LogWindow::renderContent()
	{
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &autoScroll);
			ImGui::EndPopup();
		}

		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
			clean();
		ImGui::SameLine();
		if (ImGui::Button("Copy"))
			ImGui::LogToClipboard();
		ImGui::SameLine();
		filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_ChildBg,    ImVec4(0.07f, 0.07f, 0.07f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.07f, 0.07f, 0.07f, 1.0f));
		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
			ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushFont(g_MonoMediumFont);
			for (const auto& [message, color] : logs)
			{
				if (filter.PassFilter(message.c_str()))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, color);
					ImGui::TextEx(message.c_str());
					ImGui::PopStyleColor();
				}
			}
			ImGui::PopFont();
			ImGui::PopStyleVar();

			if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
	}


	void LogWindow::addLog(const ImVec4& color, const char* fmt, ...)
	{
   		va_list args;
		va_start(args, fmt);
		char buffer[1024];
		vsnprintf(buffer, IM_ARRAYSIZE(buffer), fmt, args);
		buffer[IM_ARRAYSIZE(buffer) - 1] = '\0';
		 va_end(args);
		logs.push_back({ buffer, color });
	}

	void LogWindow::draw()
	{
		ImGui::PushFont(g_BlackLargeFont);
		const bool open = ImGui::Begin("Logs", nullptr, imgui_kit::getGlobalWindowFlags());
		ImGui::PopFont();

        if (!open)
        {
            ImGui::End();
            return;
        }
       renderContent();
		ImGui::End();
	}

} 