#include "application/application.h"
#include "user_interface/main_window.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/element_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/simulation_window.h"
#include "elements/element_factory.h"
#include "user_interface/plots_window.h"
#include "user_interface/node_graph_window.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation = std::make_shared<Simulation>("example gauss and field couplings", 25.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);
        const Application app{ simulation, visualization };

        app.addWindow<user_interface::MainWindow>();
        app.addWindow<imgui_kit::LogWindow>();
        app.addWindow<user_interface::FieldMetricsWindow>();
        app.addWindow<user_interface::ElementWindow>();
        app.addWindow<user_interface::SimulationWindow>();
        app.addWindow<user_interface::PlotControlWindow>();
        app.addWindow<user_interface::PlotsWindow>();
        app.addWindow<user_interface::NodeGraphWindow>();


        element::ElementFactory factory;
        const element::ElementDimensions input_dimensions{200, 0.7};
        const auto nf_1 = factory.createElement(element::ElementLabel::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, input_dimensions},
            element::NeuralFieldParameters{});
        const auto gk_1 = factory.createElement(element::GAUSS_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_KERNEL}, input_dimensions},
            element::GaussKernelParameters{});
        const auto nn_1 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, input_dimensions},
            element::NormalNoiseParameters{});
        const auto gs_1 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, input_dimensions},
            element::GaussStimulusParameters{});

        const element::ElementDimensions output_dimensions{100, 1.0};
        const auto nf_2 = factory.createElement(element::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, output_dimensions},
            element::NeuralFieldParameters{});
        const auto mhk_2 = factory.createElement(element::MEXICAN_HAT_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::MEXICAN_HAT_KERNEL}, output_dimensions},
            element::MexicanHatKernelParameters{});
        const auto nn_2 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, output_dimensions},
            element::NormalNoiseParameters{});
        const auto gs_2 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, output_dimensions},
            element::GaussStimulusParameters{});
        const auto gfc_1 = factory.createElement(element::GAUSS_FIELD_COUPLING,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_FIELD_COUPLING}, output_dimensions},
            element::GaussFieldCouplingParameters{input_dimensions, true, false, { {50.0, 50.0, 5.0, 5.0},
            																										{25.0, 75.0, 5.0, 5.0},
																													{75.0, 25.0, 5.0, 5.0}}});
        simulation->addElement(nf_1);
        simulation->addElement(gk_1);
        simulation->addElement(nn_1);
        simulation->addElement(nf_2);
        simulation->addElement(mhk_2);
        simulation->addElement(nn_2);
        simulation->addElement(gs_1);
        simulation->addElement(gs_2);
        simulation->addElement(gfc_1);

        nf_1->addInput(gk_1);
        gk_1->addInput(nf_1);
        nf_1->addInput(nn_1);
        nf_1->addInput(gs_1);

        nf_2->addInput(mhk_2);
        mhk_2->addInput(nf_2);
        nf_2->addInput(nn_2);
        nf_2->addInput(gs_2);

        gfc_1->addInput(nf_1);
        nf_2->addInput(gfc_1);

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 200, -20.0, 20, 0.7, 1.0},
            PlotAnnotations{ "Input neural field", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { { nf_1->getUniqueName(), "activation" }, { nf_1->getUniqueName(), "output" }, { nf_1->getUniqueName(), "input" } });

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 100, -20.0, 20, 1.0, 1.0 },
            PlotAnnotations{ "Output neural field", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { {nf_2->getUniqueName(), "activation"}, {nf_2->getUniqueName(), "output"}, {nf_2->getUniqueName(), "input"} });

        visualization->plot(
            PlotCommonParameters{
                PlotType::HEATMAP,
                PlotDimensions{0.0, 100, 0.0, 200.0, 1.0, 0.7},
                PlotAnnotations{"Field coupling", "Output field dimension", "Input field dimension"} },
                HeatmapParameters{},
            { {gfc_1->getUniqueName(), "weights"} }
        );

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 100, -20.0, 20, 1.0, 1.0},
            PlotAnnotations{ "Field coupling output", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { {gfc_1->getUniqueName(), "output"} });


        const element::ElementDimensions input_dimensions_2{150, 0.5};
        const auto nf_1_2 = factory.createElement(element::ElementLabel::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, input_dimensions_2},
            element::NeuralFieldParameters{});
        const auto gk_2_1 = factory.createElement(element::GAUSS_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_KERNEL}, input_dimensions_2},
            element::GaussKernelParameters{});
        const auto nn_2_1 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, input_dimensions_2},
            element::NormalNoiseParameters{});
        const auto gs_2_1 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, input_dimensions_2},
            element::GaussStimulusParameters{});

        const element::ElementDimensions output_dimensions_1{75, 0.5};
        const auto nf_2_2 = factory.createElement(element::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, output_dimensions_1},
            element::NeuralFieldParameters{});
        const auto mhk_2_2 = factory.createElement(element::MEXICAN_HAT_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::MEXICAN_HAT_KERNEL}, output_dimensions_1},
            element::MexicanHatKernelParameters{});
        const auto nn_2_2 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, output_dimensions_1},
            element::NormalNoiseParameters{});
        const auto gs_2_2 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, output_dimensions_1},
            element::GaussStimulusParameters{});
        const auto fc_2 = factory.createElement(element::FIELD_COUPLING,
            			element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::FIELD_COUPLING}, output_dimensions_1},
            			element::FieldCouplingParameters{input_dimensions_2, LearningRule::HEBB, 1.0, 0.01});

        simulation->addElement(nf_1_2);
        simulation->addElement(gk_2_1);
        simulation->addElement(nn_2_1);
        simulation->addElement(nf_2_2);
        simulation->addElement(mhk_2_2);
        simulation->addElement(nn_2_2);
        simulation->addElement(gs_2_1);
        simulation->addElement(gs_2_2);
        simulation->addElement(fc_2);

        nf_1_2->addInput(gk_2_1);
        gk_2_1->addInput(nf_1_2);
        nf_1_2->addInput(nn_2_1);
        nf_1_2->addInput(gs_2_1);

        nf_2_2->addInput(mhk_2_2);
        mhk_2_2->addInput(nf_2_2);
        nf_2_2->addInput(nn_2_2);
        nf_2_2->addInput(gs_2_2);

        fc_2->addInput(nf_1_2);
        nf_2_2->addInput(fc_2);

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 150, -20.0, 20, 0.5, 1.0},
            PlotAnnotations{ "Input neural field", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { { nf_1_2->getUniqueName(), "activation" }, { nf_1_2->getUniqueName(), "output" }, { nf_1_2->getUniqueName(), "input" } });

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 75, -20.0, 20, 0.5, 1.0 },
            PlotAnnotations{ "Output neural field", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { {nf_2_2->getUniqueName(), "activation"}, {nf_2_2->getUniqueName(), "output"}, {nf_2_2->getUniqueName(), "input"} });

        visualization->plot(
            PlotCommonParameters{
                PlotType::HEATMAP,
                PlotDimensions{0.0, 75, 0.0, 150.0,0.5, 0.5},
                PlotAnnotations{"Field coupling", "Output field dimension", "Input field dimension"} },
                HeatmapParameters{},
            { {fc_2->getUniqueName(), "weights"} }
        );

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 75, -20.0, 20, 0.5, 1.0},
            PlotAnnotations{ "Field coupling output", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { {fc_2->getUniqueName(), "output"} });


        const element::ElementDimensions input_dimensions_3{180, 0.6};
        const auto nf_1_3 = factory.createElement(element::ElementLabel::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, input_dimensions_3},
            element::NeuralFieldParameters{});
        const auto gk_3_1 = factory.createElement(element::GAUSS_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_KERNEL}, input_dimensions_3},
            element::GaussKernelParameters{});
        const auto nn_3_1 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, input_dimensions_3},
            element::NormalNoiseParameters{});
        const auto gs_3_1 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, input_dimensions_3},
            element::GaussStimulusParameters{});

        const element::ElementDimensions output_dimensions_3{90, 0.6};
        const auto nf_2_3 = factory.createElement(element::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, output_dimensions_3},
            element::NeuralFieldParameters{});
        const auto mhk_3_2 = factory.createElement(element::MEXICAN_HAT_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::MEXICAN_HAT_KERNEL}, output_dimensions_3},
            element::MexicanHatKernelParameters{});
        const auto nn_3_2 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, output_dimensions_3},
            element::NormalNoiseParameters{});
        const auto gs_3_2 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, output_dimensions_3},
            element::GaussStimulusParameters{});
        const auto gfc_3 = factory.createElement(element::GAUSS_FIELD_COUPLING,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_FIELD_COUPLING}, output_dimensions_3},
            element::GaussFieldCouplingParameters{input_dimensions_3, true, false, { {60, 40, 5.0, 5.0},
            	{40, 60, 5.0, 5.0} ,
            	{20, 80, 5.0, 5.0} ,
            	{80, 20, 5.0, 5.0}
            }});

        simulation->addElement(nf_1_3);
        simulation->addElement(gk_3_1);
        simulation->addElement(nn_3_1);
        simulation->addElement(nf_2_3);
        simulation->addElement(mhk_3_2);
        simulation->addElement(nn_3_2);
        simulation->addElement(gs_3_1);
        simulation->addElement(gs_3_2);
        simulation->addElement(gfc_3);

        nf_1_3->addInput(gk_3_1);
        gk_3_1->addInput(nf_1_3);
        nf_1_3->addInput(nn_3_1);
        nf_1_3->addInput(gs_3_1);

        nf_2_3->addInput(mhk_3_2);
        mhk_3_2->addInput(nf_2_3);
        nf_2_3->addInput(nn_3_2);
        nf_2_3->addInput(gs_3_2);

        gfc_3->addInput(nf_1_3);
        nf_2_3->addInput(gfc_3);

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 180, -20.0, 20, 0.6, 1.0},
            PlotAnnotations{ "Input neural field", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { { nf_1_3->getUniqueName(), "activation" }, { nf_1_3->getUniqueName(), "output" }, { nf_1_3->getUniqueName(), "input" } });

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 90, -20.0, 20, 0.6, 1.0 },
            PlotAnnotations{ "Output neural field", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { {nf_2_3->getUniqueName(), "activation"}, {nf_2_3->getUniqueName(), "output"}, {nf_2_3->getUniqueName(), "input"} });

        visualization->plot(
            PlotCommonParameters{
                PlotType::HEATMAP,
                PlotDimensions{0.0, 90, 0.0, 180.0,0.6, 0.6},
                PlotAnnotations{"Field coupling", "Output field dimension", "Input field dimension"} },
                HeatmapParameters{},
            { {gfc_3->getUniqueName(), "weights"} }
        );

        visualization->plot(
            PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 90, -20.0, 20, 0.6, 1.0},
            PlotAnnotations{ "Field coupling output", "Spatial dimension", "Amplitude" } },
            LinePlotParameters{},
            { {gfc_3->getUniqueName(), "output"} });

        // Fourth Coupled Neural Field
        const element::ElementDimensions input_dimensions_4{170, 0.8};
        const auto nf_1_4 = factory.createElement(element::ElementLabel::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, input_dimensions_4},
            element::NeuralFieldParameters{});
        const auto gk_4_1 = factory.createElement(element::GAUSS_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_KERNEL}, input_dimensions_4},
            element::GaussKernelParameters{});
        const auto nn_4_1 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, input_dimensions_4},
            element::NormalNoiseParameters{});
        const auto gs_4_1 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, input_dimensions_4},
            element::GaussStimulusParameters{});

        const element::ElementDimensions output_dimensions_4{80, 0.8};
        const auto nf_2_4 = factory.createElement(element::NEURAL_FIELD,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NEURAL_FIELD}, output_dimensions_4},
            element::NeuralFieldParameters{});
        const auto mhk_4_2 = factory.createElement(element::MEXICAN_HAT_KERNEL,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::MEXICAN_HAT_KERNEL}, output_dimensions_4},
            element::MexicanHatKernelParameters{});
        const auto nn_4_2 = factory.createElement(element::NORMAL_NOISE,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::NORMAL_NOISE}, output_dimensions_4},
            element::NormalNoiseParameters{});
        const auto gs_4_2 = factory.createElement(element::GAUSS_STIMULUS,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::GAUSS_STIMULUS}, output_dimensions_4},
            element::GaussStimulusParameters{});
        const auto fc_4 = factory.createElement(element::FIELD_COUPLING,
            element::ElementCommonParameters{element::ElementIdentifiers{element::ElementLabel::FIELD_COUPLING}, output_dimensions_4},
            element::FieldCouplingParameters{input_dimensions_4, LearningRule::OJA, 1.5, 0.015});

        simulation->addElement(nf_1_4);
        simulation->addElement(gk_4_1);
        simulation->addElement(nn_4_1);
        simulation->addElement(nf_2_4);
        simulation->addElement(mhk_4_2);
        simulation->addElement(nn_4_2);
        simulation->addElement(gs_4_1);
		simulation->addElement(gs_4_2);
        simulation->addElement(fc_4);

        nf_1_4->addInput(gk_4_1);
        gk_4_1->addInput(nf_1_4);
        nf_1_4->addInput(nn_4_1);
        nf_1_4->addInput(gs_4_1);

        nf_2_4->addInput(mhk_4_2);
        mhk_4_2->addInput(nf_2_4);
        nf_2_4->addInput(nn_4_2);
        nf_2_4->addInput(gs_4_2);

        fc_4->addInput(nf_1_4);
        nf_2_4->addInput(fc_4);

        visualization->plot(
			PlotCommonParameters{
						PlotType::LINE_PLOT,
						PlotDimensions{ 0.0, 170, -20.0, 20, 0.8, 1.0},
						PlotAnnotations{ "Input neural field", "Spatial dimension", "Amplitude" } },
			LinePlotParameters{},
			{ { nf_1_4->getUniqueName(), "activation" }, { nf_1_4->getUniqueName(), "output" }, { nf_1_4->getUniqueName(), "input" } });

        visualization->plot(
            PlotCommonParameters{
                PlotType::LINE_PLOT,
                PlotDimensions{ 0.0, 80, -20.0, 20, 0.8, 1.0 },
                PlotAnnotations{ "Output neural field", "Spatial dimension", "Amplitude" } },
                LinePlotParameters{},
                { {nf_2_4->getUniqueName(), "activation"}, {nf_2_4->getUniqueName(), "output"}, {nf_2_4->getUniqueName(), "input"} });

        visualization->plot(
			PlotCommonParameters{
							PlotType::HEATMAP,
							PlotDimensions{0.0, 80, 0.0, 170.0,0.8, 0.8},
							PlotAnnotations{"Field coupling", "Output field dimension", "Input field dimension"} },
				HeatmapParameters{},
			{ {fc_4->getUniqueName(), "weights"} }
		);

        visualization->plot(
			PlotCommonParameters{
							PlotType::LINE_PLOT,
							PlotDimensions{ 0.0, 80, -20.0, 20, 0.8, 1.0},
							PlotAnnotations{ "Field coupling output", "Spatial dimension", "Amplitude" } },
				LinePlotParameters{},
				{ {fc_4->getUniqueName(), "output"} });

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
