#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("example boost stimulus", 10.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Boost stimulus — uniform homogeneous input across the field
		const auto bscp = element::ElementCommonParameters{ "Boost stimulus" };
		const auto bsp = element::BoostStimulusParameters{ 5.0, true };
		const auto bs = std::make_shared<element::BoostStimulus>(bscp, bsp);

		// Localized Gauss stimulus to demonstrate categorical responding
		const auto gscp = element::ElementCommonParameters{ "Gauss stimulus" };
		const auto gsp = element::GaussStimulusParameters{ 5.0, 8.0, 50.0 };
		const auto gs = std::make_shared<element::GaussStimulus>(gscp, gsp);

		// Mexican hat kernel for self-excitation / lateral inhibition
		const auto kcp = element::ElementCommonParameters{ "Mexican hat kernel" };
		const auto kp = element::MexicanHatKernelParameters{};
		const auto k = std::make_shared<element::MexicanHatKernel>(kcp, kp);

		const auto nfcp = element::ElementCommonParameters{ "Neural field" };
		const auto nfp = element::NeuralFieldParameters{};
		const auto nf = std::make_shared<element::NeuralField>(nfcp, nfp);

		const auto nncp = element::ElementCommonParameters{ "Normal noise" };
		const auto nnp = element::NormalNoiseParameters{};
		const auto nn = std::make_shared<element::NormalNoise>(nncp, nnp);

		simulation->addElement(bs);
		simulation->addElement(gs);
		simulation->addElement(k);
		simulation->addElement(nf);
		simulation->addElement(nn);

		nf->addInput(bs);
		nf->addInput(gs);
		nf->addInput(k);
		nf->addInput(nn);
		k->addInput(nf);

		visualization->plot({ {nf->getUniqueName(), "activation"},
								{nf->getUniqueName(), "output"},
								{nf->getUniqueName(), "input"} });
		visualization->plot({ {bs->getUniqueName(), "output"},
								{gs->getUniqueName(), "output"} });

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
