#include "visualization/visualization.h"
#include "application/application.h"
#include "elements/element_factory.h"
#include "elements/supervised_field_coupling.h"
#include "user_interface/main_menu_bar.h"
#include "user_interface/static_layout.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		// Demonstrate supervised (Delta / Widrow-Hoff) learning between two neural fields.
		// Layout:
		//   gs_in  -> nf_source  ─┐
		//                         sfc  ->  nf_output
		//   gs_ref -> nf_ref    ──┘ (reference signal)

		const auto simulation = std::make_shared<Simulation>("Delta rule learning (example)",
			1.0, 0.0, 0.0);

		element::ElementFactory factory;
		const element::ElementDimensions dims{ 100, 1.0 };

		// Source field
		const auto nf_source = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{ element::ElementIdentifiers{"source field"}, dims },
			element::NeuralFieldParameters{});
		const auto mhk_source = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{ element::ElementIdentifiers{"kernel source"}, dims },
			element::MexicanHatKernelParameters{});
		const auto nn_source = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{ element::ElementIdentifiers{"noise source"}, dims },
			element::NormalNoiseParameters{ 0.05 });
		const auto gs_in = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{ element::ElementIdentifiers{"stimulus source"}, dims },
			element::GaussStimulusParameters{ 5.0, 15.0, 30.0 });

		// Output (target) field
		const auto nf_output = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{ element::ElementIdentifiers{"output field"}, dims },
			element::NeuralFieldParameters{});
		const auto mhk_output = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{ element::ElementIdentifiers{"kernel output"}, dims },
			element::MexicanHatKernelParameters{});
		const auto nn_output = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{ element::ElementIdentifiers{"noise output"}, dims },
			element::NormalNoiseParameters{ 0.05 });

		// Reference (teacher) field — drives what the output field should look like
		const auto nf_ref = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{ element::ElementIdentifiers{"reference field"}, dims },
			element::NeuralFieldParameters{});
		const auto mhk_ref = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{ element::ElementIdentifiers{"kernel reference"}, dims },
			element::MexicanHatKernelParameters{});
		const auto nn_ref = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{ element::ElementIdentifiers{"noise reference"}, dims },
			element::NormalNoiseParameters{ 0.05 });
		// Stimulus for reference is at a different location than the source stimulus
		const auto gs_ref = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{ element::ElementIdentifiers{"stimulus reference"}, dims },
			element::GaussStimulusParameters{ 5.0, 15.0, 70.0 });

		// Supervised coupling: learns Delta rule from source -> output using reference as target
		const auto sfc = std::make_shared<element::SupervisedFieldCoupling>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"delta coupling"}, dims },
			element::SupervisedFieldCouplingParameters{ dims, 1.0, 0.1 });

		// Add all elements to simulation
		simulation->addElement(nf_source);
		simulation->addElement(mhk_source);
		simulation->addElement(nn_source);
		simulation->addElement(gs_in);
		simulation->addElement(nf_output);
		simulation->addElement(mhk_output);
		simulation->addElement(nn_output);
		simulation->addElement(nf_ref);
		simulation->addElement(mhk_ref);
		simulation->addElement(nn_ref);
		simulation->addElement(gs_ref);
		simulation->addElement(sfc);

		// Source field recurrent loop + stimulus
		nf_source->addInput(mhk_source);
		mhk_source->addInput(nf_source);
		nf_source->addInput(nn_source);
		nf_source->addInput(gs_in);

		// Output field recurrent loop
		nf_output->addInput(mhk_output);
		mhk_output->addInput(nf_output);
		nf_output->addInput(nn_output);

		// Reference field recurrent loop + its own stimulus
		nf_ref->addInput(mhk_ref);
		mhk_ref->addInput(nf_ref);
		nf_ref->addInput(nn_ref);
		nf_ref->addInput(gs_ref);

		// Wire supervised coupling. The pre-synaptic input is the source field's activation
		// (the user chooses which component to feed the coupling).
		sfc->addInput(nf_source, "activation"); // pre-synaptic input
		sfc->addInput(nf_ref, "reference");     // reference (teacher) signal
		nf_output->addInput(sfc);               // coupling drives output field

		simulation->init();

		// Access the coupling to configure and run learning
		const auto coupling = std::dynamic_pointer_cast<element::SupervisedFieldCoupling>(
			simulation->getElement(sfc->getUniqueName()));
		coupling->clearWeights();
		coupling->setLearning(true);

		constexpr int learningIterations = 100;
		for (int i = 0; i < learningIterations; ++i)
			simulation->step();

		coupling->setLearning(false);
		coupling->writeWeights();

		// Open GUI to inspect learned weights and field dynamics
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 100, -20.0, 20, 1.0, 1.0 },
				PlotAnnotations{ "source field", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_source->getUniqueName(), "activation" }, { nf_source->getUniqueName(), "output" } });

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 100, -20.0, 20, 1.0, 1.0 },
				PlotAnnotations{ "output field", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_output->getUniqueName(), "activation" }, { nf_output->getUniqueName(), "output" } });

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 100, -20.0, 20, 1.0, 1.0 },
				PlotAnnotations{ "reference field", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_ref->getUniqueName(), "activation" }, { nf_ref->getUniqueName(), "output" } });

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{ 0.0, 100, 0.0, 100, 1.0, 1.0 },
				PlotAnnotations{ "learned weights (source -> output)", "output location", "source location" } },
			HeatmapParameters{},
			{ { sfc->getUniqueName(), "weights" } });

		app.init();

		while (!app.hasGUIBeenClosed())
			app.step();

		app.close();
		return 0;
	}
	catch (const dnf_composer::Exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
			"Exception: " + std::string(ex.what()) + " ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())) + ". ",
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
