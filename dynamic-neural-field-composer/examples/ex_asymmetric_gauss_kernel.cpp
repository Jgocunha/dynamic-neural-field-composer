#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"
// Asymmetric Gauss Kernel example
//
// A single neural field with an AsymmetricGaussKernel for self-excitation.
// Unlike a symmetric kernel, the asymmetric variant has different rise and
// fall widths, which creates a directional bias: activated peaks drift
// along the field dimension over time.
//
// Architecture:
//   GaussStimulus --> NeuralField <--> AsymmetricGaussKernel
//                         ^
//                    NormalNoise
//
// Try it:
//   - Observe the peak drifting in one direction after stimulus onset.
//   - Flip the kernel's asymmetry parameter to reverse the drift direction.
//   - Compare against a standard GaussKernel to see the symmetry difference.

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("example asymmetric gauss kernel", 10.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);

		const auto gscp_1 = element::ElementCommonParameters{ "Gauss stimulus" };
		const auto gsp_1 = element::GaussStimulusParameters{ 5, 15, 20 };
		const auto gs_1 = std::make_shared < element::GaussStimulus > (gscp_1, gsp_1);

		const auto gkcp_1 = element::ElementCommonParameters{ "Self-excitation asymmetric gauss kernel" };
		const auto gkp_1 = element::AsymmetricGaussKernelParameters{ 6.0, 14.0, -0.116, 0.0 };
		const auto gk_1 = std::make_shared < element::AsymmetricGaussKernel > ( gkcp_1, gkp_1 );

		const auto nfcp_1 = element::ElementCommonParameters{ "Neural field" };
		const auto nfp_1 = element::NeuralFieldParameters{};
		const auto nf_1 = std::make_shared < element::NeuralField > ( nfcp_1, nfp_1 );

		const auto nncp_1 = element::ElementCommonParameters{ "Normal noise" };
		const auto nnp_1 = element::NormalNoiseParameters{};
		const auto nn_1 = std::make_shared < element::NormalNoise > (nncp_1, nnp_1);

		simulation->addElement(gs_1);
		simulation->addElement(gk_1);
		simulation->addElement(nf_1);
		simulation->addElement(nn_1);

		nf_1->addInput(gs_1);
		nf_1->addInput(gk_1);
		gk_1->addInput(nf_1);
		nf_1->addInput(nn_1);

		visualization->plot({ {nf_1->getUniqueName(), "activation"},
								{nf_1->getUniqueName(), "output"},
								{nf_1->getUniqueName(), "input"},
								{gs_1->getUniqueName(), "output"}, });
		visualization->plot({ {gk_1->getUniqueName(), "kernel"} });
		visualization->plot({ {gk_1->getUniqueName(), "output"} });

		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

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
