// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "dynamic-neural-field-composer.h"

const dnf_composer::element::ElementSpatialDimensionParameters fieldDimensions{ 100, 1.0 };

std::shared_ptr<dnf_composer::Simulation> getExperimentSimulation()
{
	auto simulation = dnf_composer::createSimulation("test sim", 25, 0, 0);
	constexpr bool circularity = false;
	constexpr bool normalization = false;

	const dnf_composer::element::GaussStimulusParameters gcp_a = { 5, 10, 50, circularity, normalization};
	const std::shared_ptr<dnf_composer::element::GaussStimulus> gauss_stimulus
		(new dnf_composer::element::GaussStimulus({ "gauss stimulus", fieldDimensions }, gcp_a));
	simulation->addElement(gauss_stimulus);

	const dnf_composer::element::GaussStimulusParameters gcp_b = { 5, 10, 25, circularity, normalization };
	const std::shared_ptr<dnf_composer::element::GaussStimulus> gauss_stimulus_b
	(new dnf_composer::element::GaussStimulus({ "gauss stimulus b", fieldDimensions }, gcp_b));
	simulation->addElement(gauss_stimulus_b);

	/*const dnf_composer::element::LateralInteractionsParameters lip1 = { 5.3,7.4,6,6, -0.55, circularity, normalization};
	const std::shared_ptr<dnf_composer::element::LateralInteractions> k_1
	(new dnf_composer::element::LateralInteractions({ "k 1", fieldDimensions }, lip1));
	simulation->addElement(k_1);*/

	const dnf_composer::element::NormalNoiseParameters nnp = {0.3};
	const std::shared_ptr<dnf_composer::element::NormalNoise> noise(new dnf_composer::element::NormalNoise({ "noise", fieldDimensions }, nnp));
	simulation->addElement(noise);

	const dnf_composer::element::GaussKernelParameters gkp1 = { 2, 1, circularity, normalization };
	const std::shared_ptr<dnf_composer::element::GaussKernel> k_1
		(new dnf_composer::element::GaussKernel({ "k 1", fieldDimensions }, gkp1));
	simulation->addElement(k_1);

	//const dnf_composer::element::MexicanHatKernelParameters mhkp1 = { 6.8, 4.1, 8.9, 3.4};
	//const std::shared_ptr<dnf_composer::element::MexicanHatKernel> k_1
	//(new dnf_composer::element::MexicanHatKernel({ "k 1", fieldDimensions }, mhkp1));
	//simulation->addElement(k_1);

	const dnf_composer::element::HeavisideFunction activationFunction{ 0};
	const dnf_composer::element::NeuralFieldParameters nfp1 = { 25, -10 , activationFunction };
	const std::shared_ptr<dnf_composer::element::NeuralField> neural_field_1(new dnf_composer::element::NeuralField({ "neural field 1", fieldDimensions }, nfp1));
	simulation->addElement(neural_field_1);

	neural_field_1->addInput(k_1);
	neural_field_1->addInput(gauss_stimulus);
	neural_field_1->addInput(gauss_stimulus_b);
	neural_field_1->addInput(noise);
	k_1->addInput(neural_field_1);

	//const dnf_composer::element::LateralInteractionsParameters lip2 = { 3,7.2,12,6.4, -0.51 };
	//const std::shared_ptr<dnf_composer::element::LateralInteractions> k_2(new dnf_composer::element::LateralInteractions({ "k 2", fieldDimensions }, lip2));
	//simulation->addElement(k_2);

	//const dnf_composer::element::NeuralFieldParameters nfp2 = { 20, -10 , activationFunction };
	//const std::shared_ptr<dnf_composer::element::NeuralField> neural_field_2(new dnf_composer::element::NeuralField({ "neural field 2", fieldDimensions }, nfp2));
	//simulation->addElement(neural_field_2);

	//neural_field_2->addInput(k_2);
	//k_2->addInput(neural_field_2);

	//// excitatory connections between fields
	//dnf_composer::element::GaussKernelParameters gkp1;
	//gkp1.amplitude = 10;
	//gkp1.sigma = 8;
	//const std::shared_ptr<dnf_composer::element::GaussKernel> k_1_2(new dnf_composer::element::GaussKernel({ "k 1 2", fieldDimensions }, gkp1));
	//simulation->addElement(k_1_2);

	//neural_field_2->addInput(k_1_2);
	//k_1_2->addInput(neural_field_1);

	return simulation;
}


int main(int argc, char* argv[])
{
	try
	{
		// After defining the simulation, we can create the application.
		const auto simulation = getExperimentSimulation();

		const imgui_kit::WindowParameters winParams{ "Dynamic Neural Field Composer" };
		const imgui_kit::FontParameters fontParams{ std::string(PROJECT_DIR) + "/resources/fonts/Lexend-Light.ttf", 22 };
		const imgui_kit::StyleParameters styleParams{ ImVec4(0.2f, 0.2f, 0.2f, 0.8f) };
		const imgui_kit::IconParameters iconParams{ std::string(PROJECT_DIR) + "/resources/icons/icon.ico" };
		const imgui_kit::BackgroundImageParameters bgParams{ std::string(PROJECT_DIR) + "/resources/images/background.png", 0.4};
		imgui_kit::UserInterfaceParameters uiParameters{ winParams, fontParams, styleParams, iconParams, bgParams};
		dnf_composer::ApplicationParameters appParameters{ uiParameters };
		dnf_composer::Application app{ simulation, appParameters };

		// After creating the application, we can add the windows we want to display.
		app.addWindow<imgui_kit::LogWindow>();
		app.addWindow<dnf_composer::user_interface::SimulationWindow>();
		app.addWindow<dnf_composer::user_interface::ElementWindow>();

		// To add plots with data already loaded you need to use a Visualization object.
		dnf_composer::user_interface::PlotParameters plotParameters;
		plotParameters.annotations = { "Neural field monitoring", "Spatial dimension", "Amplitude" };
		plotParameters.dimensions = { 0, fieldDimensions.x_max, -15, 14, fieldDimensions.d_x };
		auto visualization = createVisualization(simulation);
		visualization->addPlottingData("neural field 1", "activation");
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