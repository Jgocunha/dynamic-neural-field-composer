#include <catch2/catch_test_macros.hpp>

#include "elements/field_coupling.h"

using namespace dnf_composer::element;

TEST_CASE("FieldCoupling - Initialization and Basic Properties")
{
    // Initialize FieldCoupling with valid parameters
    FieldCouplingParameters params{5,2.0,0.1,dnf_composer::LearningRule::HEBBIAN};

    REQUIRE_NOTHROW(FieldCoupling({ "field_coupling_1", 10 }, params));


    SECTION("Invalid Input Field Size Throws Exception")
    {
        // Initialize FieldCoupling with invalid input field size (<= 0)
        params.inputFieldSize = 0;
        REQUIRE_THROWS_AS(FieldCoupling({ "field_coupling_3", 10 }, params), dnf_composer::Exception);
    }
}

TEST_CASE("FieldCoupling - Computation and Output Scaling")
{
    // Initialize FieldCoupling with valid parameters
    const FieldCouplingParameters params{3, 1.5, 0.2, dnf_composer::LearningRule::DELTA_WIDROW_HOFF};

    FieldCoupling fieldCoupling({ "field_coupling_4", 5 }, params);

    fieldCoupling.step(1, 0.1);
}

TEST_CASE("FieldCoupling - File I/O Operations")
{
    // Initialize FieldCoupling with valid parameters
    FieldCouplingParameters params{2, 1.0, 0.5, dnf_composer::LearningRule::DELTA_KROGH_HERTZ};

    FieldCoupling fieldCoupling({ "field_coupling_5", 3 }, params);

    SECTION("Read and Write Weights to File")
    {
        // Save weights to file
        REQUIRE_NOTHROW(fieldCoupling.saveWeights());

        // Read weights from file
        REQUIRE(fieldCoupling.readWeights());

        // Check if weights are successfully read
        REQUIRE(fieldCoupling.getWeights().size() == 2);
        REQUIRE(fieldCoupling.getWeights()[0].size() == 3);
        REQUIRE(fieldCoupling.getWeights()[1].size() == 3);
    }

    SECTION("Invalid File Path Throws Exception")
    {
        // Set an invalid file path
        fieldCoupling.setWeightsFilePath("invalid/path");

        // Attempt to read weights from an invalid file path
        REQUIRE_FALSE(fieldCoupling.readWeights());
    }
}
