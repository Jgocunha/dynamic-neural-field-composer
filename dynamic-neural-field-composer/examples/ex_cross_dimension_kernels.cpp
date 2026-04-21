#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"
#include "elements/neural_field.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/gauss_stimulus.h"

// Demonstrates cross-dimension kernel coupling: neural fields of different spatial
// sizes connected via kernel elements (GaussKernel and MexicanHatKernel).
//
// Architecture:
//   field_a (100) <--> mhk_a (100)          self-excitation on the source field
//   field_a (100)  --> gk_ab (in=100,out=50) --> field_b (50)   downscale coupling
//   field_b (50)  <--> gk_b (50)             self-excitation on the target field
//   field_b (50)   --> gk_ba (in=50,out=100) --> field_a (100)  upscale feedback

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("cross-dimension kernels", 1.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Source field: 100 dimensions
		const element::ElementDimensions dim_a{ 100, 1.0 };
		const auto field_a = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"field A (100)"}, dim_a },
			element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		const auto mhk_a = std::make_shared<element::MexicanHatKernel>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"mhk A self"}, dim_a },
			element::MexicanHatKernelParameters{ 3.0, 12.0, 6.0, 16.0, -0.1, true, true });

		const auto noise_a = std::make_shared<element::NormalNoise>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"noise A"}, dim_a },
			element::NormalNoiseParameters{ 0.05 });

		const auto stim_a = std::make_shared<element::GaussStimulus>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"stimulus A"}, dim_a },
			element::GaussStimulusParameters{ 5.0, 10.0, 30.0 });

		// Target field: 50 dimensions
		const element::ElementDimensions dim_b{ 50, 1.0 };
		const auto field_b = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"field B (50)"}, dim_b },
			element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		const auto gk_b = std::make_shared<element::GaussKernel>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"gk B self"}, dim_b },
			element::GaussKernelParameters{ 3.0, 3.0, -0.01, true, true });

		const auto noise_b = std::make_shared<element::NormalNoise>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"noise B"}, dim_b },
			element::NormalNoiseParameters{ 0.05 });

		// Cross-dimension kernels
		// field_a (100) --> field_b (50): downscale
		const auto gk_ab = std::make_shared<element::GaussKernel>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"gk A->B (100->50)"}, dim_a },
			element::GaussKernelParameters{ 5.0, 2.0, -0.005, true, true, element::ElementDimensions{ 50, 1.0 } });

		// field_b (50) --> field_a (100): upscale feedback
		const auto gk_ba = std::make_shared<element::GaussKernel>(
			element::ElementCommonParameters{ element::ElementIdentifiers{"gk B->A (50->100)"}, dim_b },
			element::GaussKernelParameters{ 3.0, 1.5, -0.003, true, true, element::ElementDimensions{ 100, 1.0 } });

		// Add all elements to simulation
		simulation->addElement(field_a);
		simulation->addElement(mhk_a);
		simulation->addElement(noise_a);
		simulation->addElement(stim_a);
		simulation->addElement(field_b);
		simulation->addElement(gk_b);
		simulation->addElement(noise_b);
		simulation->addElement(gk_ab);
		simulation->addElement(gk_ba);

		// Wire field A self-excitation
		mhk_a->addInput(field_a);
		field_a->addInput(mhk_a);
		field_a->addInput(noise_a);
		field_a->addInput(stim_a);

		// Wire field B self-excitation
		gk_b->addInput(field_b);
		field_b->addInput(gk_b);
		field_b->addInput(noise_b);

		// Wire cross-dimension coupling A --> B (downscale 100 -> 50)
		gk_ab->addInput(field_a);
		field_b->addInput(gk_ab);

		// Wire cross-dimension feedback B --> A (upscale 50 -> 100)
		gk_ba->addInput(field_b);
		field_a->addInput(gk_ba);

		// Plots
		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 100.0, -20.0, 20.0, 1.0, 1.0 },
				PlotAnnotations{ "Field A (100 dims)", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { field_a->getUniqueName(), "activation" },
			  { field_a->getUniqueName(), "output" },
			  { field_a->getUniqueName(), "input" } });

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 50.0, -20.0, 20.0, 1.0, 1.0 },
				PlotAnnotations{ "Field B (50 dims)", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { field_b->getUniqueName(), "activation" },
			  { field_b->getUniqueName(), "output" },
			  { field_b->getUniqueName(), "input" } });

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 50.0, -5.0, 5.0, 1.0, 1.0 },
				PlotAnnotations{ "Cross-dim kernel A->B output (in 50-dim space)", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { gk_ab->getUniqueName(), "output" } });

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 100.0, -5.0, 5.0, 1.0, 1.0 },
				PlotAnnotations{ "Cross-dim kernel B->A output (in 100-dim space)", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { gk_ba->getUniqueName(), "output" } });

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
