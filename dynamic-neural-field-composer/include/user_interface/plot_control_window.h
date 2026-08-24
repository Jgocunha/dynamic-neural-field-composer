#pragma once


#include <imgui-platform-kit/user_interface_window.h>
#include "visualization/visualization.h"
#include "application/application.h"


//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace dnf_composer::user_interface
{
	/// @brief Pick the plot type quick-populate should use for a given element.
	///
	/// A 2D neural field cannot be represented by a line plot, so it is shown
	/// as a heatmap instead; every other (1D) field keeps the classic line plot.
	/// Pure decision logic, kept free of ImGui so it can be unit-tested headlessly.
	[[nodiscard]] PlotType quickPopulatePlotTypeFor(const std::shared_ptr<element::Element>& element);

	/// @brief Window for adding/removing/configuring the plots shown by @c PlotsWindow.
	class PlotControlWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Visualization> visualization;
		std::shared_ptr<Simulation> simulation;
	public:
		/// @brief Construct the plot control window for a visualization.
		/// @param visualization Visualization whose plots are added to, removed, and configured.
		explicit PlotControlWindow(const std::shared_ptr<Visualization>& visualization);
		/// @brief Draw the plot control window for this frame.
		void render() override;
		/// @brief Draw the plot control window's contents.
		void renderContent() const;
	};
}
