#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "visualization/visualization.h"

namespace dnf_composer::user_interface
{
	/// @brief Window hosting the heatmap/line plots configured on a simulation's visualization.
	class PlotsWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Visualization> visualization;
	public:
		/// @brief Construct the plots window for a visualization.
		/// @param visualization Visualization whose plots are drawn.
		explicit PlotsWindow(const std::shared_ptr<Visualization>& visualization);
		/// @brief Draw the plots window for this frame.
		void render() override;
	};
}
