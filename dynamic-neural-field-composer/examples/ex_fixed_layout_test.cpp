#include "application/application.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/main_menu_bar.h"
#include "user_interface/main_menu_window.h"
#include "user_interface/plots_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/simulation_window.h"
#include "user_interface/log_window.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("fixed layout test");
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		//app.addWindow<user_interface::MainWindow>();
		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::SimulationWindow>();
		app.addWindow<user_interface::ElementWindow>();
		app.addWindow<user_interface::FieldMetricsWindow>();
		app.addWindow<user_interface::PlotControlWindow>();
		app.addWindow<user_interface::PlotsWindow>();
		app.addWindow<user_interface::NodeGraphWindow>();
		app.addWindow<user_interface::LogWindow>();

		app.init();

		while (!app.hasGUIBeenClosed())
		{
			app.step();
		}

		app.close();

		return 0;
	}
	catch (const dnf_composer::Exception& ex)
	{
		const std::string errorMessage = "Exception: " + std::string(ex.what()) + " ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())) + ". ";
		log(dnf_composer::tools::logger::LogLevel::FATAL, errorMessage, dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception& ex)
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