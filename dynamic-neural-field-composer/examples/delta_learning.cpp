#include "visualization/visualization.h"
#include "application/application.h"
#include "elements/element_factory.h"
#include "user_interface/main_menu_bar.h"
#include "user_interface/static_layout.h"
#include "example_common.h"

// Demonstrates the supervised DELTA (Widrow-Hoff) learning rule: unlike
// HEBB/OJA, it requires a third, externally supplied teaching signal
// connected to the field coupling's "target" slot -- see
// FieldCoupling::addInput(target, "target"). Input and target are wired from
// their fields' raw "activation" here (see addInput(..., "activation") /
// addInput(..., "target:activation")) rather than the sigmoided "output",
// so the learning rule sees each field's full unsaturated activation profile
// instead of a near-binary bump. Weights grow directly from the error
// (target minus the coupling's own prediction) -- no drive on the output
// field beyond the coupling itself is needed for learning to start.
int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Delta rule learning (example)",
			1.0, 0.0, 0.0);

		element::ElementFactory factory;
		const element::ElementDimensions input_dimensions{200, 1.0};
		const auto nf_per = factory.createElement(element::ElementLabel::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"perceptual field"}, input_dimensions},
			element::NeuralFieldParameters{});
		const auto mhk_per = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel perceptual field"}, input_dimensions},
			element::MexicanHatKernelParameters{});
		const auto gs_per = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{element::ElementIdentifiers{"perceptual stimulus"}, input_dimensions},
			element::GaussStimulusParameters{5, 15, 80});

		const element::ElementDimensions output_dimensions{200, 1.0};
		const auto nf_out = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"output field"}, output_dimensions},
			element::NeuralFieldParameters{});
		const auto mhk_out = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel output field"}, output_dimensions},
			element::MexicanHatKernelParameters{});
		// The teaching signal: during learning this field is driven to the
		// target placement, and the coupling learns to reproduce it (via its
		// raw activation, see fc_1's wiring below) from the perceptual
		// field's activation alone.
		const auto nf_tar = factory.createElement(element::NEURAL_FIELD,
			element::ElementCommonParameters{element::ElementIdentifiers{"target field"}, output_dimensions},
			element::NeuralFieldParameters{});
		const auto mhk_tar = factory.createElement(element::MEXICAN_HAT_KERNEL,
			element::ElementCommonParameters{element::ElementIdentifiers{"kernel target field"}, output_dimensions},
			element::MexicanHatKernelParameters{});
		const auto gs_tar = factory.createElement(element::GAUSS_STIMULUS,
			element::ElementCommonParameters{element::ElementIdentifiers{"target stimulus"}, output_dimensions},
			element::GaussStimulusParameters{5, 15, 120});

		constexpr auto rule = LearningRule::DELTA;
		constexpr double scalar = 1.0;
		// Small: pre is raw, unbounded activation (not the sigmoided "output"),
		// summed over all 200 input positions each step -- a much larger
		// effective step than HEBB/OJA's normalized-activation update.
		constexpr double learningRate = 0.00001;
		constexpr double decayRate = 0.0;
		const auto fc_1 = factory.createElement(element::FIELD_COUPLING,
			element::ElementCommonParameters{element::ElementIdentifiers{"coupling per-out"}, output_dimensions},
			element::FieldCouplingParameters{input_dimensions, rule, scalar, learningRate, decayRate});

		simulation->addElement(nf_per);
		simulation->addElement(mhk_per);
		simulation->addElement(gs_per);
		simulation->addElement(nf_out);
		simulation->addElement(mhk_out);
		simulation->addElement(nf_tar);
		simulation->addElement(mhk_tar);
		simulation->addElement(gs_tar);
		simulation->addElement(fc_1);

		nf_per->addInput(gs_per);
		nf_per->addInput(mhk_per);
		mhk_per->addInput(nf_per);

		nf_tar->addInput(gs_tar);
		nf_tar->addInput(mhk_tar);
		mhk_tar->addInput(nf_tar);

		nf_out->addInput(mhk_out);
		mhk_out->addInput(nf_out);

		fc_1->addInput(nf_per, "activation");            // input: u_per (raw activation, not g(u_per))
		nf_out->addInput(fc_1);                          // fc's forward pass drives the output field
		fc_1->addInput(nf_tar, "target:activation");     // target: u_tar (raw activation), the teaching signal

		simulation->init();

		const auto field_coupling = std::dynamic_pointer_cast<element::FieldCoupling>(
			simulation->getElement(fc_1->getUniqueName()));
		field_coupling->clearWeights();

		// Learn the perceptual-field-to-target association.
		field_coupling->setLearning(true);
		constexpr int learningIterations = 400;
		for (int i = 0; i < learningIterations; ++i) {
			simulation->step();
		}
		field_coupling->setLearning(false);
		// The learned weights are individually small (max |w| ~ 0.009 here). That is
		// the arithmetically correct scale, not a sign of under-training: the forward
		// pass sums W over all 200 input positions, so reproducing an output peak of
		// ~8.7 needs each weight to be roughly 8.7/200. The heatmap plot below shows
		// the learned structure -- a cross linking the perceptual peak (80) to the
		// target peak (120) -- even though the colorbar values look near zero.
		field_coupling->writeWeights();

		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		// Remove the target stimulus so, once the GUI opens, only the learned
		// coupling drives the output field toward the target it was taught.
		nf_tar->removeInput(gs_tar->getUniqueName());

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 200, -20.0, 20, 1.0, 1.0 },
			PlotAnnotations{ "perceptual field", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_per->getUniqueName(), "activation" }, { nf_per->getUniqueName(), "output" } });

		visualization->plot(
			PlotCommonParameters{
			PlotType::LINE_PLOT,
			PlotDimensions{ 0.0, 200, -20.0, 20, 1.0, 1.0 },
			PlotAnnotations{ "output field vs. target", "Spatial location", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_out->getUniqueName(), "activation" }, { nf_tar->getUniqueName(), "activation" } });

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{0.0, 200, 0.0, 200, 1.0, 1.0},
				PlotAnnotations{"perceptual-output coupling", "output spatial location", "perceptual spatial location"} },
				HeatmapParameters{},
			{ {fc_1->getUniqueName(), "weights"} }
			);

		app.init();

		dnf_composer::examples::runExampleLoop(app);

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
