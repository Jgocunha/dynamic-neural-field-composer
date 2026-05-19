#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"
#include "elements/timed_gauss_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/neural_field_2d.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation = std::make_shared<Simulation>("Timed stimuli 2d (example)",
            5.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);
        const Application app{ simulation, visualization };

        app.addWindow<user_interface::MainMenuBar>();
        app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

        constexpr int    fieldSizeX = 50;
        constexpr int    fieldSizeY = 50;
        constexpr double dx         = 1.0;
        constexpr double dy         = 1.0;

        const element::ElementDimensions dims{ fieldSizeX, fieldSizeY, dx, dy };

        // Timed 2D stimulus: active during [10, 30] and [60, 80]
        element::TimedGaussStimulus2DParameters tgsp{
            5.0,   // width
            15.0,  // amplitude
            25.0,  // position_x
            25.0,  // position_y
            {{0.0, 300.0}, {600.0, 1000.0}},  // on-time windows
            true,  // circular
            false  // normalized
        };
        const auto tgs = std::make_shared<element::TimedGaussStimulus2D>(
            element::ElementCommonParameters{ "timed stim 2d", dims }, tgsp);

        element::GaussKernel2DParameters gkp{ 3.0, 3.0, -0.01, true, true };
        const auto gk = std::make_shared<element::GaussKernel2D>(
            element::ElementCommonParameters{ "gauss kernel 2d", dims }, gkp);

        element::NeuralField2DParameters nfp{};
        const auto nf = std::make_shared<element::NeuralField2D>(
            element::ElementCommonParameters{ "neural field 2d", dims }, nfp);

        simulation->addElement(tgs);
        simulation->addElement(gk);
        simulation->addElement(nf);

        simulation->createInteraction("timed stim 2d",  "output", "gauss kernel 2d");
        simulation->createInteraction("gauss kernel 2d", "output", "neural field 2d");

        visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
            PlotAnnotations{"Neural field activation", "Spatial dimension (x)", "Spatial dimension (y)"}},
            HeatmapParameters{},
            { {nf->getUniqueName(), "activation"} });
        visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
            PlotAnnotations{"Gauss kernel", "Spatial dimension (x)", "Spatial dimension (y)"}},
            HeatmapParameters{},
            { {gk->getUniqueName(), "kernel"} });
        visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
            PlotAnnotations{"Gauss kernel output", "Spatial dimension (x)", "Spatial dimension (y)"}},
            HeatmapParameters{},
            { {gk->getUniqueName(), "output"} });

        simulation->init();
        app.init();
        while (!app.hasGUIBeenClosed())
            app.step();
        app.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
