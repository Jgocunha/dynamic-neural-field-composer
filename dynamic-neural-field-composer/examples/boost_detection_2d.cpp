#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"
#include "elements/boost_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/neural_field_2d.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation = std::make_shared<Simulation>("Boost detection 2D (example)",
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

        element::BoostStimulus2DParameters bsp{ 2.0, true };
        const auto bs = std::make_shared<element::BoostStimulus2D>(
            element::ElementCommonParameters{ "boost 2d", dims }, bsp);

        element::TimedGaussStimulus2DParameters tgsp{5, 8, 25, 25, {{0, 500}}};
        const auto tgs = std::make_shared<element::TimedGaussStimulus2D>(
            element::ElementCommonParameters{ "timed stim 2d", dims }, tgsp);

        element::GaussKernel2DParameters gkp{ 3.0, 3.0, -0.01, true, true };
        const auto gk = std::make_shared<element::GaussKernel2D>(
            element::ElementCommonParameters{ "gauss kernel 2d", dims }, gkp);

        element::NeuralField2DParameters nfp{};
        const auto nf = std::make_shared<element::NeuralField2D>(
            element::ElementCommonParameters{ "neural field 2d", dims }, nfp);

        simulation->addElement(bs);
        simulation->addElement(gk);
        simulation->addElement(nf);
        simulation->addElement(tgs);

        simulation->createInteraction("boost 2d",       "output", "neural field 2d");
        simulation->createInteraction("timed stim 2d",       "output", "neural field 2d");
        simulation->createInteraction("gauss kernel 2d","output", "neural field 2d");
        simulation->createInteraction("neural field 2d","output", "gauss kernel 2d");

        visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
            PlotAnnotations{"Neural field activation", "Spatial dimension (x)", "Spatial dimension (y)"}},
            HeatmapParameters{},
            { {nf->getUniqueName(), "activation"} });
        visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
            PlotAnnotations{"Boost stimulus", "Spatial dimension (x)", "Spatial dimension (y)"}},
            HeatmapParameters{},
            { {bs->getUniqueName(), "output"} });
        visualization->plot(PlotCommonParameters{PlotType::HEATMAP,
            PlotAnnotations{"Timed stimulus", "Spatial dimension (x)", "Spatial dimension (y)"}},
            HeatmapParameters{},
            { {tgs->getUniqueName(), "output"} });

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
