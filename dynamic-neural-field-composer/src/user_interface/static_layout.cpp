#include "user_interface/static_layout_window.h"


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
	static constexpr float kTopBarH    = 50.0F;   // top control bar height
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
		simulationWindow = std::make_unique<SimulationWindow>(simulation);
		elementWindow    = std::make_unique<ElementWindow>(simulation);
		nodeGraphWindow  = std::make_unique<NodeGraphWindow>(simulation);
		plotsWindow		= std::make_unique<PlotsWindow>(visualization);
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

		panelTopBar    ({origin.x + m, y0},      {total.x - m * 2.0f, topBarH});
		panelSimulation({xA, y1},                {colAW, fullH});
		panelNodeGraph ({xC, y1},                {colCW, fullH});
		panelElement   ({xB, y1},                {colBW, fullH});
		panelStatusBar ({origin.x + m, y1 + fullH + m}, {total.x - m * 2.0f, statusH});
	}

	void StaticLayoutWindow::panelTopBar(const ImVec2 pos, const ImVec2 size) const
	{
		const float ui    = ImGui::GetIO().FontGlobalScale;
		const float btnSz = size.y - 16.0f * ui;

		if (beginPanelFixed("##sl_top", pos, size))
		{
			const float slackTop = (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) * 0.5f;
			if (slackTop > 0.0f) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slackTop);
			ImGui::AlignTextToFramePadding();
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_WAVE_SQUARE);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 8);

			static std::array<char, 128> idBuf{};
			std::snprintf(idBuf.data(), idBuf.size(), "%s",
			              simulation->getUniqueIdentifier().c_str());
			ImGui::SetNextItemWidth(225.0f * ui);
			ImGui::PushFont(g_BlackMediumFont);
			const bool idEdited = ImGui::InputText("##top_sim_id", idBuf.data(), idBuf.size(),
				ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
			if (idEdited || ImGui::IsItemDeactivatedAfterEdit())
				simulation->setUniqueIdentifier(std::string(idBuf.data()));
			ImGui::SameLine(0, 2);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(".dnf");
			ImGui::PopFont();
			ImGui::SameLine(0, 40);

			renderTopBarLeft(ui, btnSz);
			ImGui::SameLine(0, 40);

			ImGui::AlignTextToFramePadding();
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_CLOCK);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 6);
			static double editDt = simulation->getDeltaT();
			ImGui::SetNextItemWidth(60.0f * ui);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::InputDouble("##top_dt", &editDt, 0.0, 0.0, "%.2f");
			ImGui::PopFont();
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				if (std::isfinite(editDt) && editDt > 0.0)
					simulation->setDeltaT(editDt);
				editDt = simulation->getDeltaT();
			}
			ImGui::SameLine(0, 4);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("\xce\x94t");

			renderTopBarRight(ui, btnSz);
		}
		endPanel();
	}

	void StaticLayoutWindow::renderTopBarLeft(const float ui, const float btnSz) const
	{
		const ImVec2 bSz(btnSz, btnSz);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0, 0));
		ImGui::PushFont(g_LargeIconsFont);

		if (ImGui::Button(ICON_FA_PLAY        "##top_play",   bSz)) simulation->init();
		ImGui::SameLine(0, 6);
		if (ImGui::Button(ICON_FA_PAUSE       "##top_pause",  bSz)) simulation->pause();
		ImGui::SameLine(0, 6);
		if (ImGui::Button(ICON_FA_FORWARD_FAST"##top_resume", bSz)) simulation->resume();
		ImGui::SameLine(0, 6);
		if (ImGui::Button(ICON_FA_STOP        "##top_stop",   bSz)) simulation->close();

		ImGui::PopFont();
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(1);
	}

	void StaticLayoutWindow::renderTopBarRight(const float ui, const float btnSz) const
	{
		static int   tickCount = 1000;
		static bool  runNTicks = false;
		static int   startTick = 0;
		static float msCount   = 1000.0f;
		static bool  runForMs  = false;
		static std::chrono::steady_clock::time_point runMsStart;

		if (runNTicks && simulation->getT() >= startTick + tickCount)
		{
			runNTicks = false;
			simulation->pause();
		}
		if (runForMs)
		{
			const auto elapsed = std::chrono::duration<float, std::milli>(
				std::chrono::steady_clock::now() - runMsStart).count();
			if (elapsed >= msCount)
			{
				runForMs = false;
				simulation->pause();
			}
		}

		constexpr auto kFill   = ImVec4(0.96f, 0.98f, 0.99f, 1.0f);
		constexpr auto kHover  = ImVec4(0.90f, 0.97f, 0.94f, 1.0f);
		constexpr auto kActive = ImVec4(0.85f, 0.96f, 0.92f, 1.0f);

		const float innerSp = ImGui::GetStyle().ItemInnerSpacing.x;
		const float runW =
			ImGui::CalcTextSize("Run simulation for").x + innerSp + 60.0f * ui + innerSp +
			ImGui::CalcTextSize("ticks").x + innerSp + btnSz + 8.0f +
			ImGui::CalcTextSize("Run simulation for").x + innerSp + 60.0f * ui + innerSp +
			ImGui::CalcTextSize("ms").x + innerSp + btnSz +
			ImGui::GetStyle().WindowPadding.x + 8.0f;

		const float rightX = ImGui::GetWindowWidth() - runW;
		const float curX   = ImGui::GetCursorPosX();
		ImGui::SameLine(rightX > curX + 8.0f ? rightX : curX + 8.0f);

		// ── Run simulation for N ticks ────────────────────────────────────────
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Run simulation for");
		ImGui::SameLine(0, innerSp);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::SetNextItemWidth(60.0f * ui);
		if (ImGui::InputInt("##top_ticks", &tickCount, 0, 0) && tickCount < 1) tickCount = 1;
		ImGui::PopFont();
		ImGui::SameLine(0, innerSp);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("ticks");
		ImGui::SameLine(0, innerSp);

		ImGui::PushStyleColor(ImGuiCol_Button,        kFill);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  kActive);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f * ui);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0, 0));
		ImGui::PushFont(g_LargeIconsFont);
		if (ImGui::Button(ICON_FA_PLAY "##top_run_ticks", ImVec2(btnSz, btnSz)))
		{
			startTick = static_cast<int>(simulation->getT());
			if (!simulation->isInitialized()) { simulation->init(); startTick = 0; }
			simulation->resume();
			runNTicks = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(4);
		ImGui::SameLine(0, 20);

		// ── Run simulation for N ms ───────────────────────────────────────────
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Run simulation for");
		ImGui::SameLine(0, innerSp);
		ImGui::PushFont(g_MonoMediumFont);
		ImGui::SetNextItemWidth(60.0f * ui);
		if (ImGui::InputFloat("##top_ms", &msCount, 0.0f, 0.0f, "%.0f") && msCount < 0.1f) msCount = 0.1f;
		ImGui::PopFont();
		ImGui::SameLine(0, innerSp);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("ms");
		ImGui::SameLine(0, innerSp);

		ImGui::PushStyleColor(ImGuiCol_Button,        kFill);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  kActive);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f * ui);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0, 0));
		ImGui::PushFont(g_LargeIconsFont);
		if (ImGui::Button(ICON_FA_PLAY "##top_run_ms", ImVec2(btnSz, btnSz)))
		{
			if (!simulation->isInitialized()) simulation->init();
			simulation->resume();
			runForMs   = true;
			runMsStart = std::chrono::steady_clock::now();
		}
		ImGui::PopFont();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(4);
	}

	void StaticLayoutWindow::panelSimulation(const ImVec2 pos, const ImVec2 size) const
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

	void StaticLayoutWindow::panelNodeGraph(ImVec2 pos, ImVec2 size) const
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

	void StaticLayoutWindow::panelElement(ImVec2 pos, ImVec2 size) const
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

	void StaticLayoutWindow::panelStatusBar(ImVec2 pos, ImVec2 size) const
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
