#include "application/application.h"
#include "user_interface/main_window.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/element_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/simulation_window.h"
#include "elements/element_factory.h"
#include "user_interface/plots_window.h"
#include "user_interface/node_graph_window.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("example field coupling learning", 1.0, 0.0, 0.0);

		element::ElementFactory factory;
		const element::ElementDimensions input_dimensions{200, 1.0};
		const auto nf_1 = factory.createElement(element::ElementLabel::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"source field" }, input_dimensions},
			element::NeuralFieldParameters{});
		const auto mhk_1 = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel source field"}, input_dimensions},
			element::MexicanHatKernelParameters{});
		const auto nn_1 = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{element::ElementIdentifiers{"normal noise source field"}, input_dimensions},
			element::NormalNoiseParameters{0.05});
		const auto gs_1 = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{element::ElementIdentifiers{"stimulus source field"}, input_dimensions},
			element::GaussStimulusParameters{5, 15, 20});

		const element::ElementDimensions output_dimensions{400, 1.0};
		const auto nf_2 = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"output field"}, output_dimensions},
			element::NeuralFieldParameters{});
		const auto gk_2 = factory.createElement(element::GAUSS_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel output field"}, output_dimensions},
			element::GaussKernelParameters{});
		const auto nn_2 = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{element::ElementIdentifiers{"normal noise output field"}, output_dimensions},
			element::NormalNoiseParameters{0.05});
		const auto gs_2 = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{element::ElementIdentifiers{"stimulus output field"}, output_dimensions},
			element::GaussStimulusParameters{5, 15, 40});
		const auto fc_1 = factory.createElement(element::FIELD_COUPLING,
			element::ElementCommonParameters{element::ElementIdentifiers{"coupling source-output"}, output_dimensions},
			element::FieldCouplingParameters{input_dimensions});

		simulation->addElement(nf_1);
		simulation->addElement(mhk_1);
		simulation->addElement(nn_1);

		simulation->addElement(nf_2);
		simulation->addElement(gk_2);
		simulation->addElement(nn_2);

		simulation->addElement(gs_1);
		simulation->addElement(gs_2);
		simulation->addElement(fc_1);

		nf_1->addInput(mhk_1);
		mhk_1->addInput(nf_1);
		nf_1->addInput(nn_1);
		nf_1->addInput(gs_1);

		nf_2->addInput(gk_2);
		gk_2->addInput(nf_2);
		nf_2->addInput(nn_2);
		nf_2->addInput(gs_2);

		fc_1->addInput(nf_1);
		nf_2->addInput(fc_1);

		// Initialize the simulation
		// When this is called, the weights are going to be read from the pre-defined directory
		simulation->init();

		// Access the abstract field coupling element
		const auto field_coupling = std::dynamic_pointer_cast<element::FieldCoupling>(simulation->getElement(fc_1->getUniqueName()));

		// Clear the read weights
		field_coupling->clearWeights();

		// Check where weights are being saved and read from
		const std::string weights_directory = field_coupling->getWeightsDirectory();
		// Set the parameters individually
		field_coupling->setLearningRate(0.1f);
		field_coupling->setWeightsDirectory(weights_directory);
		// Set all the parameters at once
		constexpr auto rule = LearningRule::HEBB;
		constexpr double scalar = 1.0f;
		constexpr double learningRate = 0.5f;
		const element::FieldCouplingParameters parameters{input_dimensions, rule, scalar, learningRate};
		field_coupling->setParameters(parameters);

		// Input and output fields are connected to external stimuli,
		// so you can perform associative learning
		field_coupling->setLearning(true);
		constexpr int learningIterations = 50; // Whether these iterations are enough depends on the learning rate and the simulation delta_t
		for (int i = 0; i < learningIterations; ++i)
		 	simulation->step();

		// Turn off learning and save the weights
		field_coupling->setLearning(false);
		field_coupling->writeWeights();

		// Afterward, you can check if the weights are set as you wanted by running the GUI
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainWindow>();
		app.addWindow<imgui_kit::LogWindow>();
		app.addWindow<user_interface::FieldMetricsWindow>();
		app.addWindow<user_interface::ElementWindow>();
		app.addWindow<user_interface::SimulationWindow>();
		app.addWindow<user_interface::PlotControlWindow>();
		app.addWindow<user_interface::PlotsWindow>();
		app.addWindow<user_interface::NodeGraphWindow>();

		// Delete the interaction between the output field and the associative learning stimulus
		nf_2->removeInput(gs_2->getUniqueName());

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 100, -20.0, 20, 1.0, 1.0},
			PlotAnnotations{ "source field dynamics", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_1->getUniqueName(), "activation" }, { nf_1->getUniqueName(), "output" }, { nf_1->getUniqueName(), "input" } });

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 400, -20.0, 20, 1.0, 1.0 },
			PlotAnnotations{ "output field dynamics", "Spatial location", "Activation" } },
			LinePlotParameters{},
			{ {nf_2->getUniqueName(), "activation"}, {nf_2->getUniqueName(), "output"}, {nf_2->getUniqueName(), "input"} });

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{0.0, 400, 0.0, 100, 1.0, 1.0},
				PlotAnnotations{"source-output coupling", "output spatial location", "source spatial location"} },
				HeatmapParameters{},
			{ {fc_1->getUniqueName(), "weights"} }
			);

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
