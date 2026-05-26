#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Selection instability 2d (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		const element::ElementDimensions dimensions2D(100, 100, 1.0, 1.0);
		// ── Option stimuli ────────────────────────────────────────────────────
		// Three options at positions (25, 25), (50, 50) , (75, 75).
		// Amplitudes are intentionally the same, so small changes (noise) shift the winner.
		const auto sAcp = element::ElementCommonParameters{ "option A stimulus", dimensions2D};
		const auto sAp  = element::GaussStimulusParameters2D{ 3.0, 8.0, 25.0, 25.0 };
		const auto sA   = std::make_shared<element::GaussStimulus2D>(sAcp, sAp);

		const auto sBcp = element::ElementCommonParameters{ "option B stimulus", dimensions2D};
		const auto sBp  = element::GaussStimulusParameters2D{3.0, 8.0, 50, 50};
		const auto sB   = std::make_shared<element::GaussStimulus2D>(sBcp, sBp);

		const auto sCcp = element::ElementCommonParameters{ "option C stimulus", dimensions2D};
		const auto sCp  = element::GaussStimulusParameters2D{ 3.0, 8.0, 75.0, 75.0};
		const auto sC   = std::make_shared<element::GaussStimulus2D>(sCcp, sCp);

		// ── Selection field ───────────────────────────────────────────────────
		// Sub-threshold resting level; stimuli push individual locations toward
		// detection; lateral inhibition prevents two peaks from co-existing.
		const auto nfcp = element::ElementCommonParameters{ "selection field", dimensions2D};
		const auto nfp  = element::NeuralField2DParameters{ 25.0, -7.8, element::SigmoidFunction{0.0, 5.0} };
		const auto nf   = std::make_shared<element::NeuralField2D>(nfcp, nfp);

		const auto gkcp = element::ElementCommonParameters{ "selection kernel", dimensions2D};
		const auto gkp  = element::GaussKernel2DParameters{ 3.0, 23.0, -0.2, true, true };
		const auto gk   = std::make_shared<element::GaussKernel2D>(gkcp, gkp);

		// ── Normal noise ──────────────────────────────────────────────────────
		// Small noise is enough to break symmetry when two (or more) stimuli are equal.
		const auto nncp = element::ElementCommonParameters{ "normal noise", dimensions2D};
		const auto nnp  = element::NormalNoise2DParameters{ 0.2 };
		const auto nn   = std::make_shared<element::NormalNoise2D>(nncp, nnp);

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
