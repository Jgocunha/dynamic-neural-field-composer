#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Multi-peak (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Three spatially distributed stimuli driving the field
		const auto sAcp = element::ElementCommonParameters{ "stimulus A" };
		const auto sAp  = element::GaussStimulusParameters{ 3.0, 5.0, 25.0 };
		const auto sA   = std::make_shared<element::GaussStimulus>(sAcp, sAp);

		const auto sBcp = element::ElementCommonParameters{ "stimulus B" };
		const auto sBp  = element::GaussStimulusParameters{ 3.0, 5.0, 50.0 };
		const auto sB   = std::make_shared<element::GaussStimulus>(sBcp, sBp);

		const auto sCcp = element::ElementCommonParameters{ "stimulus C" };
		const auto sCp  = element::GaussStimulusParameters{ 3.0, 5.0, 75.0 };
		const auto sC   = std::make_shared<element::GaussStimulus>(sCcp, sCp);

		// Neural field
		const auto nfcp = element::ElementCommonParameters{ "neural field" };
		const auto nfp  = element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{0.0, 10.0} };
		const auto nf   = std::make_shared<element::NeuralField>(nfcp, nfp);

		// Oscillatory kernel — damped-cosine lateral interactions produce multiple co-existing peaks
		const auto okcp = element::ElementCommonParameters{ "oscillatory kernel" };
		const auto okp  = element::OscillatoryKernelParameters{ 0.7, 0.08, 0.3, -0.01, true, false };
		const auto ok   = std::make_shared<element::OscillatoryKernel>(okcp, okp);

		const auto nncp = element::ElementCommonParameters{ "normal noise" };
		const auto nnp  = element::NormalNoiseParameters{ 0.1 };
		const auto nn   = std::make_shared<element::NormalNoise>(nncp, nnp);

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

		visualization->plot({ {nf->getUniqueName(), "activation"},
		                      {nf->getUniqueName(), "output"},
		                      {nf->getUniqueName(), "input"} });
		visualization->plot({ {sA->getUniqueName(), "output"},
		                      {sB->getUniqueName(), "output"},
		                      {sC->getUniqueName(), "output"} });

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
