﻿// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "dynamic-neural-field-composer.h"

// This .cpp file is an example of how you can use the library to create your own DNF simulation.

int main(int argc, char* argv[])
{
	try
	{
		const auto simulation = dnf_composer::createSimulation( "run sim with gui example", 5, 0, 0 );
		const dnf_composer::Application app{ simulation };

		app.addWindow<imgui_kit::LogWindow>();
		app.addWindow<dnf_composer::user_interface::SimulationWindow>();
		app.addWindow<dnf_composer::user_interface::ElementWindow>();

		dnf_composer::user_interface::PlotParameters plotParameters;
		plotParameters.annotations = { "Plot title", "Spatial dimension", "Amplitude" };
		plotParameters.dimensions = { 0, 100, -30, 40 , 1.0 };
		auto visualization = dnf_composer::createVisualization(simulation);
		app.addWindow<dnf_composer::user_interface::PlotWindow>(visualization, plotParameters);
		app.addWindow<dnf_composer::user_interface::PlotWindow>(visualization, plotParameters);
		app.addWindow<dnf_composer::user_interface::PlotWindow>(visualization, plotParameters);


		app.init();


		bool userRequestClose = false;
		while (!userRequestClose)
		{
			app.step();
			userRequestClose = app.hasUIBeenClosed();
		}
		app.close();
		return 0;
	}
	catch (const dnf_composer::Exception &ex)
	{
		const std::string errorMessage = "Exception: " + std::string(ex.what()) + " ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())) + ". ";
		log(dnf_composer::tools::logger::LogLevel::FATAL, errorMessage, dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception &ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, "Exception caught: " + std::string(ex.what()) + ". ", dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
	catch (...)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, "Unknown exception occurred. ", dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
}