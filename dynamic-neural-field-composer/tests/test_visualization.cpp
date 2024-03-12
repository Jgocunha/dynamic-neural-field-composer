#include <catch2/catch_test_macros.hpp>

#include "simulation/visualization.h"
#include "elements/gauss_stimulus.h"

// Helper function to create a sample Element object for testing
std::shared_ptr<dnf_composer::element::GaussStimulus> createGaussStimulus(const std::string& elementId)
{
	dnf_composer::element::GaussStimulusParameters gsp { 1, 2, 3 };
    return std::make_shared<dnf_composer::element::GaussStimulus>(dnf_composer::element::ElementCommonParameters{ elementId, 10 }, gsp);
}


TEST_CASE("Visualization class tests", "[visualization]")
{
    
    SECTION("Visualization constructor throws exception when given null Simulation pointer")
    {
        REQUIRE_THROWS_AS(dnf_composer::Visualization(nullptr), dnf_composer::Exception);
    }

    SECTION("Visualization setSimulation method throws exception when given null Simulation pointer")
    {
	    dnf_composer::Visualization vis(std::make_shared<dnf_composer::Simulation>());
        REQUIRE_THROWS_AS(vis.setSimulation(nullptr), dnf_composer::Exception);
    }

    SECTION("Visualization addPlottingData method throws exception when data is not found")
    {
	    dnf_composer::Visualization vis(std::make_shared<dnf_composer::Simulation>());
        REQUIRE_THROWS_AS(vis.addPlottingData("invalid_element", "invalid_component"), dnf_composer::Exception);
    }

    SECTION("Visualization addPlottingData method doesn't add data when it's already in the vector")
    {
        auto sim = std::make_shared<dnf_composer::Simulation>();
        std::shared_ptr<dnf_composer::element::GaussStimulus> gaussStimulus = createGaussStimulus("element1");
        sim->addElement(gaussStimulus);

        dnf_composer::Visualization vis(sim);
        vis.addPlottingData("element1", "output");

        REQUIRE(vis.getNumberOfPlots() == 1);

        // Add the same data again
        vis.addPlottingData("element1", "output");

        REQUIRE(vis.getNumberOfPlots() == 1);
    }

    SECTION("Visualization getPlottingLabel method throws exception when given invalid index")
    {
        auto sim = std::make_shared<dnf_composer::Simulation>();
        std::shared_ptr<dnf_composer::element::GaussStimulus> gaussStimulus = createGaussStimulus("element1");
        sim->addElement(gaussStimulus);

        dnf_composer::Visualization vis(sim);
        vis.addPlottingData("element1", "output");

        REQUIRE_THROWS_AS(vis.getPlottingLabel(-1), dnf_composer::Exception);
        REQUIRE_THROWS_AS(vis.getPlottingLabel(1), dnf_composer::Exception);
    }

    SECTION("Visualization getPlottingData method throws exception when given invalid index")
    {
        auto sim = std::make_shared<dnf_composer::Simulation>();
        std::shared_ptr<dnf_composer::element::GaussStimulus> gaussStimulus = createGaussStimulus("element1");
        sim->addElement(gaussStimulus);

        dnf_composer::Visualization vis(sim);
        vis.addPlottingData("element1", "output");

        REQUIRE_THROWS_AS(vis.getPlottingData(-1), dnf_composer::Exception);
        REQUIRE_THROWS_AS(vis.getPlottingData(1), dnf_composer::Exception);
    }

    SECTION("Visualization getNumberOfPlots method returns the correct number of plots")
    {
        auto sim = std::make_shared<dnf_composer::Simulation>();
        std::shared_ptr<dnf_composer::element::GaussStimulus> gaussStimulus1 = createGaussStimulus("element1");
        std::shared_ptr<dnf_composer::element::GaussStimulus> gaussStimulus2 = createGaussStimulus("element2");
        sim->addElement(gaussStimulus1);
        sim->addElement(gaussStimulus2);

        dnf_composer::Visualization vis(sim);
        vis.addPlottingData("element1", "output");
        vis.addPlottingData("element2", "output");

        REQUIRE(vis.getNumberOfPlots() == 2);
    }
}