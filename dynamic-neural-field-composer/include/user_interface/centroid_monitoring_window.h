#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "elements/neural_field.h"

namespace dnf_composer
{
	namespace user_interface
	{
		class CentroidMonitoringWindow : public imgui_kit::UserInterfaceWindow
		{
		private:
			std::shared_ptr<Simulation> simulation;
		public:
			CentroidMonitoringWindow(const std::shared_ptr<Simulation>& simulation);

			CentroidMonitoringWindow(const CentroidMonitoringWindow&) = delete;
			CentroidMonitoringWindow& operator=(const CentroidMonitoringWindow&) = delete;
			CentroidMonitoringWindow(CentroidMonitoringWindow&&) = delete;
			CentroidMonitoringWindow& operator=(CentroidMonitoringWindow&&) = delete;

			void render() override;
			~CentroidMonitoringWindow() override = default;
		private:
			void getNeuralFieldsAndRenderCentroids() const;
		};
	}
}