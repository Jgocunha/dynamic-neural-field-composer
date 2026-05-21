#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"


int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Memory trace (example)", 10.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Gauss stimulus — localized input; set amplitude to 0 via the parameter panel to simulate removal
		const auto gscp = element::ElementCommonParameters{ "Gauss stimulus" };
		const auto gsp = element::TimedGaussStimulusParameters{ 5.0, 12.0, 50.0, {{0, 500}} };
		const auto gs = std::make_shared<element::TimedGaussStimulus>(gscp, gsp);

		// Memory trace — captures field output and sustains it after input is removed
		const auto mtcp = element::ElementCommonParameters{ "Memory trace" };
		const auto mtp = element::MemoryTraceParameters{ 50.0, 500.0, 0.5 };
		const auto mt = std::make_shared<element::MemoryTrace>(mtcp, mtp);

		// Gauss kernel — feeds memory trace output back into the field
		const auto gkcp = element::ElementCommonParameters{ "Feedback kernel" };
		const auto gkp = element::GaussKernelParameters{ 5.0, 3.0, 0.0, true, true };
		const auto gk = std::make_shared<element::GaussKernel>(gkcp, gkp);

		// Mexican hat kernel — lateral interactions for self-sustaining peak
		const auto mhcp = element::ElementCommonParameters{ "Mexican hat kernel" };
		const auto mhp = element::MexicanHatKernelParameters{};
		const auto mh = std::make_shared<element::MexicanHatKernel>(mhcp, mhp);

		// Neural field — the main field whose activation is memorized
		const auto nfcp = element::ElementCommonParameters{ "Neural field" };
		const auto nfp = element::NeuralFieldParameters{};
		const auto nf = std::make_shared<element::NeuralField>(nfcp, nfp);

		// Normal noise
		const auto nncp = element::ElementCommonParameters{ "Normal noise" };
		const auto nnp = element::NormalNoiseParameters{};
		const auto nn = std::make_shared<element::NormalNoise>(nncp, nnp);

		simulation->addElement(gs);
		simulation->addElement(mt);
		simulation->addElement(gk);
		simulation->addElement(mh);
		simulation->addElement(nf);
		simulation->addElement(nn);

		// Field receives: stimulus + memory feedback + lateral interactions + noise
		nf->addInput(gs);
		nf->addInput(gk);
		nf->addInput(mh);
		nf->addInput(nn);

		// Memory trace monitors field output
		mt->addInput(nf);

		// Memory trace output feeds back through a kernel into the field
		gk->addInput(mt);

		// Lateral interactions
		mh->addInput(nf);

		visualization->plot({ {nf->getUniqueName(), "activation"},
								{nf->getUniqueName(), "output"},
								{nf->getUniqueName(), "input"} });
		visualization->plot({ {mt->getUniqueName(), "output"},
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
