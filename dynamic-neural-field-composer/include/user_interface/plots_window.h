#pragma once

#include <vector>
#include <imgui-platform-kit/user_interface_window.h>
#include <cmath>

#include "visualization/visualization.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::user_interface
{
	//extern ImFont* g_BlackLargeFont;

	class PlotsWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Visualization> visualization;
		std::shared_ptr<Simulation>    simulation;

		// Tiled layout state
		int                lastPlotCount = -1;
		std::vector<float> colWidths;
		std::vector<float> rowHeights;

		void recomputeLayout(int n, float availW, float availH);
		void renderTiles();

	public:
		explicit PlotsWindow(const std::shared_ptr<Visualization>& visualization);
		void render() override;
	};
}
