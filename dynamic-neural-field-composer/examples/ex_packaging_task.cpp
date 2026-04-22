#include "application/application.h"
#include "user_interface/main_window.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/element_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/simulation_window.h"
#include "user_interface/node_graph_window.h"
#include "elements/element_factory.h"
#include "user_interface/plots_window.h"

#include <memory>
#include <vector>
#include <string>

#include "simulation/simulation_file_manager.h"

int main()
{
	try
	{
		 using namespace dnf_composer;

	    const std::shared_ptr<Simulation> sim =
	        std::make_shared<Simulation>("packaging task control architecture", 5.0);

	    const SimulationFileManager sfm(sim,
	      std::string(OUTPUT_DIRECTORY) + "/simulations/solution 15233 generation 32 species 2 fitness 0.950102.json");
	    sfm.loadElementsFromJson();

	    auto visualization = std::make_shared<Visualization>(sim);
	    const Application app{ sim, visualization };

	    visualization->plot(
	        PlotCommonParameters{
	        PlotType::LINE_PLOT,
	        PlotDimensions{ 0, 60, -20, 20, 1.0, 1.0},
	        PlotAnnotations{ "small-object location input field", "Spatial location", "Amplitude" } },
	        LinePlotParameters{},
	        {
	            { "nf 1", "activation" },
	            { "nf 1", "input" },
	        }
	    );

		visualization->plot(
	        PlotCommonParameters{
	        PlotType::LINE_PLOT,
	        PlotDimensions{ 0, 60, -20, 20, 1.0, 1.0},
	        PlotAnnotations{ "large-object location input field", "Spatial location", "Amplitude" } },
	        LinePlotParameters{},
	        {
	            { "nf 2", "activation" },
	            { "nf 2", "input" },
	        }
	    );

	    visualization->plot(
	        PlotCommonParameters{
	        PlotType::LINE_PLOT,
	        PlotDimensions{ 0.0, 60, -20.0, 20, 1.0, 1.0},
	        PlotAnnotations{ "hand-position input field", "Spatial location", "Amplitude" } },
	        LinePlotParameters{},
	        {
	            { "nf 3", "activation" },
	            { "nf 3", "input" },
	    });

	    visualization->plot(
	        PlotCommonParameters{
	        PlotType::LINE_PLOT,
	        PlotDimensions{ 0.0, 60, -20.0, 20, 1.0, 1.0},
	        PlotAnnotations{ "hidden field", "Spatial location", "Amplitude" } },
	        LinePlotParameters{},
	        {
	            { "nf 5", "activation" },
	            { "gk cg 1 - 5 11", "output" },
	            { "gk cg 3 - 5 96", "output" },
	    });

	    visualization->plot(
	        PlotCommonParameters{
	        PlotType::LINE_PLOT,
	        PlotDimensions{ 0.0, 60, -20.0, 20, 1.0, 1.0},
	        PlotAnnotations{ "target-robot-action field", "Spatial location", "Amplitude" } },
	        LinePlotParameters{},
	        {
	            { "nf 4", "activation" },
	            { "gk cg 5 - 4 12", "output" },
	            { "gk cg 2 - 4 66", "output" },
	            { "gk cg 3 - 4 109", "output" },
	    });

	    app.addWindow<user_interface::MainWindow>();
	    app.addWindow<imgui_kit::LogWindow>();
	    //app.addWindow<user_interface::FieldMetricsWindow>();
	    app.addWindow<user_interface::ElementWindow>();
	    app.addWindow<user_interface::SimulationWindow>();
	    //app.addWindow<user_interface::PlotControlWindow>();
	    app.addWindow<user_interface::PlotsWindow>();
	    //app.addWindow<user_interface::NodeGraphWindow>();

		app.init();

		while (!app.hasGUIBeenClosed())
		{
			app.step();
		}

		app.close();
	}
	catch (const dnf_composer::Exception& ex)
	{
		const std::string errorMessage =
			"Exception: " + std::string(ex.what()) +
			" ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())) + ". ";
		log(dnf_composer::tools::logger::LogLevel::FATAL,
			errorMessage,
			dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
			"Exception caught: " + std::string(ex.what()) + ". ",
			dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
	catch (...)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
			"Unknown exception occurred. ",
			dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
}
