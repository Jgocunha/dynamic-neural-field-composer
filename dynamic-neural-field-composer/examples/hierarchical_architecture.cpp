#include "visualization/visualization.h"
#include "application/application.h"
#include "elements/element_factory.h"
#include "user_interface/main_menu_bar.h"
#include "user_interface/static_layout.h"

// This example demonstrates a hierarchical multi-field architecture:
// three neural fields arranged in levels of increasing abstraction
// ("low", "mid", "high"). Activity is driven bottom-up through
// feedforward field couplings (low -> mid -> high), and the highest
// level in turn sends a top-down field coupling back down to the mid
// level, modulating it with feedback. Only the lowest level receives
// an external stimulus; the higher levels are driven entirely through
// the coupling structure, illustrating how a hierarchy of fields can
// be composed from the same coupling building blocks used for simple
// feedforward or bidirectional pairs.

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Hierarchical architecture (example)",
			1.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		element::ElementFactory factory;

		// ── Low level: raw, high-resolution sensory field ───────────────────
		const element::ElementDimensions dimensions_low{280, 1.0};
		const auto nf_low = factory.createElement(element::ElementLabel::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"low level field"}, dimensions_low},
			element::NeuralFieldParameters{});
		const auto mhk_low = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel low level field"}, dimensions_low},
			element::MexicanHatKernelParameters{});
		const auto nn_low = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{element::ElementIdentifiers{"normal noise low level field"}, dimensions_low},
			element::NormalNoiseParameters{0.05});
		const auto gs_low = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{element::ElementIdentifiers{"stimulus low level field"}, dimensions_low},
			element::GaussStimulusParameters{5, 15, 40});

		// ── Mid level: intermediate abstraction, coarser resolution ─────────
		const element::ElementDimensions dimensions_mid{150, 1.0};
		const auto nf_mid = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"mid level field"}, dimensions_mid},
			element::NeuralFieldParameters{});
		const auto gk_mid = factory.createElement(element::GAUSS_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel mid level field"}, dimensions_mid},
			element::GaussKernelParameters{});
		const auto nn_mid = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{element::ElementIdentifiers{"normal noise mid level field"}, dimensions_mid},
			element::NormalNoiseParameters{0.05});

		// ── High level: most abstract, coarsest resolution ──────────────────
		const element::ElementDimensions dimensions_high{80, 1.0};
		const auto nf_high = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"high level field"}, dimensions_high},
			element::NeuralFieldParameters{});
		const auto gk_high = factory.createElement(element::GAUSS_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel high level field"}, dimensions_high},
			element::GaussKernelParameters{});
		const auto nn_high = factory.createElement(element::NORMAL_NOISE,
			element::ElementCommonParameters{element::ElementIdentifiers{"normal noise high level field"}, dimensions_high},
			element::NormalNoiseParameters{0.05});

		// ── Feedforward couplings (bottom-up) ───────────────────────────────
		const auto fc_low_mid = factory.createElement(element::FIELD_COUPLING,
			element::ElementCommonParameters{element::ElementIdentifiers{"coupling low-to-mid"}, dimensions_mid},
			element::FieldCouplingParameters{dimensions_low});
		const auto fc_mid_high = factory.createElement(element::FIELD_COUPLING,
			element::ElementCommonParameters{element::ElementIdentifiers{"coupling mid-to-high"}, dimensions_high},
			element::FieldCouplingParameters{dimensions_mid});

		// ── Feedback coupling (top-down) ─────────────────────────────────────
		const auto fc_high_mid = factory.createElement(element::FIELD_COUPLING,
			element::ElementCommonParameters{element::ElementIdentifiers{"coupling high-to-mid (feedback)"}, dimensions_mid},
			element::FieldCouplingParameters{dimensions_high});

		simulation->addElement(nf_low);
		simulation->addElement(mhk_low);
		simulation->addElement(nn_low);
		simulation->addElement(gs_low);

		simulation->addElement(nf_mid);
		simulation->addElement(gk_mid);
		simulation->addElement(nn_mid);

		simulation->addElement(nf_high);
		simulation->addElement(gk_high);
		simulation->addElement(nn_high);

		simulation->addElement(fc_low_mid);
		simulation->addElement(fc_mid_high);
		simulation->addElement(fc_high_mid);

		nf_low->addInput(mhk_low);
		mhk_low->addInput(nf_low);
		nf_low->addInput(nn_low);
		nf_low->addInput(gs_low);

		nf_mid->addInput(gk_mid);
		gk_mid->addInput(nf_mid);
		nf_mid->addInput(nn_mid);

		nf_high->addInput(gk_high);
		gk_high->addInput(nf_high);
		nf_high->addInput(nn_high);

		// Bottom-up: low drives mid, mid drives high.
		fc_low_mid->addInput(nf_low);
		nf_mid->addInput(fc_low_mid);

		fc_mid_high->addInput(nf_mid);
		nf_high->addInput(fc_mid_high);

		// Top-down: high feeds back into mid.
		fc_high_mid->addInput(nf_high);
		nf_mid->addInput(fc_high_mid);

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 280, -20.0, 20, 1.0, 1.0},
			PlotAnnotations{ "low level field dynamics", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_low->getUniqueName(), "activation" }, { nf_low->getUniqueName(), "output" }, { nf_low->getUniqueName(), "input" } });

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 150, -20.0, 20, 1.0, 1.0 },
			PlotAnnotations{ "mid level field dynamics", "Spatial location", "Activation" } },
			LinePlotParameters{},
			{ {nf_mid->getUniqueName(), "activation"}, {nf_mid->getUniqueName(), "output"}, {nf_mid->getUniqueName(), "input"} });

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 80, -20.0, 20, 1.0, 1.0 },
			PlotAnnotations{ "high level field dynamics", "Spatial location", "Activation" } },
			LinePlotParameters{},
			{ {nf_high->getUniqueName(), "activation"}, {nf_high->getUniqueName(), "output"}, {nf_high->getUniqueName(), "input"} });

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{0.0, 150, 0.0, 280, 1.0, 1.0},
				PlotAnnotations{"low-to-mid coupling", "mid spatial location", "low spatial location"} },
				HeatmapParameters{},
			{ {fc_low_mid->getUniqueName(), "weights"} }
			);

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{0.0, 80, 0.0, 150, 1.0, 1.0},
				PlotAnnotations{"mid-to-high coupling", "high spatial location", "mid spatial location"} },
				HeatmapParameters{},
			{ {fc_mid_high->getUniqueName(), "weights"} }
			);

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{0.0, 150, 0.0, 80, 1.0, 1.0},
				PlotAnnotations{"high-to-mid feedback coupling", "mid spatial location", "high spatial location"} },
				HeatmapParameters{},
			{ {fc_high_mid->getUniqueName(), "weights"} }
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
