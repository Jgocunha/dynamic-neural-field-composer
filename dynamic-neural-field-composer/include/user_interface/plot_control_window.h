#pragma once


#include <imgui-platform-kit/user_interface_window.h>
#include "visualization/visualization.h"
#include "application/application.h"

extern ImFont* g_BlackFont;

namespace dnf_composer::user_interface
{
	class PlotControlWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Visualization> visualization;
		std::shared_ptr<Simulation> simulation;
	public:
		explicit PlotControlWindow(const std::shared_ptr<Visualization>& visualization);
		void render() override;
	};
}
