#include "user_interface/static_layout.h"
#include "user_interface/fonts/IconsFontAwesome6.h"


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
		// ── Column widths (fixed px at a UI scale = 1.0, scale with zoom) ──────────
		static constexpr float kColABase   = 515.0F;  // Simulation Control
		static constexpr float kColBBase   = 400.0F;  // Element Control (rightmost)
		static constexpr float kStatusBarH = 30.0F;   // status bar height (bottom)
		static constexpr float kTopBarH    = 50.0F;   // top control bar height
		static constexpr float kMargin     = -3.0F;
		static constexpr float kRounding   = 1.0F;

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
			if (LogWindow::isActive())
			{
				logWindow->render();
			}
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
			const float colCW = total.x - colAW - colBW - (m * 4.0F);

			// y0: top bar row; y1: three column row
			const float y0    = origin.y + m;
			const float y1    = y0 + topBarH + m;

			// Full height of the three main columns; leave room for top bar + status bar + margins.
			const float fullH = total.y - (m * 2.0F) - statusH - m - topBarH - m;

			// Column x-positions: A (sim control) | C (node graph) | B (element control)
			const float xA = origin.x + m;
			const float xC = xA + colAW + m;
			const float xB = xC + colCW + m;

			drawPanelControlBar({origin.x + m, y0},      {total.x - (m * 2.0F), topBarH});
			drawPanelSimulation({xA, y1},                {colAW, fullH});
			drawPanelElement   ({xB, y1},                {colBW, fullH});
			drawPanelNodeGraph ({xC, y1},                {colCW, fullH});
			drawPanelStatusBar ({origin.x + m, y1 + fullH + m}, {total.x - (m * 2.0F), statusH});
		}

		void StaticLayoutWindow::drawPanelControlBar(const ImVec2 pos, const ImVec2 size) const
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0F, 4.0F));
			if (beginPanelFixed("##sl_control_bar", pos, size))
			{
				const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
							| ImGuiWindowFlags_NoScrollbar
							| ImGuiWindowFlags_NoScrollWithMouse
							| ImGuiWindowFlags_NoSavedSettings;

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
				if (ImGui::BeginChild("##cb", ImVec2(0, 0), 0,
					flags))
				{
					controlBarWindow->drawContents();
				}
				ImGui::EndChild();
				ImGui::PopStyleColor();
			}
			endPanel();
			ImGui::PopStyleVar();
		}

		void StaticLayoutWindow::drawPanelSimulation(const ImVec2 pos, const ImVec2 size) const
		{
			ImGui::SetCursorScreenPos(pos);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			if (ImGui::BeginChild("##sl_sim", size, false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				simulationWindow->renderSidebarContents();
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();
		}

		void StaticLayoutWindow::drawPanelElement(const ImVec2 pos, const ImVec2 size) const
		{
			ImGui::SetCursorScreenPos(pos);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.922f, 0.922f, 0.922f, 1.0f));  // tone b
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kRounding);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0F);
			if (ImGui::BeginChild("##sl_elem", size, static_cast<int>(true),
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				const float startY = ImGui::GetCursorPosY();
				const float yOff   = (g_MediumLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5f;
				ImGui::SetCursorPosY(startY + yOff);
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
				ImGui::PushFont(g_MediumIconsFont);
				ImGui::TextUnformatted(ICON_FA_SLIDERS);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::SameLine(0, 8.0F);
				ImGui::SetCursorPosY(startY);
				ImGui::PushFont(g_MediumLargeFont);
				ImGui::TextUnformatted("Element Control");
				ImGui::PopFont();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
				elementWindow->renderElementControlCard();
				ImGui::PopStyleColor();
			}
			ImGui::EndChild();
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();
		}

		void StaticLayoutWindow::drawPanelNodeGraph(const ImVec2 pos, const ImVec2 size) const
		{
			if (beginPanelFixed("##sl_node", pos, size))
			{
			const float startY = ImGui::GetCursorPosY();
				const float yOff   = (g_MediumLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5f;
				ImGui::SetCursorPosY(startY + yOff);
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
				ImGui::PushFont(g_MediumIconsFont);
				ImGui::TextUnformatted(ICON_FA_SHARE_NODES);
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::SameLine(0, 8.0F);
				ImGui::SetCursorPosY(startY);
				ImGui::PushFont(g_MediumLargeFont);
				ImGui::TextUnformatted("Node Graph");
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
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0F, 4.0F));
			if (beginPanelFixed("##sl_status_bar", pos, size))
			{
				const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
							| ImGuiWindowFlags_NoScrollbar
							| ImGuiWindowFlags_NoScrollWithMouse
							| ImGuiWindowFlags_NoSavedSettings;

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
				if (ImGui::BeginChild("##sb", ImVec2(0, 0), 0,
					flags))
				{
					statusBarWindow->drawContents();
				}
				ImGui::EndChild();
				ImGui::PopStyleColor();
			}
			endPanel();
			ImGui::PopStyleVar();
		}
	}
}
