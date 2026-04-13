#include "user_interface/plots_window.h"


namespace dnf_composer { extern ImFont* g_BlackFont; }
using dnf_composer::g_BlackFont;

namespace dnf_composer::user_interface
{
	static constexpr float kSplitterThickness = 4.0f;
	static constexpr float kTileMinSize       = 80.0f;

	PlotsWindow::PlotsWindow(const std::shared_ptr<Visualization>& visualization)
		: visualization(visualization), simulation(visualization->getSimulation())
	{}

	void PlotsWindow::recomputeLayout(int n, float availW, float availH)
	{
		if (n <= 0) { colWidths.clear(); rowHeights.clear(); return; }

		const int cols = static_cast<int>(std::ceil(std::sqrt(float(n))));
		const int rows = (n + cols - 1) / cols;

		const float cellW = (availW - kSplitterThickness * float(cols - 1)) / float(cols);
		const float cellH = (availH - kSplitterThickness * float(rows - 1)) / float(rows);

		colWidths.assign(cols, std::max(cellW, kTileMinSize));
		rowHeights.assign(rows, std::max(cellH, kTileMinSize));
	}

	void PlotsWindow::render()
	{
		ImGui::PushFont(g_BlackFont);
		const bool open = ImGui::Begin("Plots", nullptr, imgui_kit::getGlobalWindowFlags());
		ImGui::PopFont();

		if (!open) { ImGui::End(); return; }

		const int n = static_cast<int>(visualization->getPlots().size());

		if (n != lastPlotCount)
		{
			recomputeLayout(n, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
			lastPlotCount = n;
		}

		if (n > 0)
			renderTiles();

		ImGui::End();
	}

	void PlotsWindow::renderTiles()
	{
		const auto plots = visualization->getPlots();
		const int  n     = static_cast<int>(plots.size());
		if (n == 0) return;

		const int cols = static_cast<int>(colWidths.size());
		const int rows = static_cast<int>(rowHeights.size());
		if (cols == 0 || rows == 0) return;

		// Collect plot IDs in stable order
		std::vector<int> ids;
		ids.reserve(n);
		for (const auto& [plot, _] : plots)
			ids.push_back(plot->getUniqueIdentifier());

		const ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
		const float  ui           = ImGui::GetIO().FontGlobalScale;

		// --- Compute tile origins ---
		std::vector<float> colX(cols), rowY(rows);
		colX[0] = 0.0f;
		for (int c = 1; c < cols; ++c)
			colX[c] = colX[c-1] + colWidths[c-1] + kSplitterThickness;
		rowY[0] = 0.0f;
		for (int r = 1; r < rows; ++r)
			rowY[r] = rowY[r-1] + rowHeights[r-1] + kSplitterThickness;

		// --- Render each tile ---
		int idx = 0;
		for (int r = 0; r < rows && idx < n; ++r)
		{
			for (int c = 0; c < cols && idx < n; ++c, ++idx)
			{
				const int   plotId    = ids[idx];
				const float tileW     = colWidths[c];
				const float tileH     = rowHeights[r];
				const ImVec2 tilePos  = { canvasOrigin.x + colX[c], canvasOrigin.y + rowY[r] };

				ImGui::SetCursorScreenPos(tilePos);

				const std::string childId = "##tile_" + std::to_string(plotId);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
					ImVec2(ImGui::GetStyle().FramePadding.x, 2.0f * ui));

				if (ImGui::BeginChild(childId.c_str(), ImVec2(tileW, tileH), true,
				    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar))
				{
					// Title in menu bar
					if (ImGui::BeginMenuBar())
					{
						ImGui::PushFont(g_BlackFont);
						ImGui::TextUnformatted(("Plot #" + std::to_string(plotId)).c_str());
						ImGui::PopFont();
						ImGui::EndMenuBar();
					}
					visualization->renderTile(plotId);
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
			}
		}

		// --- Vertical splitters (between columns) ---
		for (int c = 0; c < cols - 1; ++c)
		{
			const float totalH = rowY[rows-1] + rowHeights[rows-1];
			const ImVec2 splitterPos = {
				canvasOrigin.x + colX[c] + colWidths[c],
				canvasOrigin.y
			};
			ImGui::SetCursorScreenPos(splitterPos);

			const std::string splitterId = "##vsplit_" + std::to_string(c);
			ImGui::InvisibleButton(splitterId.c_str(), ImVec2(kSplitterThickness, totalH));

			if (ImGui::IsItemHovered())
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

			if (ImGui::IsItemActive())
			{
				const float delta = ImGui::GetIO().MouseDelta.x;
				colWidths[c]   = std::max(colWidths[c]   + delta, kTileMinSize);
				colWidths[c+1] = std::max(colWidths[c+1] - delta, kTileMinSize);
				// recompute origins
				for (int i = 1; i < cols; ++i)
					colX[i] = colX[i-1] + colWidths[i-1] + kSplitterThickness;
			}

			// Draw a subtle line
			ImGui::GetWindowDrawList()->AddLine(
				splitterPos,
				{ splitterPos.x, splitterPos.y + totalH },
				ImGui::IsItemHovered() || ImGui::IsItemActive()
					? IM_COL32(120, 120, 120, 180)
					: IM_COL32(180, 180, 180, 100),
				1.5f);
		}

		// --- Horizontal splitters (between rows) ---
		for (int r = 0; r < rows - 1; ++r)
		{
			const float totalW = colX[cols-1] + colWidths[cols-1];
			const ImVec2 splitterPos = {
				canvasOrigin.x,
				canvasOrigin.y + rowY[r] + rowHeights[r]
			};
			ImGui::SetCursorScreenPos(splitterPos);

			const std::string splitterId = "##hsplit_" + std::to_string(r);
			ImGui::InvisibleButton(splitterId.c_str(), ImVec2(totalW, kSplitterThickness));

			if (ImGui::IsItemHovered())
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

			if (ImGui::IsItemActive())
			{
				const float delta = ImGui::GetIO().MouseDelta.y;
				rowHeights[r]   = std::max(rowHeights[r]   + delta, kTileMinSize);
				rowHeights[r+1] = std::max(rowHeights[r+1] - delta, kTileMinSize);
				for (int i = 1; i < rows; ++i)
					rowY[i] = rowY[i-1] + rowHeights[i-1] + kSplitterThickness;
			}

			ImGui::GetWindowDrawList()->AddLine(
				splitterPos,
				{ splitterPos.x + totalW, splitterPos.y },
				ImGui::IsItemHovered() || ImGui::IsItemActive()
					? IM_COL32(120, 120, 120, 180)
					: IM_COL32(180, 180, 180, 100),
				1.5f);
		}

		// Advance cursor past the entire tiled area so ImGui flow is correct
		const float totalW = colX[cols-1] + colWidths[cols-1];
		const float totalH = rowY[rows-1] + rowHeights[rows-1];
		ImGui::SetCursorScreenPos({ canvasOrigin.x, canvasOrigin.y + totalH });
		ImGui::Dummy(ImVec2(totalW, 0));
	}
}
