// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "dynamic-neural-field-composer.h"

int main(int argc, char* argv[])
{
	try
	{
		using namespace dnf_composer;
		auto simulation = std::make_shared<Simulation>("Panda robot joint task model", 5, 0, 0);
		constexpr bool activateUserInterface = true;
		const Application app{ simulation, activateUserInterface };

		element::ElementFactory factory;
		element::ElementSpatialDimensionParameters dim_params{ 90, 1.0 };

		// Action observation layer
		element::GaussStimulusParameters hand_position_gsp = { 6, 0, 0 };
		const auto hand_position_stimulus = factory.createElement(element::GAUSS_STIMULUS, { "hand position stimulus", dim_params }, { hand_position_gsp });
		simulation->addElement(hand_position_stimulus);

		const element::SigmoidFunction aol_af = { 0.0, 10.0 };
		element::NeuralFieldParameters aol_params = { 20, -5, aol_af };
		const auto aol = factory.createElement(element::NEURAL_FIELD, { "aol", dim_params }, { aol_params });
		simulation->addElement(aol);

		element::GaussKernelParameters aol_aol_k_params = { 3, 4.5 };
		const auto aol_aol_k = factory.createElement(element::GAUSS_KERNEL, { "aol -> aol", dim_params }, { aol_aol_k_params });
		simulation->addElement(aol_aol_k);

		const element::NormalNoiseParameters aol_nn_params = { 0.001 };
		const auto aol_nn = factory.createElement(element::NORMAL_NOISE, { "normal noise aol", dim_params }, aol_nn_params);
		simulation->addElement(aol_nn);

		simulation->createInteraction("aol", "output", "aol -> aol");
		simulation->createInteraction("aol -> aol", "output", "aol");
		simulation->createInteraction("normal noise aol", "output", "aol");
		simulation->createInteraction("hand position stimulus", "output", "aol");

		// Action simulation layer
		const element::SigmoidFunction asl_af = { 0.0, 10.0 };
		element::NeuralFieldParameters asl_params = { 20, -5, asl_af };
		const auto asl = factory.createElement(element::NEURAL_FIELD, { "asl", dim_params }, { asl_params });
		simulation->addElement(asl);

		element::LateralInteractionsParameters asl_asl_k_params = { 2, 19, 10, 30, -1.67 };
		const auto asl_asl_k = factory.createElement(element::LATERAL_INTERACTIONS, { "asl -> asl", dim_params }, { asl_asl_k_params });
		simulation->addElement(asl_asl_k);

		element::GaussKernelParameters aol_asl_k_params = { 3, -4.5 };
		const auto aol_asl_k = factory.createElement(element::GAUSS_KERNEL, { "aol -> asl", dim_params }, { aol_asl_k_params });
		simulation->addElement(aol_asl_k);

		const element::NormalNoiseParameters asl_nn_params = { 0.001 };
		const auto asl_nn = factory.createElement(element::NORMAL_NOISE, { "normal noise asl", dim_params }, asl_nn_params);
		simulation->addElement(asl_nn);

		simulation->createInteraction("asl", "output", "asl -> asl");
		simulation->createInteraction("asl -> asl", "output", "asl");
		simulation->createInteraction("normal noise asl", "output", "asl");

		simulation->createInteraction("aol", "output", "aol -> asl");
		simulation->createInteraction("aol -> asl", "output", "asl");

		// Object memory layer
		element::GaussStimulusParameters oml_gsp = { 3, 5, 30 };
		const auto oml_stimulus_1 = factory.createElement(element::GAUSS_STIMULUS, { "object stimulus 1", dim_params }, { oml_gsp });
		simulation->addElement(oml_stimulus_1);

		oml_gsp = { 3, 5, 60};
		const auto oml_stimulus_2 = factory.createElement(element::GAUSS_STIMULUS, { "object stimulus 2", dim_params }, { oml_gsp });
		simulation->addElement(oml_stimulus_2);

		oml_gsp = { 3, 5, 0 };
		const auto oml_stimulus_3 = factory.createElement(element::GAUSS_STIMULUS, { "object stimulus 3", dim_params }, { oml_gsp });
		simulation->addElement(oml_stimulus_3);

		element::SigmoidFunction oml_af = { 0.0, 10.0 };
		element::NeuralFieldParameters oml_params = { 20, -5, oml_af };
		const auto oml = factory.createElement(element::NEURAL_FIELD, { "oml", dim_params }, { oml_params });
		simulation->addElement(oml);

		element::GaussKernelParameters oml_oml_k_params = { 3, 5 };
		const auto oml_oml_k = factory.createElement(element::GAUSS_KERNEL, { "oml -> oml", dim_params }, { oml_oml_k_params });
		simulation->addElement(oml_oml_k);

		element::GaussKernelParameters oml_asl_k_params = { 3, 10 };
		const auto oml_asl_k = factory.createElement(element::GAUSS_KERNEL, { "oml -> asl", dim_params }, { oml_asl_k_params });
		simulation->addElement(oml_asl_k);

		element::NormalNoiseParameters oml_nn_params = { 0.001 };
		const auto oml_nn = factory.createElement(element::NORMAL_NOISE, { "normal noise oml", dim_params }, oml_nn_params);
		simulation->addElement(oml_nn);

		simulation->createInteraction("oml", "output", "oml -> asl");
		simulation->createInteraction("oml -> asl", "output", "asl");
		simulation->createInteraction("oml", "output", "oml -> oml");
		simulation->createInteraction("oml -> oml", "output", "oml");
		simulation->createInteraction("normal noise oml", "output", "oml");
		simulation->createInteraction("object stimulus 1", "output", "oml");
		simulation->createInteraction("object stimulus 2", "output", "oml");
		simulation->createInteraction("object stimulus 3", "output", "oml");

		// Action execution layer
		element::SigmoidFunction ael_af = { 0.0, 10.0 };
		element::NeuralFieldParameters ael_params = { 20, -5, ael_af };
		const auto ael = factory.createElement(element::NEURAL_FIELD, { "ael", dim_params }, { ael_params });
		simulation->addElement(ael);

		element::GaussKernelParameters asl_ael_k_params = { 3, 15 };
		const auto asl_ael_k = factory.createElement(element::GAUSS_KERNEL, { "asl -> ael", dim_params }, { asl_ael_k_params });
		simulation->addElement(asl_ael_k);

		element::LateralInteractionsParameters ael_ael_k_params = { 4, 6, 11, 12, -0.8 }; 
		const auto ael_ael_k = factory.createElement(element::LATERAL_INTERACTIONS, { "ael -> ael", dim_params }, { ael_ael_k_params });
		simulation->addElement(ael_ael_k);

		element::NormalNoiseParameters ael_nn_params = { 0.001 };
		const auto ael_nn = factory.createElement(element::NORMAL_NOISE, { "normal noise ael", dim_params }, ael_nn_params);
		simulation->addElement(ael_nn);

		simulation->createInteraction("ael", "output", "ael -> ael");
		simulation->createInteraction("ael -> ael", "output", "ael");
		simulation->createInteraction("normal noise ael", "output", "ael");
		simulation->createInteraction("asl", "output", "asl -> ael");
		simulation->createInteraction("asl -> ael", "output", "ael");

		// Create User Interface windows
		app.activateUserInterfaceWindow(user_interface::SIMULATION_WINDOW);
		app.activateUserInterfaceWindow(user_interface::LOG_WINDOW);
		app.activateUserInterfaceWindow(user_interface::ELEMENT_WINDOW);
		app.activateUserInterfaceWindow(user_interface::MONITORING_WINDOW);

		constexpr int yMax = 10;
		constexpr int yMin = 8;

		// Create a plot for each neural field
		user_interface::PlotParameters aolPlotParameters;
		aolPlotParameters.annotations = { "Action observation layer", "Spatial dimension", "Amplitude" };
		aolPlotParameters.dimensions = { 0, dim_params.x_max, -yMin, yMax, dim_params.d_x };
		const auto aolPlotWindow = std::make_shared<user_interface::PlotWindow>(simulation, aolPlotParameters);
		aolPlotWindow->addPlottingData("aol", "activation");
		aolPlotWindow->addPlottingData("aol", "input");
		aolPlotWindow->addPlottingData("aol", "output");
		app.activateUserInterfaceWindow(aolPlotWindow);

		user_interface::PlotParameters aslPlotParameters;
		aslPlotParameters.annotations = { "Action simulation layer", "Spatial dimension", "Amplitude" };
		aslPlotParameters.dimensions = { 0, dim_params.x_max, -yMin, yMax, dim_params.d_x };
		const auto aslPlotWindow = std::make_shared<user_interface::PlotWindow>(simulation, aslPlotParameters);
		aslPlotWindow->addPlottingData("asl", "activation");
		aslPlotWindow->addPlottingData("asl", "input");
		aslPlotWindow->addPlottingData("asl", "output");
		app.activateUserInterfaceWindow(aslPlotWindow);

		user_interface::PlotParameters omlPlotParameters;
		omlPlotParameters.annotations = { "Object memory layer", "Spatial dimension", "Amplitude" };
		omlPlotParameters.dimensions = { 0, dim_params.x_max, -yMin, yMax, dim_params.d_x };
		const auto omlPlotWindow = std::make_shared<user_interface::PlotWindow>(simulation, omlPlotParameters);
		omlPlotWindow->addPlottingData("oml", "activation");
		omlPlotWindow->addPlottingData("oml", "input");
		omlPlotWindow->addPlottingData("oml", "output");
		app.activateUserInterfaceWindow(omlPlotWindow);

		user_interface::PlotParameters aelPlotParameters;
		aelPlotParameters.annotations = { "Action execution layer", "Spatial dimension", "Amplitude" };
		aelPlotParameters.dimensions = { 0, dim_params.x_max, -yMin, yMax, dim_params.d_x };
		const auto aelPlotWindow = std::make_shared<user_interface::PlotWindow>(simulation, aelPlotParameters);
		aelPlotWindow->addPlottingData("ael", "activation");
		aelPlotWindow->addPlottingData("ael", "input");
		aelPlotWindow->addPlottingData("ael", "output");
		app.activateUserInterfaceWindow(aelPlotWindow);

		app.init();

		bool userRequestClose = false;
		while (!userRequestClose)
		{
			app.step();
			userRequestClose = app.getCloseUI();
		}
		app.close();
		return 0;
	}
	catch (const dnf_composer::Exception& ex)
	{
		const std::string errorMessage = "Exception: " + std::string(ex.what()) + " ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())) + ". \n";
		log(dnf_composer::tools::logger::LogLevel::FATAL, errorMessage, dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, "Exception caught: " + std::string(ex.what()) + ". \n", dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
	catch (...)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL, "Unknown exception occurred. \n", dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
}