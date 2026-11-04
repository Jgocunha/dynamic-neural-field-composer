#include "visualization/visualization.h"
#include "application/application.h"
#include "elements/element_factory.h"
#include "user_interface/main_menu_bar.h"
#include "user_interface/static_layout.h"

// This example demonstrates a bidirectional multi-field architecture:
// two neural fields ("field A" and "field B") that mutually influence
// each other through a pair of learned field couplings, one for each
// direction (A -> B and B -> A). Unlike a purely feedforward chain
// (see weighted_field_coupling.cpp), activity in either field can be
// shaped by activity that itself originated in the other field,
// forming a closed reciprocal loop.

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Bidirectional architecture (example)",
			1.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		element::ElementFactory factory;

		// ── Field A ──────────────────────────────────────────────────────────
		const element::ElementDimensions dimensions_a{200, 1.0};
		const auto nf_a = factory.createElement(element::ElementLabel::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"field A"}, dimensions_a},
			element::NeuralFieldParameters{});
		const auto mhk_a = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel field A"}, dimensions_a},
			element::MexicanHatKernelParameters{});
		const auto nn_a = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{element::ElementIdentifiers{"normal noise field A"}, dimensions_a},
			element::NormalNoiseParameters{0.05});
		const auto gs_a = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{element::ElementIdentifiers{"stimulus field A"}, dimensions_a},
			element::GaussStimulusParameters{5, 15, 30});

		// ── Field B ──────────────────────────────────────────────────────────
		const element::ElementDimensions dimensions_b{200, 1.0};
		const auto nf_b = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"field B"}, dimensions_b},
			element::NeuralFieldParameters{});
		const auto gk_b = factory.createElement(element::GAUSS_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel field B"}, dimensions_b},
			element::GaussKernelParameters{});
		const auto nn_b = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{element::ElementIdentifiers{"normal noise field B"}, dimensions_b},
			element::NormalNoiseParameters{0.05});
		const auto gs_b = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{element::ElementIdentifiers{"stimulus field B"}, dimensions_b},
			element::GaussStimulusParameters{5, 15, 120});

		// ── Bidirectional couplings ─────────────────────────────────────────
		// A -> B: output lives on field B's dimensions, reads from field A.
		const auto fc_ab = factory.createElement(element::FIELD_COUPLING,
			element::ElementCommonParameters{element::ElementIdentifiers{"coupling A-to-B"}, dimensions_b},
			element::FieldCouplingParameters{dimensions_a});
		// B -> A: output lives on field A's dimensions, reads from field B.
		const auto fc_ba = factory.createElement(element::FIELD_COUPLING,
			element::ElementCommonParameters{element::ElementIdentifiers{"coupling B-to-A"}, dimensions_a},
			element::FieldCouplingParameters{dimensions_b});

		simulation->addElement(nf_a);
		simulation->addElement(mhk_a);
		simulation->addElement(nn_a);
		simulation->addElement(gs_a);

		simulation->addElement(nf_b);
		simulation->addElement(gk_b);
		simulation->addElement(nn_b);
		simulation->addElement(gs_b);

		simulation->addElement(fc_ab);
		simulation->addElement(fc_ba);

		nf_a->addInput(mhk_a);
		mhk_a->addInput(nf_a);
		nf_a->addInput(nn_a);
		nf_a->addInput(gs_a);

		nf_b->addInput(gk_b);
		gk_b->addInput(nf_b);
		nf_b->addInput(nn_b);
		nf_b->addInput(gs_b);

		// Close the loop: A feeds B, and B feeds back into A.
		fc_ab->addInput(nf_a);
		nf_b->addInput(fc_ab);

		fc_ba->addInput(nf_b);
		nf_a->addInput(fc_ba);

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 200, -20.0, 20, 1.0, 1.0},
			PlotAnnotations{ "field A dynamics", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_a->getUniqueName(), "activation" }, { nf_a->getUniqueName(), "output" }, { nf_a->getUniqueName(), "input" } });

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 200, -20.0, 20, 1.0, 1.0 },
			PlotAnnotations{ "field B dynamics", "Spatial location", "Activation" } },
			LinePlotParameters{},
			{ {nf_b->getUniqueName(), "activation"}, {nf_b->getUniqueName(), "output"}, {nf_b->getUniqueName(), "input"} });

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{0.0, 200, 0.0, 200, 1.0, 1.0},
				PlotAnnotations{"A-to-B coupling", "field B spatial location", "field A spatial location"} },
				HeatmapParameters{},
			{ {fc_ab->getUniqueName(), "weights"} }
			);

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{0.0, 200, 0.0, 200, 1.0, 1.0},
				PlotAnnotations{"B-to-A coupling", "field A spatial location", "field B spatial location"} },
				HeatmapParameters{},
			{ {fc_ba->getUniqueName(), "weights"} }
			);

		app.init();

		while (!app.hasGUIBeenClosed())
		{
			app.step();
		}

		app.close();

		return 0;
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
