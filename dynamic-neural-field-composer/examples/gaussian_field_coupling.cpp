#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try
	{
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Gaussian field coupling (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);
		const Application app{ simulation, visualization };

		app.addWindow<user_interface::MainMenuBar>();
		app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

		const element::ElementDimensions input_dim{ 200, 0.5 };
		const element::ElementDimensions output_dim{ 100, 1.0 };

		// ── Input field ───────────────────────────────────────────────────────
		const auto nf_in_cp = element::ElementCommonParameters{ "input field", input_dim };
		const auto nf_in    = std::make_shared<element::NeuralField>(nf_in_cp, element::NeuralFieldParameters{});

		const auto gk_in_cp = element::ElementCommonParameters{ "input kernel", input_dim };
		const auto gk_in    = std::make_shared<element::GaussKernel>(gk_in_cp, element::GaussKernelParameters{});

		const auto gs_in_cp = element::ElementCommonParameters{ "input stimulus", input_dim };
		const auto gs_in_p  = element::GaussStimulusParameters{ 5.0, 10.0, 50.0 };
		const auto gs_in    = std::make_shared<element::GaussStimulus>(gs_in_cp, gs_in_p);

		const auto nn_in_cp = element::ElementCommonParameters{ "input noise", input_dim };
		const auto nn_in    = std::make_shared<element::NormalNoise>(nn_in_cp, element::NormalNoiseParameters{});

		// ── Output field ──────────────────────────────────────────────────────
		const auto nf_out_cp = element::ElementCommonParameters{ "output field", output_dim };
		const auto nf_out    = std::make_shared<element::NeuralField>(nf_out_cp, element::NeuralFieldParameters{});

		const auto mh_out_cp = element::ElementCommonParameters{ "output kernel", output_dim };
		const auto mh_out    = std::make_shared<element::MexicanHatKernel>(mh_out_cp, element::MexicanHatKernelParameters{});

		const auto nn_out_cp = element::ElementCommonParameters{ "output noise", output_dim };
		const auto nn_out    = std::make_shared<element::NormalNoise>(nn_out_cp, element::NormalNoiseParameters{});

		// ── Gaussian field coupling ───────────────────────────────────────────
		// Each entry: {input_position, output_position, sigma_input, sigma_output}
		const auto gfc_cp = element::ElementCommonParameters{ "gauss field coupling", output_dim };
		const auto gfc_p  = element::GaussFieldCouplingParameters{ input_dim, true, false,
			{ {25.0, 25.0, 10.0, 5.0},
			  {50.0, 50.0, 10.0, 5.0},
			  {75.0, 75.0, 10.0, 5.0} } };
		const auto gfc    = std::make_shared<element::GaussFieldCoupling>(gfc_cp, gfc_p);

		simulation->addElement(nf_in);
		simulation->addElement(gk_in);
		simulation->addElement(gs_in);
		simulation->addElement(nn_in);
		simulation->addElement(nf_out);
		simulation->addElement(mh_out);
		simulation->addElement(nn_out);
		simulation->addElement(gfc);

		nf_in->addInput(gk_in);
		nf_in->addInput(gs_in);
		nf_in->addInput(nn_in);
		gk_in->addInput(nf_in);

		gfc->addInput(nf_in);

		nf_out->addInput(gfc);
		nf_out->addInput(mh_out);
		nf_out->addInput(nn_out);
		mh_out->addInput(nf_out);

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 200, -20.0, 20.0, 0.5, 1.0 },
				PlotAnnotations{ "Input field", "Spatial dimension", "Amplitude" } },
			LinePlotParameters{},
			{ {nf_in->getUniqueName(), "activation"},
			  {nf_in->getUniqueName(), "output"},
			  {nf_in->getUniqueName(), "input"} });

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 100, -20.0, 20.0, 1.0, 1.0 },
				PlotAnnotations{ "Output field", "Spatial dimension", "Amplitude" } },
			LinePlotParameters{},
			{ {nf_out->getUniqueName(), "activation"},
			  {nf_out->getUniqueName(), "output"},
			  {nf_out->getUniqueName(), "input"} });

		visualization->plot(
			PlotCommonParameters{
				PlotType::HEATMAP,
				PlotDimensions{ 0.0, 100, 0.0, 200.0, 1.0, 0.5 },
				PlotAnnotations{ "Coupling weights", "Output dimension", "Input dimension" } },
			HeatmapParameters{},
			{ {gfc->getUniqueName(), "weights"} });

		visualization->plot(
			PlotCommonParameters{
				PlotType::LINE_PLOT,
				PlotDimensions{ 0.0, 100, -20.0, 20.0, 1.0, 1.0 },
				PlotAnnotations{ "Coupling output", "Spatial dimension", "Amplitude" } },
			LinePlotParameters{},
			{ {gfc->getUniqueName(), "output"} });

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
