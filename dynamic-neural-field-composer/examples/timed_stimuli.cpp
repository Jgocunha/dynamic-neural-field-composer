#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"
#include "elements/timed_gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/neural_field.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation = std::make_shared<Simulation>("Timed stimuli (example)",
            5.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);
        const Application app{ simulation, visualization };

        app.addWindow<user_interface::MainMenuBar>();
        app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

        constexpr int    fieldSize = 100;
        constexpr double dx        = 1.0;

        const element::ElementDimensions dims{ fieldSize, dx };

        // Timed stimulus: active during [10, 30] and [60, 80] (simulation time in ms)
        element::TimedGaussStimulusParameters tgsp{
            5.0,   // width
            15.0,  // amplitude
            50.0,  // position
            {{0.0, 300.0}, {600.0, 1000.0}},  // on-time windows
            true,  // circular
            false  // normalized
        };
        const auto tgs = std::make_shared<element::TimedGaussStimulus>(
            element::ElementCommonParameters{ "timed stim", dims }, tgsp);

        element::GaussKernelParameters gkp{ 5.0, 12.0, -0.1, true, true };
        const auto gk = std::make_shared<element::GaussKernel>(
            element::ElementCommonParameters{ "gauss kernel", dims }, gkp);

        element::NeuralFieldParameters nfp{};
        const auto nf = std::make_shared<element::NeuralField>(
            element::ElementCommonParameters{ "neural field", dims }, nfp);

        simulation->addElement(tgs);
        simulation->addElement(gk);
        simulation->addElement(nf);

        simulation->createInteraction("timed stim",  "output", "gauss kernel");
        simulation->createInteraction("gauss kernel", "output", "neural field");

        visualization->plot({ {tgs->getUniqueName(), "output"} });
        visualization->plot({ {gk->getUniqueName(), "output"} });
        visualization->plot({ {nf->getUniqueName(), "activation"} });

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
