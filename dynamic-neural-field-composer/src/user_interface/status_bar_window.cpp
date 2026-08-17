#include "user_interface/status_bar_window.h"

#include <array>

#include "application/application.h"
#include "user_interface/colour_registry.h"


namespace dnf_composer::user_interface
{
		StatusBarWindow::StatusBarWindow(const std::shared_ptr<Simulation> &simulation)
        :simulation(simulation)
		{}

		void StatusBarWindow::render()
		{
			const ImGuiViewport* vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - 28.0F), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, 28.0F), ImGuiCond_FirstUseEver);

			const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
				| ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoScrollWithMouse
				| ImGuiWindowFlags_NoDocking;

			const bool open = ImGui::Begin("##status", nullptr, flags | ImGuiWindowFlags_NoTitleBar);

			if (open)
			{
				drawContents();
			}

			ImGui::End();
		}

		void StatusBarWindow::drawContents() const
		{
			if (const float slackTop = (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) * 0.5F; slackTop > 0.0F) {
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slackTop);
}

			constexpr float sep = 20.0F;
			const bool running = simulation->isInitialized() && !simulation->isPaused();
			const bool paused  = simulation->isInitialized() &&  simulation->isPaused();

			ImVec4 dotColor = colour::kStatusDotStopped;
			const char* stateStr = "Stopped";
			if (running)
			{
				dotColor = colour::kStatusDotRunning;
				stateStr = "Running";
			}
			else if (paused)
			{
				dotColor = colour::kStatusDotPaused;
				stateStr = "Paused";
			}

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

			std::array<char, 32> fpsBuf{};
			std::array<char, 16> zoomBuf{};
			std::array<char, 32> memBuf{};
			std::snprintf(fpsBuf.data(),  fpsBuf.size(),  "%.1f", ImGui::GetIO().Framerate);
			std::snprintf(zoomBuf.data(), zoomBuf.size(), "%d%%",  static_cast<int>(Application::getUiScalePct()));
			std::snprintf(memBuf.data(),  memBuf.size(),  "%.1f MB", tools::utils::getProcessMemoryMb());

			const float rW =
				ImGui::CalcTextSize("FPS ").x + ImGui::CalcTextSize(fpsBuf.data()).x  + sep +
				ImGui::CalcTextSize("Zoom ").x + ImGui::CalcTextSize(zoomBuf.data()).x + sep +
				ImGui::CalcTextSize("Mem. ").x + ImGui::CalcTextSize(memBuf.data()).x +
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
			ImGui::TextUnformatted(fpsBuf.data());
			ImGui::PopFont();
			ImGui::SameLine(0, sep);

			ImGui::TextUnformatted("Zoom");
			ImGui::SameLine(0, 4);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::TextUnformatted(zoomBuf.data());
			ImGui::PopFont();
			ImGui::SameLine(0, sep);

			ImGui::TextUnformatted("Mem.");
			ImGui::SameLine(0, 4);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::TextUnformatted(memBuf.data());
			ImGui::PopFont();
		}
}
