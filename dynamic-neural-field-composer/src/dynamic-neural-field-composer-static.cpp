#include "dynamic-neural-field-composer-static.h"

#include <format>



int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("default simulation");
		const auto visualization = std::make_shared<Visualization>(simulation);

		const Application app{ simulation, visualization };

		// Add the windows to the application
		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		app.init();

		while (!app.hasGUIBeenClosed())
		{
			app.step();
		}

		app.close();
	}
	catch (const dnf_composer::Exception& ex)
	{
		const std::string errorMessage = std::format("Exception: {} ErrorCode: {}. ", ex.what(), static_cast<int>(ex.getErrorCode()));
		log(dnf_composer::tools::logger::LogLevel::FATAL, errorMessage, dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, std::format("Exception caught: {}. ", ex.what()), dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
	catch (...)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, "Unknown exception occurred. ", dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
}