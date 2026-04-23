#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"

// Competitive Selection example
//
// Three localized stimuli (options A, B, C) feed a single selection field.
// A Mexican-hat kernel provides local self-excitation and broad lateral
// inhibition — only the option whose input crosses threshold first forms a
// stable peak; the others are suppressed (winner-takes-all).
//
// Try it:
//   - Run with default parameters: option B (strongest) wins.
//   - Reduce option B amplitude below option A or C: the winner shifts.
//   - Give two options equal amplitude: noise breaks the tie stochastically.

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("example competitive selection", 25.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// ── Option stimuli ────────────────────────────────────────────────────
		// Three options at positions 25, 50, 75 with graded amplitudes.
		// Amplitudes are intentionally the same so small changes shift the winner.
		const auto sAcp = element::ElementCommonParameters{ "option A stimulus" };
		const auto sAp  = element::GaussStimulusParameters{ 3.0, 5.0, 25.0 };
		const auto sA   = std::make_shared<element::GaussStimulus>(sAcp, sAp);

		const auto sBcp = element::ElementCommonParameters{ "option B stimulus" };
		const auto sBp  = element::GaussStimulusParameters{ 3.0, 5.0, 50.0 };
		const auto sB   = std::make_shared<element::GaussStimulus>(sBcp, sBp);

		const auto sCcp = element::ElementCommonParameters{ "option C stimulus" };
		const auto sCp  = element::GaussStimulusParameters{ 3.0, 5.0, 75.0 };
		const auto sC   = std::make_shared<element::GaussStimulus>(sCcp, sCp);

		// ── Selection field ───────────────────────────────────────────────────
		// Sub-threshold resting level; stimuli push individual locations toward
		// detection; lateral inhibition prevents two peaks from co-existing.
		const auto nfcp = element::ElementCommonParameters{ "selection field" };
		const auto nfp  = element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{0.0, 5.0} };
		const auto nf   = std::make_shared<element::NeuralField>(nfcp, nfp);

		const auto gkcp = element::ElementCommonParameters{ "selection kernel" };
		const auto gkp  = element::GaussKernelParameters{ 3.0, 5.0, -0.25, true, true };
		const auto gk   = std::make_shared<element::GaussKernel>(gkcp, gkp);

		// ── Normal noise ──────────────────────────────────────────────────────
		// Small noise is enough to break symmetry when two stimuli are equal.
		const auto nncp = element::ElementCommonParameters{ "normal noise" };
		const auto nnp  = element::NormalNoiseParameters{ 0.2 };
		const auto nn   = std::make_shared<element::NormalNoise>(nncp, nnp);

		simulation->addElement(sA);
		simulation->addElement(sB);
		simulation->addElement(sC);
		simulation->addElement(nf);
		simulation->addElement(gk);
		simulation->addElement(nn);

		nf->addInput(sA);
		nf->addInput(sB);
		nf->addInput(sC);
		nf->addInput(gk);
		nf->addInput(nn);
		gk->addInput(nf);

		// ── Plots ─────────────────────────────────────────────────────────────
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
