#include "user_interface/log_window.h"


namespace dnf_composer::user_interface
{
	LogWindow::LogWindow()
	{
   		isWindowActive = false;
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
		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_None))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::PushTextWrapPos(0.0f);
			for (const auto& [message, color] : logs)
			{
				if (filter.PassFilter(message.c_str()))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, color);
					ImGui::TextEx(message.c_str());
					ImGui::PopStyleColor();
				}
			}
			ImGui::PopTextWrapPos();
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
		const bool open = ImGui::Begin("##log_window", &isWindowActive,
			imgui_kit::getGlobalWindowFlags() | ImGuiWindowFlags_NoTitleBar);
		if (open)
		{
			const float startY = ImGui::GetCursorPosY();
			const float yOff = (g_BlackLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5f;
			ImGui::SetCursorPosY(startY + yOff);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_TERMINAL);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 8.0f);
			ImGui::SetCursorPosY(startY);
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::TextUnformatted("Logs");
			ImGui::PopFont();
			ImGui::Separator();
			renderContent();
		}
		ImGui::End();
	}
} 