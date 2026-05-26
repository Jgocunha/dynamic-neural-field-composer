#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Memory instability 2d (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		const element::ElementDimensions dimensions2D(50, 50, 1.0, 1.0);

		// Single timed stimulus — present for 500 ms, then removed
		const auto gscp = element::ElementCommonParameters{ "Gauss stimulus", dimensions2D };
		const auto gsp  = element::TimedGaussStimulus2DParameters{ 5.0, 12.0, 25.0, 25.0, {{0, 500}} };
		const auto gs   = std::make_shared<element::TimedGaussStimulus2D>(gscp, gsp);

		// Memory field — Mexican hat kernel enables self-sustaining peak after stimulus off
		const auto nfcp = element::ElementCommonParameters{ "memory field", dimensions2D };
		const auto nfp  = element::NeuralField2DParameters{};
		const auto nf   = std::make_shared<element::NeuralField2D>(nfcp, nfp);

		const auto mhcp = element::ElementCommonParameters{ "memory kernel", dimensions2D };
		const auto mhp  = element::MexicanHatKernel2DParameters{3, 25, 5, 10, -0.01};
		const auto mh   = std::make_shared<element::MexicanHatKernel2D>(mhcp, mhp);

		const auto nncp = element::ElementCommonParameters{ "normal noise", dimensions2D };
		const auto nnp  = element::NormalNoise2DParameters{ 0.01 };
		const auto nn   = std::make_shared<element::NormalNoise2D>(nncp, nnp);

		simulation->addElement(gs);
		simulation->addElement(nf);
		simulation->addElement(mh);
		simulation->addElement(nn);

		nf->addInput(gs);
		nf->addInput(mh);
		nf->addInput(nn);
		mh->addInput(nf);

		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Neural field activation", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {nf->getUniqueName(), "activation"} });
		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Stimulus", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {gs->getUniqueName(), "output"} });

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
