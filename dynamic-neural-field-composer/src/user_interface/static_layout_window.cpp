#include "user_interface/static_layout_window.h"

#include "application/application.h"
#include "user_interface/widgets.h"

extern ImFont* g_BlackLargeFont;
extern ImFont* g_BlackSmallFont;
extern ImFont* g_MonoMediumFont;
extern ImFont* g_MediumIconsFont;

namespace dnf_composer::user_interface
{
	// ── Column widths (fixed px at UI scale = 1.0, scale with zoom) ──────────
	static constexpr float kColABase   = 475.0f;  // Simulation Control
	static constexpr float kColBBase   = 360.0f;  // Element Control (rightmost)
	static constexpr float kStatusBarH = 40.0f;   // status bar height
	static constexpr float kMargin     = 6.0f;
	static constexpr float kRounding   = 8.0f;

	// ── Panel helpers ─────────────────────────────────────────────────────────

	static bool beginPanelFixed(const char* id, ImVec2 pos, ImVec2 size)
	{
		ImGui::SetCursorScreenPos(pos);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
		return ImGui::BeginChild(id, size, true,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	}

	static void endPanel()
	{
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	// ── Constructor ───────────────────────────────────────────────────────────
	StaticLayoutWindow::StaticLayoutWindow(
		const std::shared_ptr<Simulation>&    simulation,
		const std::shared_ptr<Visualization>& visualization)
		: simulation(simulation), visualization(visualization)
	{
		simulationWindow = std::make_unique<SimulationWindow>(simulation);
		elementWindow    = std::make_unique<ElementWindow>(simulation);
		nodeGraphWindow  = std::make_unique<NodeGraphWindow>(simulation);
	}

	// ── render() ─────────────────────────────────────────────────────────────
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
			drawPanels();

		ImGui::End();
	}

	// ── drawPanels() ──────────────────────────────────────────────────────────
	void StaticLayoutWindow::drawPanels()
	{
		const ImVec2 origin = ImGui::GetWindowPos();
		const ImVec2 total  = ImGui::GetWindowSize();
		const float  m      = kMargin;
		const float  scale  = ImGui::GetIO().FontGlobalScale;

		const float colAW   = kColABase * scale;
		const float colBW   = kColBBase * scale;
		const float statusH = kStatusBarH * scale;

		// Node graph (col C) fills the remaining horizontal space.
		const float colCW = total.x - colAW - colBW - m * 4.0f;

		// Full height of the three main columns; leave room for status bar + margin.
		const float fullH = total.y - m * 2.0f - statusH - m;

		// Column x-positions: A (sim control) | C (node graph) | B (element control)
		const float xA = origin.x + m;
		const float xC = xA + colAW + m;
		const float xB = xC + colCW + m;
		const float y0 = origin.y + m;

		panelSimulation({xA, y0}, {colAW, fullH});
		panelNodeGraph ({xC, y0}, {colCW, fullH});
		panelElement   ({xB, y0}, {colBW, fullH});
		panelStatusBar ({origin.x + m, y0 + fullH + m}, {total.x - m * 2.0f, statusH});
	}

	// ── Panel implementations ─────────────────────────────────────────────────

	void StaticLayoutWindow::panelSimulation(ImVec2 pos, ImVec2 size) const
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
				simulationWindow->renderSimulationParametersCard();
				ImGui::Spacing();
				simulationWindow->renderSimulationControlsCard();
				ImGui::Spacing();
				simulationWindow->renderRunForIterationsCard();
				ImGui::Spacing();
				simulationWindow->renderAddElementCard();
				ImGui::Spacing();
				simulationWindow->renderRemoveElementCard();
				ImGui::Spacing();
				simulationWindow->renderSetInteractionCard();
				ImGui::Spacing();
				simulationWindow->renderLogElementParametersCard();
				ImGui::Spacing();
				simulationWindow->renderExportElementComponentCard();
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

			nodeGraphWindow->renderGraph();
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
			const std::string simId = simulation->getIdentifier();
			const bool running = simulation->isInitialized() && !simulation->isPaused();
			const bool paused  = simulation->isInitialized() &&  simulation->isPaused();

			const ImVec4 dotColor = running ? ImVec4(0.20f, 0.75f, 0.20f, 1.0f)
			                      : paused  ? ImVec4(0.90f, 0.70f, 0.10f, 1.0f)
			                                : ImVec4(0.75f, 0.20f, 0.20f, 1.0f);
			const char* stateStr  = running ? "Running" : paused ? "Paused" : "Stopped";

			// ── Logo (left-most) ───────────────────────────────────────────────
			// ImGui::AlignTextToFramePadding();
			// ImGui::PushFont(g_MediumIconsFont);
			// ImGui::TextUnformatted(ICON_FA_ATOM);
			// ImGui::PopFont();
			// ImGui::SameLine(0, 8);

			// ── Simulation name in bold-black small font ───────────────────────
			ImGui::PushFont(g_BlackSmallFont);
			const std::string identifier = simId + ".dnf";
			ImGui::TextUnformatted(identifier.c_str());
			ImGui::PopFont();
			ImGui::SameLine(0, 20);

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
			ImGui::TextUnformatted("Sim. time");
			ImGui::SameLine(0, 4);
			ImGui::PushFont(g_MonoMediumFont);
			ImGui::Text("%.0f", simulation->getT());
			ImGui::PopFont();
			ImGui::SameLine(0, 4);
			ImGui::TextUnformatted("iter.");
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

			// ── FPS + Zoom — right-flushed ────────────────────────────────────
			char fpsBuf[32], zoomBuf[16];
			std::snprintf(fpsBuf,  sizeof(fpsBuf),  "%.1f", ImGui::GetIO().Framerate);
			std::snprintf(zoomBuf, sizeof(zoomBuf), "%d%%",  static_cast<int>(Application::getUiScalePct()));

			const float sep  = 14.0f;
			const float rW   =
				ImGui::CalcTextSize("FPS ").x + ImGui::CalcTextSize(fpsBuf).x  + sep +
				ImGui::CalcTextSize("Zoom ").x + ImGui::CalcTextSize(zoomBuf).x +
				ImGui::GetStyle().WindowPadding.x;

			const float rightX = ImGui::GetWindowWidth() - rW;
			const float curX   = ImGui::GetCursorPosX();
			if (rightX > curX + sep)
				ImGui::SameLine(rightX);
			else
				ImGui::SameLine(0, sep);

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
		}
		endPanel();
	}
}
