#pragma once


#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/gauss_field_coupling.h"
#include "elements/field_coupling.h"
#include "tools/math.h"


namespace dnf_composer
{
	namespace user_interface
	{
		struct HeatmapParameters
		{
			float scaleMin, scaleMax;

			HeatmapParameters()
				: scaleMin(0.0f), scaleMax(0.03f)
			{}
		};

		class HeatmapWindow : public imgui_kit::UserInterfaceWindow
		{
		private:
			std::shared_ptr<Simulation> simulation;
			HeatmapParameters parameters;
		public:
			HeatmapWindow(const std::shared_ptr<Simulation>& simulation);
			void render() override;
			~HeatmapWindow() override = default;
		private:
			void renderColormap();
			void renderPlot();
		};
	}
}


