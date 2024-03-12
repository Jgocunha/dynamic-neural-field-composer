#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "elements/gauss_stimulus.h"

void createGaussStimulusWrapper(const std::string& id, const double& size, const dnf_composer::element::GaussStimulusParameters& gsp) 
{
    dnf_composer::element::GaussStimulus stimulus({id, size}, gsp);
}

TEST_CASE("GaussStimulus class tests", "[GaussStimulus]")
{
    // Create a GaussStimulus object for testing
    std::string id = "testGaussStimulus";
    int size = 3;
    double sigma = 1.0;
    double amplitude = 2.0;
    double position = 0.5;
    dnf_composer::element::GaussStimulusParameters gsp{sigma, amplitude, position};

    SECTION("GaussStimulus constructor getParameters() method")
    {
        dnf_composer::element::GaussStimulus gaussStimulus({ id, size }, gsp);

        REQUIRE(gaussStimulus.getLabel() == dnf_composer::element::ElementLabel::GAUSS_STIMULUS);
        REQUIRE(gaussStimulus.getUniqueName() == id);
        REQUIRE(gaussStimulus.getSize() == size);
        REQUIRE(gaussStimulus.getComponent("output").size() == size);

        REQUIRE(gaussStimulus.getParameters() == gsp);

        // invalid position
        gsp.position = position + size;
        REQUIRE_THROWS_AS(createGaussStimulusWrapper(id, size, gsp), dnf_composer::Exception);
        //The C++ standard doesn't allow direct calls to constructors to create objects in regular code flow. 
        //REQUIRE_THROWS_AS(dnf_composer::element::GaussStimulus::GaussStimulus({ id, size }, gsp), dnf_composer::Exception);
    }

    SECTION("GaussStimulus constructor with invalid size")
    {
        int invalidSize = 0;  // Choose a size that triggers the exception
        REQUIRE_THROWS_AS(createGaussStimulusWrapper(id, size, gsp), dnf_composer::Exception);
        //REQUIRE_THROWS_AS(dnf_composer::element::GaussStimulus({ id, invalidSize }, gsp), dnf_composer::Exception);
    }

    SECTION("init() method")
    {
        dnf_composer::element::GaussStimulus gaussStimulus({ id, size }, gsp);

        // Call init() method
        gaussStimulus.init();

        // Check if components["output"] has correct values
        REQUIRE(gaussStimulus.getComponent("output")[0] == Catch::Approx(amplitude * dnf_composer::tools::math::circularGauss(size, sigma, position)[0]));
        REQUIRE(gaussStimulus.getComponent("output")[1] == Catch::Approx(amplitude * dnf_composer::tools::math::circularGauss(size, sigma, position)[1]));
        REQUIRE(gaussStimulus.getComponent("output")[2] == Catch::Approx(amplitude * dnf_composer::tools::math::circularGauss(size, sigma, position)[2]));
    }

    SECTION("setParameters() method")
    {
        dnf_composer::element::GaussStimulus gaussStimulus({ id, size }, gsp);
        double newSigma = 1.1;
        double newAmplitude = 3.0;
        double newPosition = 1.5;
	    dnf_composer::element::GaussStimulusParameters newgsp{ newSigma, newAmplitude, newPosition};

        gaussStimulus.setParameters(newgsp);

        // Check if parameters were updated and init() was called
        REQUIRE(gaussStimulus.getParameters() == newgsp);
        REQUIRE(gaussStimulus.getComponent("output")[0] == Catch::Approx(newAmplitude * dnf_composer::tools::math::circularGauss(size, newSigma, newPosition)[0]));
        REQUIRE(gaussStimulus.getComponent("output")[1] == Catch::Approx(newAmplitude * dnf_composer::tools::math::circularGauss(size, newSigma, newPosition)[1]));
        REQUIRE(gaussStimulus.getComponent("output")[2] == Catch::Approx(newAmplitude * dnf_composer::tools::math::circularGauss(size, newSigma, newPosition)[2]));
    }

    SECTION("init() step() close() methods")
    {
        dnf_composer::element::GaussStimulus gaussStimulus({ id, size }, gsp);
        gaussStimulus.init();
        gaussStimulus.step(1, 1);
        gaussStimulus.close();
    }
}