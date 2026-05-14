#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "elements/field_expansion.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation = std::make_shared<Simulation>("example field expansion", 25.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);
        const Application app{ simulation, visualization };

        app.addWindow<user_interface::MainMenuBar>();
        app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

        constexpr int    fieldSize1D = 50;
        constexpr double dx          = 1.0;
        constexpr int    fieldSizeY  = 50;
        constexpr double dy          = 1.0;

        const element::ElementDimensions dims1D{ fieldSize1D, dx };
        const element::ElementDimensions dims2D{ fieldSize1D, fieldSizeY, dx, dy };

        element::GaussStimulusParameters gsp{ 5.0, 15.0, 25.0, true, false };
        const auto gs = std::make_shared<element::GaussStimulus>(
            element::ElementCommonParameters{ "gauss stimulus", dims1D }, gsp);

        element::GaussKernelParameters gkp{ 3.0, 3.0, -0.01, true, true };
        const auto gk = std::make_shared<element::GaussKernel>(
            element::ElementCommonParameters{ "gauss kernel", dims1D }, gkp);

        element::NeuralFieldParameters nfp{ 25.0, -5.0, element::SigmoidFunction(0.0, 10.0) };
        const auto nf = std::make_shared<element::NeuralField>(
            element::ElementCommonParameters{ "neural field", dims1D }, nfp);

        // Expand the 1D field (varies along X) into a 2D field by repeating along Y.
        element::FieldExpansionParameters fep{ 0 };
        const auto fe = std::make_shared<element::FieldExpansion>(
            element::ElementCommonParameters{ "field expansion", dims2D }, fep);

        const auto nf2d = std::make_shared<element::NeuralField2D>(
            element::ElementCommonParameters{ "neural field 2d", dims2D }, element::NeuralField2DParameters{});

        simulation->addElement(gs);
        simulation->addElement(gk);
        simulation->addElement(nf);
        simulation->addElement(fe);
        simulation->addElement(nf2d);

        simulation->createInteraction("gauss stimulus", "output", "neural field");
        simulation->createInteraction("gauss kernel",   "output", "neural field");
        simulation->createInteraction("neural field",   "output", "gauss kernel");
        simulation->createInteraction("neural field",   "output", "field expansion");
        simulation->createInteraction("field expansion", "output", "neural field 2d");

        visualization->plot({ {gs->getUniqueName(), "output"} });
        visualization->plot({ {nf->getUniqueName(), "output"} });
        visualization->plot({ {fe->getUniqueName(), "output"} });
        visualization->plot({ {nf2d->getUniqueName(), "output"} });

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
