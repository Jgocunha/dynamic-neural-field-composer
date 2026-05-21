#include "user_interface/static_layout.h"


extern ImFont* g_BlackLargeFont;
extern ImFont* g_BlackMediumFont;
extern ImFont* g_BlackSmallFont;
extern ImFont* g_MonoMediumFont;
extern ImFont* g_MediumIconsFont;
extern ImFont* g_SmallIconsFont;

namespace dnf_composer::user_interface
{
	// ── Column widths (fixed px at a UI scale = 1.0, scale with zoom) ──────────
	static constexpr float kColABase   = 515.0F;  // Simulation Control
	static constexpr float kColBBase   = 360.0F;  // Element Control (rightmost)
	static constexpr float kStatusBarH = 40.0F;   // status bar height
	static constexpr float kTopBarH    = 70.0F;   // top control bar height
	static constexpr float kMargin     = 6.0F;
	static constexpr float kRounding   = 8.0F;

	static bool beginPanelFixed(const char* id, const ImVec2 pos, const ImVec2 size)
	{
		ImGui::SetCursorScreenPos(pos);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0F);
		return ImGui::BeginChild(id, size, static_cast<int>(true),
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	}

	static void endPanel()
	{
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	StaticLayoutWindow::StaticLayoutWindow(
		const std::shared_ptr<Simulation>&    simulation,
		const std::shared_ptr<Visualization>& visualization)
		: simulation(simulation), visualization(visualization)
	{
		controlBarWindow = std::make_unique<ControlBarWindow>(simulation);
		simulationWindow = std::make_unique<SimulationWindow>(simulation);
		elementWindow    = std::make_unique<ElementWindow>(simulation);
		nodeGraphWindow  = std::make_unique<NodeGraphWindow>(simulation);
		plotsWindow		 = std::make_unique<PlotsWindow>(visualization);
		statusBarWindow  = std::make_unique<StatusBarWindow>(simulation);
		logWindow        = std::make_unique<LogWindow>();
	}

	void StaticLayoutWindow::render()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;

		const ImGuiViewport* vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowViewport(vp->ID);
		ImGui::SetNextWindowPos(vp->WorkPos,  ImGuiCond_Always);
		ImGui::SetNextWindowSize(vp->WorkSize, ImGuiCond_Always);

		constexpr ImGuiWindowFlags root_flags =
			ImGuiWindowFlags_NoDecoration          |
			ImGuiWindowFlags_NoMove                |
			ImGuiWindowFlags_NoResize              |
			ImGuiWindowFlags_NoCollapse            |
			ImGuiWindowFlags_NoSavedSettings       |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoScrollbar           |
			ImGuiWindowFlags_NoBackground          |
			ImGuiWindowFlags_NoDocking;

		if (ImGui::Begin("##static_root", nullptr, root_flags))
		{
			renderWindows();
		}

		ImGui::End();
	}

	void StaticLayoutWindow::renderWindows() const
	{
		drawPanels();
		plotsWindow->render();
	}

	void StaticLayoutWindow::drawPanels() const
	{
		const ImVec2 origin = ImGui::GetWindowPos();
		const ImVec2 total  = ImGui::GetWindowSize();
		constexpr float  m      = kMargin;
		const float  scale  = ImGui::GetIO().FontGlobalScale;

		const float colAW   = kColABase * scale;
		const float colBW   = kColBBase * scale;
		const float statusH = kStatusBarH * scale;
		const float topBarH = kTopBarH * scale;

		// Node graph (col C) fills the remaining horizontal space.
		const float colCW = total.x - colAW - colBW - m * 4.0f;

		// y0: top bar row; y1: three column row
		const float y0    = origin.y + m;
		const float y1    = y0 + topBarH + m;

		// Full height of the three main columns; leave room for top bar + status bar + margins.
		const float fullH = total.y - m * 2.0f - statusH - m - topBarH - m;

		// Column x-positions: A (sim control) | C (node graph) | B (element control)
		const float xA = origin.x + m;
		const float xC = xA + colAW + m;
		const float xB = xC + colCW + m;

		drawPanelControlBar({origin.x + m, y0},      {total.x - m * 2.0f, topBarH});
		drawPanelSimulation({xA, y1},                {colAW, fullH});
		drawPanelElement   ({xB, y1},                {colBW, fullH});
		drawPanelNodeGraph ({xC, y1},                {colCW, fullH});
		drawPanelStatusBar ({origin.x + m, y1 + fullH + m}, {total.x - m * 2.0f, statusH});
	}

	void StaticLayoutWindow::drawPanelControlBar(const ImVec2 pos, const ImVec2 size) const
	{
		if (beginPanelFixed("##sl_control_bar", pos, size))
		{
			const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
						| ImGuiWindowFlags_NoScrollbar
						| ImGuiWindowFlags_NoScrollWithMouse
						| ImGuiWindowFlags_NoSavedSettings;

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			if (ImGui::BeginChild("##ng_graph_c", ImVec2(0, 0), false,
				flags))
			{
				controlBarWindow->drawContents();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
		endPanel();
	}

	void StaticLayoutWindow::drawPanelSimulation(const ImVec2 pos, const ImVec2 size) const
	{
		if (beginPanelFixed("##sl_sim", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Simulation Control");
			ImGui::PopFont();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			if (ImGui::BeginChild("##sl_sim_scroll", ImVec2(0, 0), false,
				ImGuiWindowFlags_NoSavedSettings))
			{
				simulationWindow->renderAddElementCard();
				ImGui::Spacing();
				simulationWindow->renderRemoveElementCard();
				ImGui::Spacing();
				simulationWindow->renderSetInteractionCard();
				ImGui::Spacing();
				simulationWindow->renderLogElementParametersCard();
				ImGui::Spacing();
				simulationWindow->renderExportElementComponentCard();
				ImGui::Spacing();
				simulationWindow->renderMonitoringCard();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
		endPanel();
	}

	void StaticLayoutWindow::drawPanelElement(const ImVec2 pos, const ImVec2 size) const
	{
		if (beginPanelFixed("##sl_elem", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Element Control");
			ImGui::PopFont();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			elementWindow->renderElementControlCard();
			ImGui::PopStyleColor();
		}
		endPanel();
	}

	void StaticLayoutWindow::drawPanelNodeGraph(const ImVec2 pos, const ImVec2 size) const
	{
		if (beginPanelFixed("##sl_node", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Node Graph");
			ImGui::PopFont();
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			if (ImGui::BeginChild("##ng_graph_c", ImVec2(0, 0), false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				nodeGraphWindow->renderEmbedded();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
		endPanel();
	}

	void StaticLayoutWindow::drawPanelStatusBar(const ImVec2 pos, const ImVec2 size) const
	{
		if (beginPanelFixed("##sl_status", pos, size))
		{
			const float slackSt = (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) * 0.5f;
			if (slackSt > 0.0f) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slackSt);
			const std::string simId = simulation->getIdentifier();
			const bool running = simulation->isInitialized() && !simulation->isPaused();
			const bool paused  = simulation->isInitialized() &&  simulation->isPaused();

			const ImVec4 dotColor = running ? ImVec4(0.20F, 0.75F, 0.20F, 1.0F)
			                      : paused  ? ImVec4(0.90F, 0.70F, 0.10F, 1.0F)
			                                : ImVec4(0.75F, 0.20F, 0.20F, 1.0F);
			const char* stateStr  = running ? "Running" : paused ? "Paused" : "Stopped";

			// ── State dot + state label ────────────────────────────────────────
			ImGui::TextColored(dotColor, "\xe2\x97\x8f");  // U+25CF BLACK CIRCLE
			ImGui::SameLine(0, 4);
			ImGui::TextUnformatted(stateStr);
			ImGui::SameLine(0, 14);

			// ── Δt ────────────────────────────────────────────────────────────
			ImGui::TextUnformatted("\xce\x94t");
			ImGui::SameLine(0, 4);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::Text("%.1f", simulation->getDeltaT());
			ImGui::PopFont();
			ImGui::SameLine(0, 14);

			// ── Sim. time ─────────────────────────────────────────────────────
			ImGui::TextUnformatted("Ticks");
			ImGui::SameLine(0, 4);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::Text("%.0f", simulation->getT());
			ImGui::PopFont();
			ImGui::SameLine(0, 14);

			// ── Real time ─────────────────────────────────────────────────────
			const long long totalUs = simulation->getTotalRunDuration().count() / 1000LL;
			const long long hh  = totalUs / 3'600'000'000LL;
			const long long mm  = (totalUs % 3'600'000'000LL) / 60'000'000LL;
			const long long ss  = (totalUs % 60'000'000LL)    / 1'000'000LL;
			const long long ms  = (totalUs % 1'000'000LL)     / 1'000LL;
			ImGui::TextUnformatted("Real time");
			ImGui::SameLine(0, 4);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::Text("%lldh%lldm%llds%lldms", hh, mm, ss, ms);
			ImGui::PopFont();
			ImGui::SameLine(0, 14);

			char fpsBuf[32];
			char zoomBuf[16];
			char memBuf[32];
			std::snprintf(fpsBuf,  sizeof(fpsBuf),  "%.1f", ImGui::GetIO().Framerate);
			std::snprintf(zoomBuf, sizeof(zoomBuf), "%d%%",  static_cast<int>(Application::getUiScalePct()));
			std::snprintf(memBuf,  sizeof(memBuf),  "%.1f MB", tools::utils::getProcessMemoryMb());

			constexpr float sep = 14.0F;
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
		endPanel();
	}
}
