#include "user_interface/log_window.h"

#include <array>
#include <imgui-platform-kit/colour_palette.h>

#include "user_interface/colour_registry.h"

namespace dnf_composer::user_interface
{
	ImVec4 getLogLevelColorCodeGui(const tools::logger::LogLevel level)
	{
		ImVec4 currentTextColor = imgui_kit::colours::Gray;
		if (ImGui::GetCurrentContext() != nullptr)
		{
			const ImGuiStyle& style = ImGui::GetStyle();
			currentTextColor = style.Colors[ImGuiCol_Text];
		}

		using tools::logger::LogLevel;
		switch (level)
		{
		case LogLevel::DEBUG:     return imgui_kit::colours::Green;
		case LogLevel::INFO:      return imgui_kit::colours::White;
		case LogLevel::WARNING:   return imgui_kit::colours::Yellow;
		case LogLevel::ERROR:
		case LogLevel::FATAL:     return imgui_kit::colours::Red;
		default:                  return currentTextColor;
		}
	}

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

		if (ImGui::Button("Options")) {
			ImGui::OpenPopup("Options");
}
		ImGui::SameLine();
		if (ImGui::Button("Clear")) {
			clean();
}
		ImGui::SameLine();
		if (ImGui::Button("Copy")) {
			ImGui::LogToClipboard();
}
		ImGui::SameLine();
		filter.Draw("Filter", -100.0F);

		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_ChildBg,    colour::kLogConsoleBackground);
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, colour::kLogConsoleBackground);
		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), 0, ImGuiWindowFlags_None))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::PushTextWrapPos(0.0F);
			{
				std::lock_guard lock(logsMutex);
				for (const auto& entry : logs)
				{
					if (filter.PassFilter(entry.message.c_str()))
					{
						const ImVec4 color = entry.resolveColorFromLevel
							? getLogLevelColorCodeGui(entry.level) : entry.color;
						ImGui::PushStyleColor(ImGuiCol_Text, color);
						ImGui::TextEx(entry.message.c_str());
						ImGui::PopStyleColor();
					}
				}
			}
			ImGui::PopTextWrapPos();
			ImGui::PopFont();
			ImGui::PopStyleVar();

			if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0F);
}
		}
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
	}


	void LogWindow::addLog(const ImVec4& color, const char* fmt, ...)
	{
   		va_list args;
		va_start(args, fmt);
		std::array<char, 1024> buffer{};
		vsnprintf(buffer.data(), buffer.size(), fmt, args);
		buffer.back() = '\0';
		 va_end(args);
		std::lock_guard lock(logsMutex);
		logs.push_back({ buffer.data(), color, false, tools::logger::LogLevel::INFO });
	}

	void LogWindow::addLog(const tools::logger::LogLevel level, const char* fmt, ...)
	{
   		va_list args;
		va_start(args, fmt);
		std::array<char, 1024> buffer{};
		vsnprintf(buffer.data(), buffer.size(), fmt, args);
		buffer.back() = '\0';
		 va_end(args);
		std::lock_guard lock(logsMutex);
		logs.push_back({ buffer.data(), ImVec4{}, true, level });
	}

	void LogWindow::draw()
	{
		const ImGuiViewport* vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.3F, vp->WorkPos.y + vp->WorkSize.y * 0.6F), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(700.0F, 300.0F), ImGuiCond_FirstUseEver);
		const bool open = ImGui::Begin("Log##log_window", &isWindowActive,
			imgui_kit::getGlobalWindowFlags() | ImGuiWindowFlags_NoTitleBar);
		if (open)
		{
			const float startY = ImGui::GetCursorPosY();
			const float yOff = (g_BlackLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5F;
			ImGui::SetCursorPosY(startY + yOff);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_TERMINAL);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 8.0F);
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