#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const element::ElementDimensions dims2D(50, 50, 1.0, 1.0);

		const auto simulation    = std::make_shared<Simulation>("ex oscillatory kernel 2d", 10.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Localised Gaussian input
		const auto gscp = element::ElementCommonParameters{ "gauss stimulus 2d", dims2D };
		const auto gsp  = element::GaussStimulusParameters2D{ 5.0, 15.0, 25.0, 25.0, true, false };
		const auto gs   = std::make_shared<element::GaussStimulus2D>(gscp, gsp);

		// Oscillatory lateral-interaction kernel
		const auto okcp = element::ElementCommonParameters{ "oscillatory kernel 2d", dims2D };
		const auto okp  = element::OscillatoryKernel2DParameters{ 1.0, 0.08, 0.3, -0.01, true, false };
		const auto ok   = std::make_shared<element::OscillatoryKernel2D>(okcp, okp);

		// Background noise
		const auto nncp = element::ElementCommonParameters{ "normal noise 2d", dims2D };
		const auto nnp  = element::NormalNoise2DParameters{ 0.1 };
		const auto nn   = std::make_shared<element::NormalNoise2D>(nncp, nnp);

		// 2D neural field
		const auto nfcp = element::ElementCommonParameters{ "neural field 2d", dims2D };
		const auto nfp  = element::NeuralField2DParameters{ 25.0, -5.0, element::SigmoidFunction(0.0, 10.0) };
		const auto nf   = std::make_shared<element::NeuralField2D>(nfcp, nfp);

		simulation->addElement(gs);
		simulation->addElement(ok);
		simulation->addElement(nn);
		simulation->addElement(nf);

		nf->addInput(gs);
		nf->addInput(ok);
		nf->addInput(nn);
		ok->addInput(nf);

		visualization->plot({ {nf->getUniqueName(), "activation"} });
		visualization->plot({ {ok->getUniqueName(), "output"} });

		app.init();
		while (!app.hasGUIBeenClosed())
			app.step();
		app.close();
	}
	catch (const dnf_composer::Exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
		    "Exception: " + std::string(ex.what()),
		    dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return static_cast<int>(ex.getErrorCode());
	}
	catch (const std::exception& ex)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
		    "Exception caught: " + std::string(ex.what()),
		    dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
	catch (...)
	{
		log(dnf_composer::tools::logger::LogLevel::FATAL,
		    "Unknown exception occurred.",
		    dnf_composer::tools::logger::LogOutputMode::CONSOLE);
		return 1;
	}
}
