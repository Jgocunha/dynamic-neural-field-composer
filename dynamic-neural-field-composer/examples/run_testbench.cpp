// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "dynamic-neural-field-composer.h"


const dnf_composer::element::ElementSpatialDimensionParameters fieldDimensions{ 50, 1.0 };

void printMaxValueOfElementComponent(const std::shared_ptr<dnf_composer::Simulation>& simulation, const std::string& elementName, const std::string& componentName)
{
	auto component = simulation->getComponent(elementName, componentName);
	std::cout << "Max value of " << elementName << " " << componentName << " component: " << *std::ranges::max_element(component.begin(), component.end()) << std::endl;
}

std::shared_ptr<dnf_composer::Simulation> getExperimentSimulation()
{
	auto simulation = dnf_composer::createSimulation("test sim", 25, 0, 0);
	constexpr bool circularity = false;
	constexpr bool normalization = true;

	//const dnf_composer::element::GaussStimulusParameters gcp_a = { 3, 8, 25, circularity, false };
	//const std::shared_ptr<dnf_composer::element::GaussStimulus> gauss_stimulus
	//(new dnf_composer::element::GaussStimulus({ "gauss stimulus", fieldDimensions }, gcp_a));
	//simulation->addElement(gauss_stimulus);

	////const dnf_composer::element::GaussStimulusParameters gcp_b = { 5, 10, 25, circularity, normalization };
	////const std::shared_ptr<dnf_composer::element::GaussStimulus> gauss_stimulus_b
	////(new dnf_composer::element::GaussStimulus({ "gauss stimulus b", fieldDimensions }, gcp_b));
	////simulation->addElement(gauss_stimulus_b);

	////const dnf_composer::element::LateralInteractionsParameters lip1 = { 5.3,7.4,6,6, -0.55, circularity, normalization};
	////const std::shared_ptr<dnf_composer::element::LateralInteractions> k_3
	////(new dnf_composer::element::LateralInteractions({ "k 3", fieldDimensions }, lip1));
	////simulation->addElement(k_3);

	///*const dnf_composer::element::NormalNoiseParameters nnp = { 0.01 };
	//const std::shared_ptr<dnf_composer::element::NormalNoise> noise(new dnf_composer::element::NormalNoise({ "noise", fieldDimensions }, nnp));
	//simulation->addElement(noise);*/

	//const dnf_composer::element::GaussKernelParameters gkp1 = { 3.0, 2.0, -0.0, circularity, normalization };
	//const std::shared_ptr<dnf_composer::element::GaussKernel> k_1
	//(new dnf_composer::element::GaussKernel({ "k 1", fieldDimensions }, gkp1));
	//simulation->addElement(k_1);
	//const dnf_composer::element::GaussKernelParameters gkp2 = { 3.0, 2.0, -0.0,circularity, normalization };
	//const std::shared_ptr<dnf_composer::element::GaussKernel> k_2
	//(new dnf_composer::element::GaussKernel({ "k 2", fieldDimensions }, gkp2));
	//simulation->addElement(k_2);

	////const dnf_composer::element::MexicanHatKernelParameters mhkp1 = { 6.8, 4.1, 8.9, 3.4};
	////const std::shared_ptr<dnf_composer::element::MexicanHatKernel> k_1
	////(new dnf_composer::element::MexicanHatKernel({ "k 1", fieldDimensions }, mhkp1));
	////simulation->addElement(k_1);

	//const dnf_composer::element::HeavisideFunction activationFunction{ 0 };

	//const dnf_composer::element::NeuralFieldParameters nfp1 = { 100, -5 , activationFunction };
	//const std::shared_ptr<dnf_composer::element::NeuralField> neural_field_1(new dnf_composer::element::NeuralField({ "neural field 1", fieldDimensions }, nfp1));
	//simulation->addElement(neural_field_1);
	//const std::shared_ptr<dnf_composer::element::NeuralField> neural_field_2(new dnf_composer::element::NeuralField({ "neural field 2", fieldDimensions }, nfp1));
	//simulation->addElement(neural_field_2);
	////const std::shared_ptr<dnf_composer::element::NeuralField> neural_field_3(new dnf_composer::element::NeuralField({ "neural field 3", fieldDimensions }, nfp1));
	////simulation->addElement(neural_field_3);

	//neural_field_1->addInput(k_1);
	//neural_field_1->addInput(gauss_stimulus);
	////neural_field_1->addInput(noise);
	//k_1->addInput(neural_field_1);

	//neural_field_2->addInput(k_2);
	//k_2->addInput(neural_field_2);

	////neural_field_3->addInput(k_3);
	////k_3->addInput(neural_field_3);

	//const dnf_composer::element::GaussKernelParameters gkp12 = { 3.0, 7.0, 0.0, circularity, normalization };
	//const std::shared_ptr<dnf_composer::element::GaussKernel> k_1_2
	//(new dnf_composer::element::GaussKernel({ "k 1 2", fieldDimensions }, gkp12));
	//simulation->addElement(k_1_2);
	////const std::shared_ptr<dnf_composer::element::GaussKernel> k_2_3
	////(new dnf_composer::element::GaussKernel({ "k 2 3", fieldDimensions }, gkp1));
	////simulation->addElement(k_2_3);

	//const dnf_composer::element::GaussFieldCouplingParameters gfcp = { normalization, circularity };
	//const std::shared_ptr < dnf_composer::element::GaussFieldCoupling> gfc (new dnf_composer::element::GaussFieldCoupling({ "gfc 1 - 2", fieldDimensions }, gfcp));
	//gfc->addCoupling({ 10.00, 40.0, 6.0, 3.0 });
	//gfc->addCoupling({ 25.0, 30.0, 10.0, 6.0 });
	//gfc->addCoupling({ 35.0, 10.0, 8.0, 3.0 });
	//simulation->addElement(gfc);

	//gfc->addInput(neural_field_1);
	//neural_field_2->addInput(gfc);

	////k_1_2->addInput(neural_field_1);
	////neural_field_2->addInput(k_1_2);
	////k_2_3->addInput(neural_field_2);
	////neural_field_3->addInput(k_2_3);
	///
	///std::shared_ptr<dnf_composer::Simulation> simulation = std::make_shared<dnf_composer::Simulation>("id", 30, 0, 0);

	// element common parameters
	dnf_composer::element::ElementSpatialDimensionParameters perceptualFieldSpatialDimensions{ 360, 0.5 };
	dnf_composer::element::ElementSpatialDimensionParameters outputFieldSpatialDimensions{28, 0.1};

	// create neural field
	//const dnf_composer::element::HeavisideFunction activationFunction{ 0 };
	const dnf_composer::element::SigmoidFunction activationFunction{0.0, 10.0};
	const dnf_composer::element::NeuralFieldParameters nfp1 = { 25, -5 , activationFunction };
	const dnf_composer::element::NeuralFieldParameters nfp2 = { 25, -5 , activationFunction };
	const std::shared_ptr<dnf_composer::element::NeuralField> perceptual_field
		(new dnf_composer::element::NeuralField({ "perceptual field", perceptualFieldSpatialDimensions }, nfp1));
	const std::shared_ptr<dnf_composer::element::NeuralField> output_field
		(new dnf_composer::element::NeuralField({ "output field", outputFieldSpatialDimensions }, nfp2));

	simulation->addElement(perceptual_field);
	simulation->addElement(output_field);

	// create interactions and add them to the simulation
	dnf_composer::element::GaussKernelParameters gkp1;
	gkp1.amplitude = 40;  // self-sustained (without input)
	gkp1.width = 25;
	gkp1.amplitudeGlobal = -0.08;
	gkp1.circular = true;
	gkp1.normalized = true;
	const std::shared_ptr<dnf_composer::element::GaussKernel> k_per_per
	(new dnf_composer::element::GaussKernel({ "per - per", perceptualFieldSpatialDimensions }, gkp1)); // self-excitation u-u
	simulation->addElement(k_per_per);

	dnf_composer::element::GaussKernelParameters gkp2;
	gkp2.amplitude = 20;  // self-stabilized (with input) //20
	gkp2.width = 25; // 25
	gkp2.amplitudeGlobal = -0.10;
	gkp2.circular = true;
	gkp2.normalized = true;
	const std::shared_ptr<dnf_composer::element::GaussKernel> k_out_out
	(new dnf_composer::element::GaussKernel({ "out - out", outputFieldSpatialDimensions }, gkp2)); // self-excitation v-v
	simulation->addElement(k_out_out);

	dnf_composer::element::FieldCouplingParameters fcp{ perceptualFieldSpatialDimensions.size, 0.2, 0.01, dnf_composer::LearningRule::DELTA_KROGH_HERTZ };
	const std::shared_ptr<dnf_composer::element::FieldCoupling> w_per_out(
		new dnf_composer::element::FieldCoupling({ "per - out", outputFieldSpatialDimensions }, fcp));
	simulation->addElement(w_per_out);

	// create noise stimulus and noise kernel
	const std::shared_ptr<dnf_composer::element::NormalNoise> noise_per
	(new dnf_composer::element::NormalNoise({ "noise per", perceptualFieldSpatialDimensions }, { 0.01 }));
	const std::shared_ptr<dnf_composer::element::NormalNoise> noise_out
	(new dnf_composer::element::NormalNoise({ "noise out", outputFieldSpatialDimensions }, { 0.01 }));
	const std::shared_ptr<dnf_composer::element::GaussKernel> noise_kernel_per
	(new dnf_composer::element::GaussKernel({ "noise kernel per", perceptualFieldSpatialDimensions }, { 0.25, 0.02, 0.0 }));
	const std::shared_ptr<dnf_composer::element::GaussKernel> noise_kernel_out
	(new dnf_composer::element::GaussKernel({ "noise kernel out", outputFieldSpatialDimensions }, { 0.25, 0.02, 0.0 }));

	simulation->addElement(noise_per);
	simulation->addElement(noise_out);
	simulation->addElement(noise_kernel_per);
	simulation->addElement(noise_kernel_out);

	// define the interactions between the elements
	perceptual_field->addInput(k_per_per); // self-excitation
	perceptual_field->addInput(noise_kernel_per); // noise

	output_field->addInput(k_out_out); // self-excitation
	output_field->addInput(noise_kernel_out); // noise
	output_field->addInput(w_per_out); // coupling

	k_per_per->addInput(perceptual_field);
	k_out_out->addInput(output_field);
	w_per_out->addInput(perceptual_field, "activation");

	noise_kernel_per->addInput(noise_per);
	noise_kernel_out->addInput(noise_out);

	return simulation;
}

int main(int argc, char* argv[])
{
	try
	{
		using namespace dnf_composer;
		//const auto simulation = std::make_shared<Simulation>("test sim", 5, 0, 0);
		//tools::logger::Logger::setMinLogLevel(tools::logger::LogLevel::DEBUG);


		//const element::NeuralField field{ { "field", fieldDimensions}, {25, -10, element::HeavisideFunction{0}} };
		//const element::GaussStimulus stimulus{ { "stimulus", fieldDimensions }, {5, 25, 50} };

		//simulation->addElement(std::make_shared<element::NeuralField>(field));
		//simulation->addElement(std::make_shared<element::GaussStimulus>(stimulus));
		//simulation->createInteraction("stimulus", "output", "field");

		//simulation->init();
		//double highestActivation = std::dynamic_pointer_cast<element::NeuralField>(simulation->getElement("field"))->getHighestActivation();
		//std::cout << "Highest activation: " << highestActivation << std::endl;
		//simulation->run(10);
		//highestActivation = std::dynamic_pointer_cast<element::NeuralField>(simulation->getElement("field"))->getHighestActivation();
		//std::cout << "Highest activation: " << highestActivation << std::endl;
		//simulation->close();
		//simulation->step();
		//simulation->init();
		//highestActivation = std::dynamic_pointer_cast<element::NeuralField>(simulation->getElement("field"))->getHighestActivation();
		//std::cout << "Highest activation: " << highestActivation << std::endl;
		//std::dynamic_pointer_cast<element::NeuralField>(simulation->getElement("field"))->setThresholdForStability(0.0001);
		//uint16_t steps = 0;
		//do
		//{
		//	steps++;
		//	simulation->step();
		//} while (!std::dynamic_pointer_cast<element::NeuralField>(simulation->getElement("field"))->isStable());
		//highestActivation = std::dynamic_pointer_cast<element::NeuralField>(simulation->getElement("field"))->getHighestActivation();
		//std::cout << "Highest activation: " << highestActivation << " in " << steps << " steps." << std::endl;

		//std::ofstream logFile(std::string(OUTPUT_DIRECTORY) + "/profiling/simulation-setup-with-logs.txt");

		//if (!logFile.is_open())
		//{
		//	std::cerr << "Failed to open log file!" << std::endl;
		//	return 1;
		//}

		//for (int i = 0; i < 1000; i++)
		//{
		//	tools::profiling::Timer timer{"Simulation setup with logs", logFile};
		//	const element::NeuralField field{ { "field", fieldDimensions}, {25, -10, element::HeavisideFunction{0}} };
		//	const element::GaussStimulus stimulus{ { "stimulus", fieldDimensions }, {5, 10, 50} };
		//	simulation->addElement(std::make_shared<element::NeuralField>(field));
		//	simulation->addElement(std::make_shared<element::GaussStimulus>(stimulus));
		//	simulation->createInteraction("stimulus", "output", "field");
		//	simulation->step();
		//	simulation->close();
		//	simulation->removeElement("field");
		//	simulation->removeElement("stimulus");
		//}

		// After defining the simulation, we can create the application.
		//const auto simulation = std::make_shared<dnf_composer::Simulation>("test", 25, 0, 0);
		//simulation->read("C:/dev-files/dynamic-neural-field-composer/dynamic-neural-field-composer/data/simulations/[neat-dnfs] self-sustained-single-bump.json");
		const auto simulation = getExperimentSimulation();
		const dnf_composer::Application app{ simulation };

		// After creating the application, we can add the windows we want to display.
		app.addWindow<dnf_composer::user_interface::MainWindow>();
		app.addWindow<imgui_kit::LogWindow>();
		app.addWindow<dnf_composer::user_interface::SimulationWindow>();
		app.addWindow<dnf_composer::user_interface::ElementWindow>();

		// To add plots with data already loaded you need to use a Visualization object.
		dnf_composer::user_interface::PlotParameters plotParameters;
		plotParameters.annotations = { "Neural field monitoring", "Spatial dimension", "Amplitude" };
		//plotParameters.dimensions = { 0, fieldDimensions.x_max, -8, 5, fieldDimensions.d_x };
		auto visualization = createVisualization(simulation);
		/*visualization->addPlottingData("gauss stimulus", "output");
		visualization->addPlottingData("neural field 1", "activation");
		visualization->addPlottingData("neural field 2", "activation");
		visualization->addPlottingData("k 1", "kernel");
		visualization->addPlottingData("k 2", "kernel");
		visualization->addPlottingData("k 1 2", "output");*/

		app.addWindow<user_interface::PlotWindow>(visualization, plotParameters);
		//app.addWindow<user_interface::NodeGraphWindow>();
		app.addWindow<user_interface::FieldMetricsWindow>();
		app.addWindow<user_interface::HeatmapWindow>();

		app.init();

		bool userRequestClose = false;
		while (!userRequestClose)
		{
			app.step();
			userRequestClose = app.hasUIBeenClosed();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			//printMaxValueOfElementComponent(simulation, "gauss stimulus", "output");
			//printMaxValueOfElementComponent(simulation, "neural field 1", "activation");
			//printMaxValueOfElementComponent(simulation, "neural field 2", "activation");
			//printMaxValueOfElementComponent(simulation, "k 1", "kernel");
			//printMaxValueOfElementComponent(simulation, "k 2", "kernel");
			//printMaxValueOfElementComponent(simulation, "k 1 2", "output");
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