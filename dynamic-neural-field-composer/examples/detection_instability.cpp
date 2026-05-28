#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Detection instability (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Single timed stimulus — present for 500 ms, then removed
		const auto gscp = element::ElementCommonParameters{ "Gauss stimulus" };
		const auto gsp  = element::TimedGaussStimulusParameters{ 3.0, 5.0, 50.0, {{0, 500}} };
		const auto gs   = std::make_shared<element::TimedGaussStimulus>(gscp, gsp);

		// Detection field — sub-threshold at rest; stimulus drives it above threshold
		const auto nfcp = element::ElementCommonParameters{ "detection field" };
		const auto nfp  = element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{0.0, 5.0} };
		const auto nf   = std::make_shared<element::NeuralField>(nfcp, nfp);

		// Gauss kernel — sub-critical coupling; peak does not self-sustain after stimulus off
		const auto gkcp = element::ElementCommonParameters{ "detection kernel" };
		const auto gkp  = element::GaussKernelParameters{ 3.0, 5.0, -0.25, true, true };
		const auto gk   = std::make_shared<element::GaussKernel>(gkcp, gkp);

		const auto nncp = element::ElementCommonParameters{ "normal noise" };
		const auto nnp  = element::NormalNoiseParameters{ 0.2 };
		const auto nn   = std::make_shared<element::NormalNoise>(nncp, nnp);

		simulation->addElement(gs);
		simulation->addElement(nf);
		simulation->addElement(gk);
		simulation->addElement(nn);

		nf->addInput(gs);
		nf->addInput(gk);
		nf->addInput(nn);
		gk->addInput(nf);

		visualization->plot({ {nf->getUniqueName(), "activation"},
		                      {nf->getUniqueName(), "output"},
		                      {nf->getUniqueName(), "input"} });
		visualization->plot({ {gs->getUniqueName(), "output"} });

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
