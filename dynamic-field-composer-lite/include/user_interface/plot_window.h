#pragma once


#include "user_interface_window.h"
#include "simulation/visualization.h"


namespace dnf_composer
{
	namespace user_interface
	{
		static int current_id = 0;

		struct PlotDimensions
		{
			int xMin = 0, xMax = 0, yMin = 0, yMax = 0;
			double dx = 0.0;
		};

		struct PlotAnnotations
		{
			std::string title{}, x_label{}, y_label{};
		};

		struct PlotParameters
		{
			int id;
			std::shared_ptr<Visualization> visualization;
			PlotDimensions dimensions;
			PlotAnnotations annotations;
		};

		class PlotWindow : public UserInterfaceWindow
		{
		private:
			std::vector<PlotParameters> plots;
			bool elementSelectorWindowSelected;
		public:
			PlotWindow(const std::shared_ptr<Simulation>& simulation, bool renderElementSelector = true);
			PlotWindow(const std::shared_ptr<Simulation>& simulation, PlotParameters parameters, bool renderElementSelector = true);
			PlotWindow(const std::shared_ptr<Visualization>& visualization, bool renderElementSelector = true);
			PlotWindow(const std::shared_ptr<Visualization>& visualization, PlotParameters parameters, bool renderElementSelector = true);

			void render() override;
			~PlotWindow() override = default;
		private:
			void createPlot(PlotParameters& parameters);
			void renderPlotControl();
			static void renderPlot(const PlotParameters& parameters);
			static void renderElementSelector(const PlotParameters& parameters);
			static void configure(const PlotDimensions& dimensions);
		};
	}
}


