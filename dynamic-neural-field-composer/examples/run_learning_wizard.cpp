// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "dynamic-neural-field-composer.h"

// This .cpp file is an example of how you can use the library to create your own DNF simulation.
// This setup runs the application with a GUI.
// This .exe creates a simulation object programatically defining its elements.
// This .exe runs a learning wizard to compute the coupling strengths between two fields.

std::shared_ptr<dnf_composer::Simulation> getExperimentSimulation()
{
	// define if you want to train the weights or not
	constexpr bool train = false;

	// create simulation object
	std::shared_ptr<dnf_composer::Simulation> simulation = std::make_shared<dnf_composer::Simulation>("learning_wizard", 1, 0, 0);

	const dnf_composer::element::ElementSpatialDimensionParameters perceptualFieldSpatialDimensionParameters{360, 0.5};
	const dnf_composer::element::ElementSpatialDimensionParameters outputFieldSpatialDimensionParameters{180, 0.5};

	// create neural fields
	const dnf_composer::element::HeavisideFunction activationFunction{ 0 };
	const dnf_composer::element::NeuralFieldParameters nfp1 = { 25, -10 , activationFunction };
	const std::shared_ptr<dnf_composer::element::NeuralField> perceptual_field (new dnf_composer::element::NeuralField({ "perceptual field", perceptualFieldSpatialDimensionParameters }, nfp1));


	const dnf_composer::element::NeuralFieldParameters nfp2 = { 20, -10 , activationFunction };
	const std::shared_ptr<dnf_composer::element::NeuralField> output_field(new dnf_composer::element::NeuralField({ "output field", outputFieldSpatialDimensionParameters }, nfp2));

	simulation->addElement(perceptual_field);
	simulation->addElement(output_field);

	// create interactions and add them to the simulation
	dnf_composer::element::GaussKernelParameters gkp1;
	gkp1.amplitude = 10;  
	gkp1.width = 5;
	const std::shared_ptr<dnf_composer::element::GaussKernel> k_per_per(new dnf_composer::element::GaussKernel({ "per - per", perceptualFieldSpatialDimensionParameters}, gkp1));
	simulation->addElement(k_per_per);

	dnf_composer::element::GaussKernelParameters gkp2;
	gkp2.amplitude = 5;  
	gkp2.width = 3;
	const std::shared_ptr<dnf_composer::element::GaussKernel> k_out_out(new dnf_composer::element::GaussKernel({ "out - out", outputFieldSpatialDimensionParameters }, gkp2)); // self-excitation v-v
	simulation->addElement(k_out_out);

	dnf_composer::element::FieldCouplingParameters fcp{ perceptualFieldSpatialDimensionParameters.size , 0.75, 0.01, dnf_composer::LearningRule::DELTA_WIDROW_HOFF };
	const std::shared_ptr<dnf_composer::element::FieldCoupling> w_per_out
	(new dnf_composer::element::FieldCoupling({ "per - out", outputFieldSpatialDimensionParameters }, fcp));
	simulation->addElement(w_per_out);

	// create noise stimulus and noise kernel
	const std::shared_ptr<dnf_composer::element::NormalNoise> noise_per(new dnf_composer::element::NormalNoise({ "noise per", perceptualFieldSpatialDimensionParameters }, { 0.2 }));
	const std::shared_ptr<dnf_composer::element::NormalNoise> noise_dec(new dnf_composer::element::NormalNoise({ "noise out", outputFieldSpatialDimensionParameters }, { 0.2 }));
	const std::shared_ptr<dnf_composer::element::GaussKernel> noise_kernel_per(new dnf_composer::element::GaussKernel({ "noise kernel per", perceptualFieldSpatialDimensionParameters }, { 0.25, 0.2 }));
	const std::shared_ptr<dnf_composer::element::GaussKernel> noise_kernel_dec(new dnf_composer::element::GaussKernel({ "noise kernel out", outputFieldSpatialDimensionParameters }, { 0.25, 0.2 }));

	simulation->addElement(noise_per);
	simulation->addElement(noise_dec);
	simulation->addElement(noise_kernel_per);
	simulation->addElement(noise_kernel_dec);

	// define the interactions between the elements
	perceptual_field->addInput(k_per_per); // self-excitation
	perceptual_field->addInput(noise_kernel_per); // noise

	output_field->addInput(k_out_out); // self-excitation
	output_field->addInput(noise_kernel_dec); // noise
	output_field->addInput(w_per_out); // coupling

	k_per_per->addInput(perceptual_field);
	k_out_out->addInput(output_field);
	w_per_out->addInput(perceptual_field, "activation");

	noise_kernel_per->addInput(noise_per);
	noise_kernel_dec->addInput(noise_dec);

	if(train)
	{
		// set up the learning wizard
		dnf_composer::LearningWizard learning_wizard{ simulation, "per - out" };

		std::vector<std::vector<double>> inputTargetPeaksForCoupling =
		{
			{ 90.00 },
			{ 180.00  },
			{ 270.00 },
		};
		std::vector<std::vector<double>> outputTargetPeaksForCoupling =
		{
			{ 15.00 },
			{ 40.00 },
			{ 65.00 , 90.00 },
		};

		learning_wizard.setTargetPeakLocationsForNeuralFieldPre(inputTargetPeaksForCoupling);
		learning_wizard.setTargetPeakLocationsForNeuralFieldPost(outputTargetPeaksForCoupling);

		learning_wizard.simulateAssociation();

		learning_wizard.trainWeights(1000);
		learning_wizard.saveWeights();
	}

	return simulation;
}


int main(int argc, char* argv[])
{
	using namespace dnf_composer;

	try
	{
	    // After defining the simulation, we can create the application.
	    auto simulation = getExperimentSimulation();
	    // You can run the application without the user interface by setting the second parameter to false.
	    constexpr bool activateUserInterface = true;
	    const Application app{ simulation, activateUserInterface };

	    // After creating the application, we can add the windows we want to display.
		app.addWindow<imgui_kit::LogWindow>();
		app.addWindow<user_interface::SimulationWindow>();
		app.addWindow<user_interface::ElementWindow>();

		user_interface::PlotParameters perceptualFieldPlotParams;
		perceptualFieldPlotParams.annotations = { "Perceptual field", "Spatial dimension", "Amplitude" };
		perceptualFieldPlotParams.dimensions = { 0, 360, -12, 20, 0.5};
		auto perceptualFieldVisualization = createVisualization(simulation);
		perceptualFieldVisualization->addPlottingData("perceptual field", "activation");
		perceptualFieldVisualization->addPlottingData("perceptual field", "input");
		perceptualFieldVisualization->addPlottingData("perceptual field", "output");
		app.addWindow<user_interface::PlotWindow>(perceptualFieldVisualization, perceptualFieldPlotParams);

		user_interface::PlotParameters outputFieldPlotParams;
		outputFieldPlotParams.annotations = { "Output field", "Spatial dimension", "Amplitude" };
		outputFieldPlotParams.dimensions = { 0, 180, -12, 20, 0.5};
		auto outputFieldVisualization = createVisualization(simulation);
		outputFieldVisualization->addPlottingData("output field", "activation");
		outputFieldVisualization->addPlottingData("output field", "input");
		outputFieldVisualization->addPlottingData("output field", "output");
		app.addWindow<user_interface::PlotWindow>(outputFieldVisualization, outputFieldPlotParams);

		// test the training by adding a stimulus to the perceptual field
		const element::GaussStimulusParameters gcp_a = { 5, 11, 90};
		const std::shared_ptr<element::GaussStimulus> gauss_stimulus(new element::GaussStimulus({ "gauss stimulus",{360, 0.5} }, gcp_a));
		simulation->addElement(gauss_stimulus);
		simulation->createInteraction("gauss stimulus", "output", "perceptual field");

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