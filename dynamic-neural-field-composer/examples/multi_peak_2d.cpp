#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Multi-peak 2d (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		const element::ElementDimensions dimensions2D(50, 50, 1.0, 1.0);

		// Three spatially distributed stimuli driving the field
		const auto sAcp = element::ElementCommonParameters{ "stimulus A", dimensions2D };
		const auto sAp  = element::GaussStimulus2DParameters{ 3.0, 8.0, 15.0, 15.0 };
		const auto sA   = std::make_shared<element::GaussStimulus2D>(sAcp, sAp);

		const auto sBcp = element::ElementCommonParameters{ "stimulus B", dimensions2D };
		const auto sBp  = element::GaussStimulus2DParameters{ 3.0, 8.0, 35.0, 35.0 };
		const auto sB   = std::make_shared<element::GaussStimulus2D>(sBcp, sBp);

		const auto sCcp = element::ElementCommonParameters{ "stimulus C", dimensions2D };
		const auto sCp  = element::GaussStimulus2DParameters{ 3.0, 8.0, 15.0, 35.0 };
		const auto sC   = std::make_shared<element::GaussStimulus2D>(sCcp, sCp);

		// Neural field
		const auto nfcp = element::ElementCommonParameters{ "neural field", dimensions2D };
		const auto nfp  = element::NeuralField2DParameters{ 25.0, -5.0,
			element::SigmoidFunction{0.0, 10.0} };
		const auto nf   = std::make_shared<element::NeuralField2D>(nfcp, nfp);

		// Oscillatory kernel — damped-cosine lateral interactions produce multiple co-existing peaks
		const auto okcp = element::ElementCommonParameters{ "oscillatory kernel", dimensions2D };
		const auto okp  = element::OscillatoryKernel2DParameters{ 1.0, 0.08, 0.3,
			-0.01, true, true };
		const auto ok   = std::make_shared<element::OscillatoryKernel2D>(okcp, okp);

		const auto nncp = element::ElementCommonParameters{ "normal noise", dimensions2D };
		const auto nnp  = element::NormalNoise2DParameters{ 0.1 };
		const auto nn   = std::make_shared<element::NormalNoise2D>(nncp, nnp);

		simulation->addElement(sA);
		simulation->addElement(sB);
		simulation->addElement(sC);
		simulation->addElement(nf);
		simulation->addElement(ok);
		simulation->addElement(nn);

		nf->addInput(sA);
		nf->addInput(sB);
		nf->addInput(sC);
		nf->addInput(ok);
		nf->addInput(nn);
		ok->addInput(nf);

		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Neural field activation", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {nf->getUniqueName(), "activation"} });
		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Stimulus A", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {sA->getUniqueName(), "output"} });
		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Stimulus B", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {sB->getUniqueName(), "output"} });
		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Stimulus C", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {sC->getUniqueName(), "output"} });

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
