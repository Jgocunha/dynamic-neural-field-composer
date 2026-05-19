#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"

int main()
{
	try {
		using namespace dnf_composer;

		const auto simulation = std::make_shared<Simulation>("Travelling bump of activation (example)",
			5.0, 0.0, 0.0);
		const auto visualization = std::make_shared<Visualization>(simulation);

		const element::ElementDimensions dimensions2D(50, 50, 1.0, 1.0);
		const auto tgscp_1 = element::ElementCommonParameters{ "Timed gauss stimulus", dimensions2D};
		const auto tgsp_1 = element::TimedGaussStimulus2DParameters{5, 15, 25, 25, {{0, 1000}}};
		const auto tgs_1 = std::make_shared < element::TimedGaussStimulus2D > (tgscp_1, tgsp_1);

		const auto gkcp_1 = element::ElementCommonParameters{ "Self-excitation asymmetric gauss kernel" , dimensions2D};
		const auto gkp_1 = element::AsymmetricGaussKernel2DParameters{ 6.0, 18.0, -0.01, 1.0, 1.0 };
		const auto gk_1 = std::make_shared < element::AsymmetricGaussKernel2D > ( gkcp_1, gkp_1 );

		const auto nfcp_1 = element::ElementCommonParameters{ "Neural field", dimensions2D};
		const auto nfp_1 = element::NeuralField2DParameters{};
		const auto nf_1 = std::make_shared < element::NeuralField2D > ( nfcp_1, nfp_1 );

		const auto nncp_1 = element::ElementCommonParameters{ "Normal noise", dimensions2D};
		const auto nnp_1 = element::NormalNoise2DParameters{};
		const auto nn_1 = std::make_shared < element::NormalNoise2D > (nncp_1, nnp_1);

		simulation->addElement(tgs_1);
		simulation->addElement(gk_1);
		simulation->addElement(nf_1);
		simulation->addElement(nn_1);

		nf_1->addInput(tgs_1);
		nf_1->addInput(gk_1);
		gk_1->addInput(nf_1);
		nf_1->addInput(nn_1);

		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Neural field activation", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {nf_1->getUniqueName(), "activation"} });
		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Asymmetric gauss kernel", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {gk_1->getUniqueName(), "kernel"} });
		visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
			PlotAnnotations{"Asymmetric gauss kernel output", "Spatial dimension (x)", "Spatial dimension (y)"}},
			HeatmapParameters{},
			{ {gk_1->getUniqueName(), "output"} });


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

