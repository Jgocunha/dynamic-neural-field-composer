#include "user_interface/status_bar_window.h"

#include "application/application.h"


namespace dnf_composer
{
	extern ImFont* g_BlackLargeFont;
	extern ImFont* g_BlackMediumFont;
	extern ImFont* g_BlackSmallFont;
	extern ImFont* g_MonoMediumFont;
	extern ImFont* g_MediumIconsFont;
	extern ImFont* g_SmallIconsFont;

	namespace user_interface
	{
		StatusBarWindow::StatusBarWindow(const std::shared_ptr<Simulation> &simulation)
        :simulation(simulation)
    {}

    void StatusBarWindow::render()
    {
        const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::PushFont(g_BlackLargeFont);
        const bool open = ImGui::Begin("Status", nullptr, flags);
        ImGui::PopFont();

        if (open)
        {
            drawContents();
        }

        ImGui::End();
    }

    void StatusBarWindow::drawContents() const
    {
        if (const float slackTop = (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) * 0.5f; slackTop > 0.0f)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slackTop);

		constexpr float sep = 20.0F;
		const bool running = simulation->isInitialized() && !simulation->isPaused();
		const bool paused  = simulation->isInitialized() &&  simulation->isPaused();

		const ImVec4 dotColor = running ? imgui_kit::colours::Green
							  : paused  ? imgui_kit::colours::Yellow
										: imgui_kit::colours::Red;
		const char* stateStr  = running ? "Running" : paused ? "Paused" : "Stopped";

		ImGui::TextColored(dotColor, "\xe2\x97\x8f");  // U+25CF BLACK CIRCLE
		ImGui::SameLine(0, 4);
		ImGui::TextUnformatted(stateStr);
		ImGui::SameLine(0, sep);

		ImGui::TextUnformatted("\xce\x94t");
		ImGui::SameLine(0, 4);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::Text("%.2f", simulation->getDeltaT());
		ImGui::PopFont();
		ImGui::SameLine(0, sep);

		ImGui::TextUnformatted("Ticks");
		ImGui::SameLine(0, 4);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::Text("%.0f", simulation->getT());
		ImGui::PopFont();
		ImGui::SameLine(0, sep);

		const long long totalUs = simulation->getTotalRunDuration().count() / 1000LL;
		const long long hh  = totalUs / 3'600'000'000LL;
		const long long mm  = (totalUs % 3'600'000'000LL) / 60'000'000LL;
		const long long ss  = (totalUs % 60'000'000LL)    / 1'000'000LL;
		const long long ms  = (totalUs % 1'000'000LL)     / 1'000LL;
		ImGui::TextUnformatted("Real time");
		ImGui::SameLine(0, 4);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::Text("%lldh %lldm %llds %lldms", hh, mm, ss, ms);
		ImGui::PopFont();
		ImGui::SameLine(0, sep);

		char fpsBuf[32];
		char zoomBuf[16];
		char memBuf[32];
		std::snprintf(fpsBuf,  sizeof(fpsBuf),  "%.1f", ImGui::GetIO().Framerate);
		std::snprintf(zoomBuf, sizeof(zoomBuf), "%d%%",  static_cast<int>(Application::getUiScalePct()));
		std::snprintf(memBuf,  sizeof(memBuf),  "%.1f MB", tools::utils::getProcessMemoryMb());

		const float rW =
			ImGui::CalcTextSize("FPS ").x + ImGui::CalcTextSize(fpsBuf).x  + sep +
			ImGui::CalcTextSize("Zoom ").x + ImGui::CalcTextSize(zoomBuf).x + sep +
			ImGui::CalcTextSize("Mem. ").x + ImGui::CalcTextSize(memBuf).x +
				ImGui::GetStyle().WindowPadding.x;

		const float rightX = ImGui::GetWindowWidth() - rW;
		if (const float curX = ImGui::GetCursorPosX(); rightX > curX + sep) {
			ImGui::SameLine(rightX);
		} else {
			ImGui::SameLine(0, sep);
		}

		ImGui::TextUnformatted("FPS");
		ImGui::SameLine(0, 4);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::TextUnformatted(fpsBuf);
		ImGui::PopFont();
		ImGui::SameLine(0, sep);

		ImGui::TextUnformatted("Zoom");
		ImGui::SameLine(0, 4);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::TextUnformatted(zoomBuf);
		ImGui::PopFont();
		ImGui::SameLine(0, sep);

		ImGui::TextUnformatted("Mem.");
		ImGui::SameLine(0, 4);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::TextUnformatted(memBuf);
		ImGui::PopFont();
		}
    }
}
