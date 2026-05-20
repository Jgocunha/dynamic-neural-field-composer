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

	void LogWindow::renderPopUp()
	{
		const ImGuiViewport* vp = ImGui::GetMainViewport();
		const float          ui = ImGui::GetIO().FontGlobalScale;

		constexpr float kCollapsedW = 200.0f;
		constexpr float kCollapsedH = 50.0f;
		constexpr float kExpandedW  = 1020.0f;
		constexpr float kExpandedH  = 320.0f;
		constexpr float kBottomGap  = 100.0f;
		constexpr float kLeftGap    = 20.0f;

		const float w = s_expanded ? kExpandedW * ui : kCollapsedW * ui;
		const float h = s_expanded ? kExpandedH * ui : kCollapsedH * ui;

		const ImVec2 cornerPos(
			vp->WorkPos.x + kLeftGap,
			vp->WorkPos.y + vp->WorkSize.y - h - kBottomGap * ui);

		static bool s_needAnchor = true;

		ImGuiWindowFlags kFlags =
			ImGuiWindowFlags_NoDecoration    |
			ImGuiWindowFlags_NoResize        |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoDocking;

		if (!s_expanded)
		{
			ImGui::SetNextWindowPos(cornerPos, ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
			kFlags |= ImGuiWindowFlags_NoMove;
			s_needAnchor = true;
		}
		else
		{
			if (s_needAnchor)
			{
				ImGui::SetNextWindowPos(cornerPos, ImGuiCond_Always);
				s_needAnchor = false;
			}
			ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
		}

		ImGui::SetNextWindowBgAlpha(0.95f);

		if (!ImGui::Begin("##console_float", nullptr, kFlags))
		{
			ImGui::End();
			return;
		}

		if (!s_expanded)
		{
			const float slackY = (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) * 0.5f;
			if (slackY > 0.0f) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slackY);
			ImGui::AlignTextToFramePadding();

			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_TERMINAL);
			ImGui::PopFont();
			ImGui::PopStyleColor();

			ImGui::PushFont(g_BlackMediumFont);
			ImGui::SameLine(0, 6);
			ImGui::Text("Console");
			ImGui::PopFont();


			ImGui::PushFont(g_MediumIconsFont);
			const float chevW = ImGui::CalcTextSize(ICON_FA_CHEVRON_UP).x;
			ImGui::SameLine(ImGui::GetWindowWidth() - chevW - ImGui::GetStyle().WindowPadding.x - 2.0f);
			ImGui::AlignTextToFramePadding();
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			ImGui::TextUnformatted(ICON_FA_CHEVRON_UP);
			ImGui::PopStyleColor();
			ImGui::PopFont();

			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				s_expanded = true;
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_TERMINAL);
			ImGui::PopFont();
			ImGui::PopStyleColor();

			ImGui::SameLine(0, 6);
			ImGui::AlignTextToFramePadding();
			ImGui::PushFont(g_BlackMediumFont);
			ImGui::Text("Console");
			ImGui::PopFont();

			ImGui::PushFont(g_MediumIconsFont);
			const float chevW = ImGui::CalcTextSize(ICON_FA_CHEVRON_DOWN).x;
			ImGui::SameLine(ImGui::GetWindowWidth() - chevW - ImGui::GetStyle().WindowPadding.x - 2.0f);
			ImGui::AlignTextToFramePadding();
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			ImGui::TextUnformatted(ICON_FA_CHEVRON_DOWN);
			ImGui::PopStyleColor();
			ImGui::PopFont();

			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				s_expanded = false;

			ImGui::Separator();
			renderContent();
		}

		ImGui::End();
	}

} 