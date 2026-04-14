#include "user_interface/static_layout_window.h"

#include "application/application.h"
#include "user_interface/widgets.h"

extern ImFont* g_BlackLargeFont;

namespace dnf_composer::user_interface
{
	// ── Layout fractions (relative to total viewport) ────────────────────────
	static constexpr float kColAFrac    = 0.14f;   // Simulation + FieldMonitoring column
	static constexpr float kColBFrac    = 0.12f;   // Element Control column
	// Column C = remaining width

	static constexpr float kRowSimFrac  = 0.72f;   // Simulation Control height (of col A)
	// Neural Field Monitoring = 1 - kRowSimFrac

	static constexpr float kRowNodeFrac = 0.42f;   // Node graph height (of col C)
	static constexpr float kRowPltFrac  = 0.42f;   // Plots height (of col C)
	// Bottom strip = 1 - kRowNodeFrac - kRowPltFrac

	static constexpr float kLogsFrac    = 0.55f;   // Logs width (of bottom strip)
	// Plot Control = 1 - kLogsFrac

	static constexpr float kMargin      = 6.0f;    // gap between panels (px)
	static constexpr float kRounding    = 8.0f;    // child window rounding

	// ── Panel helpers ─────────────────────────────────────────────────────────

	// Non-scrollable panel (node graph, plots — manage their own scroll)
	static bool beginPanelFixed(const char* id, ImVec2 pos, ImVec2 size)
	{
		ImGui::SetCursorScreenPos(pos);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
		return ImGui::BeginChild(id, size, true,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	}

	// Scrollable panel (simulation control, neural field monitoring)
	static bool beginPanelScrollable(const char* id, ImVec2 pos, ImVec2 size)
	{
		ImGui::SetCursorScreenPos(pos);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
		return ImGui::BeginChild(id, size, true, ImGuiWindowFlags_None);
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
		simulationWindow   = std::make_unique<SimulationWindow>(simulation);
		elementWindow      = std::make_unique<ElementWindow>(simulation);
		nodeGraphWindow    = std::make_unique<NodeGraphWindow>(simulation);
		fieldMetricsWindow = std::make_unique<FieldMetricsWindow>(simulation);
		plotControlWindow  = std::make_unique<PlotControlWindow>(visualization);
		plotsWindow        = std::make_unique<PlotsWindow>(visualization);
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

		const float colAW = total.x * kColAFrac - m;
		const float colBW = total.x * kColBFrac - m;
		const float colCW = total.x - colAW - colBW - m * 4.0f;

		const float fullH = total.y - m * 2.0f;

		const float simH  = fullH * kRowSimFrac - m * 0.5f;
		const float nfmH  = fullH - simH - m;

		const float nodeH = fullH * kRowNodeFrac - m * 0.5f;
		const float pltH  = fullH * kRowPltFrac  - m * 0.5f;
		const float botH  = fullH - nodeH - pltH - m * 2.0f;

		const float logsW = colCW * kLogsFrac - m * 0.5f;
		const float pctlW = colCW - logsW - m;

		const float xA = origin.x + m;
		const float xB = xA + colAW + m;
		const float xC = xB + colBW + m;
		const float y0 = origin.y + m;

		panelSimulation  ({xA, y0},                             {colAW, simH });
		panelFieldMonitor({xA, y0 + simH + m},                  {colAW, nfmH });
		panelElement     ({xB, y0},                             {colBW, fullH});
		panelNodeGraph   ({xC, y0},                             {colCW, nodeH});
		panelPlots       ({xC, y0 + nodeH + m},                 {colCW, pltH });
		panelLogs        ({xC, y0 + nodeH + pltH + m * 2.0f},  {logsW, botH });
		panelPlotControl ({xC + logsW + m, y0 + nodeH + pltH + m * 2.0f}, {pctlW, botH});
	}

	// ── Panel implementations ─────────────────────────────────────────────────

	void StaticLayoutWindow::panelSimulation(ImVec2 pos, ImVec2 size) const
	{
		// Scrollable — content taller than the panel
		if (beginPanelScrollable("##sl_sim", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Simulation Control");
			ImGui::PopFont();
			ImGui::Separator();
			ImGui::Spacing();

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
		endPanel();
	}

	void StaticLayoutWindow::panelFieldMonitor(ImVec2 pos, ImVec2 size) const
	{
		// Scrollable — number of cards varies
		if (beginPanelScrollable("##sl_nfm", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Neural Field Monitoring");
			ImGui::PopFont();
			ImGui::Separator();
			ImGui::Spacing();
			fieldMetricsWindow->renderCards();
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

			// renderElementControlCard() starts its own BeginChild — make it transparent
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			elementWindow->renderElementControlCard();
			ImGui::PopStyleColor();
		}
		endPanel();
	}

	void StaticLayoutWindow::panelNodeGraph(ImVec2 pos, ImVec2 size) const
	{
		if (beginPanelFixed("##sl_node", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Node graph");
			ImGui::PopFont();
			ImGui::Separator();

			// Call renderGraph() directly — the outer ##sl_node child window already
			// provides a clip rect for this region, and imgui-node-editor creates its
			// own internal BeginChild for the canvas. Adding an extra intermediate
			// child caused the canvas origin to go stale when the panel was moved.
			nodeGraphWindow->renderGraph();
		}
		endPanel();
	}

	void StaticLayoutWindow::panelPlots(ImVec2 pos, ImVec2 size)
	{
		if (beginPanelFixed("##sl_plots", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Plots");
			ImGui::PopFont();
			ImGui::Separator();

			// Use PlotsWindow tiled renderer — recomputes layout internally when
			// plot count changes; respects child boundaries, no floating Begin() calls.
			plotsWindow->renderTiles();
		}
		endPanel();
	}

	void StaticLayoutWindow::panelLogs(ImVec2 pos, ImVec2 size) const
	{
		if (beginPanelFixed("##sl_logs", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Logs");
			ImGui::PopFont();
			ImGui::Separator();
			LogWindow::renderContent();
		}
		endPanel();
	}

	void StaticLayoutWindow::panelPlotControl(ImVec2 pos, ImVec2 size) const
	{
		if (beginPanelFixed("##sl_pctl", pos, size))
		{
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::Text("Plot Control");
			ImGui::PopFont();
			ImGui::Separator();
			plotControlWindow->renderContent();
		}
		endPanel();
	}
}
