#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Collapse / Expand (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// ──────────────────────────────────────────────────────────────────
		// Collapse: 2D field u → Collapse (sum over y) → 1D field v
		// ──────────────────────────────────────────────────────────────────
		const element::ElementDimensions dim2D(50, 50, 1.0, 1.0);
		const element::ElementDimensions dim1Dx(50, 1.0); // kept axis = x

		const auto stim2D = std::make_shared<element::GaussStimulus2D>(
			element::ElementCommonParameters{ "stimulus (2d)", dim2D },
			element::GaussStimulus2DParameters{ 5.0, 10.0, 25.0, 25.0 });

		const auto u2D = std::make_shared<element::NeuralField2D>(
			element::ElementCommonParameters{ "neural field u (2d)", dim2D },
			element::NeuralField2DParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		const auto collapse = std::make_shared<element::Collapse>(
			element::ElementCommonParameters{ "collapse u-v", dim1Dx },
			element::CollapseParameters{ element::CompressionType::SUM, element::ProjectionAxis::X, dim2D });

		const auto vCollapsed = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ "neural field v (1d)", dim1Dx },
			element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		simulation->addElement(stim2D);
		simulation->addElement(u2D);
		simulation->addElement(collapse);
		simulation->addElement(vCollapsed);

		u2D->addInput(stim2D);
		collapse->addInput(u2D);
		vCollapsed->addInput(collapse);

		// ──────────────────────────────────────────────────────────────────
		// Expand: 1D field a → Expand (broadcast along y) → 2D field b
		// ──────────────────────────────────────────────────────────────────
		const element::ElementDimensions dim1D(50, 1.0);
		const element::ElementDimensions dim2Db(50, 50, 1.0, 1.0);

		const auto stim1D = std::make_shared<element::GaussStimulus>(
			element::ElementCommonParameters{ "stimulus (1d)", dim1D },
			element::GaussStimulusParameters{ 5.0, 10.0, 25.0 });

		const auto a1D = std::make_shared<element::NeuralField>(
			element::ElementCommonParameters{ "neural field a (1d)", dim1D },
			element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		const auto expand = std::make_shared<element::Expand>(
			element::ElementCommonParameters{ "expand a-b", dim2Db },
			element::ExpandParameters{ element::ProjectionAxis::X, dim1D });

		const auto bExpanded = std::make_shared<element::NeuralField2D>(
			element::ElementCommonParameters{ "neural field b (2d)", dim2Db },
			element::NeuralField2DParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

		simulation->addElement(stim1D);
		simulation->addElement(a1D);
		simulation->addElement(expand);
		simulation->addElement(bExpanded);

		a1D->addInput(stim1D);
		expand->addInput(a1D);
		bExpanded->addInput(expand);

		// Plots: 2D source u (heatmap) and its 1D collapse v (line);
		//        1D source a (line) and its 2D expansion b (heatmap).
		visualization->plot(PlotCommonParameters{ PlotType::HEATMAP,
			PlotAnnotations{ "Neural field u activation (2d)", "Spatial dimension (x)", "Spatial dimension (y)" } },
			HeatmapParameters{},
			{ {u2D->getUniqueName(), "activation"} });
		visualization->plot({ {vCollapsed->getUniqueName(), "activation"} });
		visualization->plot({ {a1D->getUniqueName(), "activation"} });
		visualization->plot(PlotCommonParameters{ PlotType::HEATMAP,
			PlotAnnotations{ "Neural field b activation (2d)", "Spatial dimension (x)", "Spatial dimension (y)" } },
			HeatmapParameters{},
			{ {bExpanded->getUniqueName(), "activation"} });

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
