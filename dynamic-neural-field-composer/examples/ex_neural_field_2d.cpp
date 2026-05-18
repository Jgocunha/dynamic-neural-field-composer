#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		// 50x50 grid, d_x = d_y = 1.0
		const element::ElementDimensions dims2D(50, 50, 1.0, 1.0);

		const auto simulation = std::make_shared<Simulation>("example neural field 2d", 10.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Localized Gaussian input stimulus centred at (25, 25)
		const auto gscp = element::ElementCommonParameters{ "Gauss stimulus 2d", dims2D };
		const auto gsp  = element::GaussStimulusParameters2D{ 5.0, 15.0, 25.0, 25.0, true, false };
		const auto gs   = std::make_shared<element::GaussStimulus2D>(gscp, gsp);

		// Mexican hat kernel — local excitation, surround inhibition
		const auto mhcp = element::ElementCommonParameters{ "Mexican hat kernel 2d", dims2D };
		const auto mhp  = element::MexicanHatKernel2DParameters{ 2.5, 11.0, 5.0, 15.0, -0.1, true, true };
		const auto mh   = std::make_shared<element::MexicanHatKernel2D>(mhcp, mhp);

		// Background noise
		const auto nncp = element::ElementCommonParameters{ "Normal noise 2d", dims2D };
		const auto nnp  = element::NormalNoise2DParameters{ 0.1 };
		const auto nn   = std::make_shared<element::NormalNoise2D>(nncp, nnp);

		// 2D neural field
		const auto nfcp = element::ElementCommonParameters{ "Neural field 2d", dims2D };
		const auto nfp  = element::NeuralField2DParameters{ 25.0, -5.0, element::SigmoidFunction(0.0, 10.0) };
		const auto nf   = std::make_shared<element::NeuralField2D>(nfcp, nfp);

		simulation->addElement(gs);
		simulation->addElement(mh);
		simulation->addElement(nn);
		simulation->addElement(nf);

		// Field receives: stimulus + lateral interactions + noise
		nf->addInput(gs);
		nf->addInput(mh);
		nf->addInput(nn);

		// Lateral interactions read field output
		mh->addInput(nf);

		// Visualize field activation as a heatmap (50 rows × 50 cols)
		visualization->plot({ {nf->getUniqueName(), "activation"} });
		visualization->plot({ {gs->getUniqueName(), "output"} });

		app.init();

		while (!app.hasGUIBeenClosed())
			app.step();

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
