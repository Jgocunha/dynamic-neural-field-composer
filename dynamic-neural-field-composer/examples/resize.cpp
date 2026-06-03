#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Resize (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// ──────────────────────────────────────────────────────────────────
		// 1D architecture: stimulus a -> field u -> kernel u-u
		//                  field u -> resize u-v -> field v
		// Field v has a different spatial size and discretization than field u.
		// ──────────────────────────────────────────────────────────────────
		const element::ElementDimensions dimU1D(100, 1.0); // size 100, step 1.0
		const element::ElementDimensions dimV1D(50, 2.0);  // size 25,  step 2.0

		const auto sA = std::make_shared<element::GaussStimulus>(
			element::ElementCommonParameters{ "stimulus a (1d)", dimU1D },
			element::GaussStimulusParameters{ 5.0, 10.0, 50.0 });

		const auto u = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ "neural field u (1d)", dimU1D },
			element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		const auto kUU = std::make_shared<element::GaussKernel>(
			element::ElementCommonParameters{ "kernel u-u (1d)", dimU1D },
			element::GaussKernelParameters{ 3.0, 4.0, -0.01, true, true });

		const auto resizeUV = std::make_shared<element::Resize>(
			element::ElementCommonParameters{ "resize u-v (1d)", dimV1D },
			element::ResizeParameters{ element::InterpolationMethod::LINEAR, dimU1D });

		const auto v = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ "neural field v (1d)", dimV1D },
			element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		simulation->addElement(sA);
		simulation->addElement(u);
		simulation->addElement(kUU);
		simulation->addElement(resizeUV);
		simulation->addElement(v);

		u->addInput(sA);
		u->addInput(kUU);
		kUU->addInput(u);
		resizeUV->addInput(u);
		v->addInput(resizeUV);

		visualization->plot({ {u->getUniqueName(), "activation"} });
		visualization->plot({ {v->getUniqueName(), "activation"} });

		// ──────────────────────────────────────────────────────────────────
		// 2D architecture: stimulus a -> field u -> kernel u-u
		//                  field u -> resize u-v -> field v
		// Field v has a different spatial size than field u.
		// ──────────────────────────────────────────────────────────────────
		const element::ElementDimensions dimU2D(50, 50, 1.0, 1.0); // 50 x 50
		const element::ElementDimensions dimV2D(80, 80, 1.0, 1.0); // 80 x 80

		const auto sA2 = std::make_shared<element::GaussStimulus2D>(
			element::ElementCommonParameters{ "stimulus a (2d)", dimU2D },
			element::GaussStimulus2DParameters{ 3.0, 10.0, 25.0, 25.0 });

		const auto u2 = std::make_shared<element::NeuralField2D>(
			element::ElementCommonParameters{ "neural field u (2d)", dimU2D },
			element::NeuralField2DParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		const auto kUU2 = std::make_shared<element::GaussKernel2D>(
			element::ElementCommonParameters{ "kernel u-u (2d)", dimU2D },
			element::GaussKernel2DParameters{ 3.0, 4.0, -0.01, true, true });

		const auto resizeUV2 = std::make_shared<element::Resize2D>(
			element::ElementCommonParameters{ "resize u-v (2d)", dimV2D },
			element::Resize2DParameters{ element::InterpolationMethod::LINEAR, dimU2D });

		const auto v2 = std::make_shared<element::NeuralField2D>(
			element::ElementCommonParameters{ "neural field v (2d)", dimV2D },
			element::NeuralField2DParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		simulation->addElement(sA2);
		simulation->addElement(u2);
		simulation->addElement(kUU2);
		simulation->addElement(resizeUV2);
		simulation->addElement(v2);

		u2->addInput(sA2);
		u2->addInput(kUU2);
		kUU2->addInput(u2);
		resizeUV2->addInput(u2);
		v2->addInput(resizeUV2);

		visualization->plot(PlotCommonParameters{ PlotType::HEATMAP,
			PlotAnnotations{ "Neural field u activation (2d)", "Spatial dimension (x)", "Spatial dimension (y)" } },
			HeatmapParameters{},
			{ {u2->getUniqueName(), "activation"} });
		visualization->plot(PlotCommonParameters{ PlotType::HEATMAP,
			PlotAnnotations{ "Neural field v activation (2d)", "Spatial dimension (x)", "Spatial dimension (y)" } },
			HeatmapParameters{},
			{ {v2->getUniqueName(), "activation"} });

		app.init();

		while (!app.hasGUIBeenClosed())
		{
			app.step();
		}

		app.close();
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
