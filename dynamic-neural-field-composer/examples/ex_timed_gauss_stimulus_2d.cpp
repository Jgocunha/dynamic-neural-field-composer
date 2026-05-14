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

        const auto simulation = std::make_shared<Simulation>("example timed gauss stimulus 2d", 25.0, 0.0, 0.0);
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
            {{10.0, 30.0}, {60.0, 80.0}},  // on-time windows
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

        simulation->createInteraction("timed stim 2d",  "output", "gauss kernel 2d", "input");
        simulation->createInteraction("gauss kernel 2d", "output", "neural field 2d", "input");

        visualization->addPlottingData("timed stim 2d",  "output");
        visualization->addPlottingData("gauss kernel 2d","output");
        visualization->addPlottingData("neural field 2d","output");

        simulation->init();
        app.init();
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
