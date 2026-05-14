#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/field_projection.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation = std::make_shared<Simulation>("example field projection", 25.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);
        const Application app{ simulation, visualization };

        app.addWindow<user_interface::MainMenuBar>();
        app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

        constexpr int    fieldSizeX = 50;
        constexpr int    fieldSizeY = 50;
        constexpr double dx         = 1.0;
        constexpr double dy         = 1.0;

        const element::ElementDimensions dims{ fieldSizeX, fieldSizeY, dx, dy };

        element::GaussStimulusParameters2D gsp{ 5.0, 15.0, 25.0, 25.0, true, false };
        const auto gs = std::make_shared<element::GaussStimulus2D>(
            element::ElementCommonParameters{ "gauss stimulus 2d", dims }, gsp);

        element::GaussKernel2DParameters gkp{ 3.0, 3.0, -0.01, true, true };
        const auto gk = std::make_shared<element::GaussKernel2D>(
            element::ElementCommonParameters{ "gauss kernel 2d", dims }, gkp);

        element::NeuralField2DParameters nfp{};
        const auto nf = std::make_shared<element::NeuralField2D>(
            element::ElementCommonParameters{ "neural field 2d", dims }, nfp);

        // Project the 2D field onto the X axis (sum over Y)
        element::FieldProjectionParameters fpp{ 0 };
        const auto fp = std::make_shared<element::FieldProjection>(
            element::ElementCommonParameters{ "field projection", dims }, fpp);

        simulation->addElement(gs);
        simulation->addElement(gk);
        simulation->addElement(nf);
        simulation->addElement(fp);

        simulation->createInteraction("gauss stimulus 2d", "output", "gauss kernel 2d");
        simulation->createInteraction("gauss kernel 2d",   "output", "neural field 2d");
        simulation->createInteraction("neural field 2d",   "output", "field projection");

        visualization->plot({ {gs->getUniqueName(), "output"} });
        visualization->plot({ {nf->getUniqueName(), "output"} });
        visualization->plot({ {fp->getUniqueName(), "output"} });

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
