#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>

#include "user_interface/element_creation_error.h"
#include "exceptions/exception.h"
#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"

using namespace dnf_composer;
using dnf_composer::user_interface::describeElementCreationFailure;

// ---------------------------------------------------------------------------
// Regression tests for issue #146.
//
// The library deliberately fails loudly now: ElementDimensions (#96), the
// Element base constructor (#118) and ElementFactory (#113) all throw on bad
// input. But the element-creation forms in SimulationWindow run inside the
// ImGui render loop, so an exception escaping one of them unwinds straight out
// of the frame and terminates the application -- a user typing a bad size into
// the form could kill the process.
//
// describeElementCreationFailure() is the seam that stops that: it runs the
// construct-and-add step and converts any failure into a message the form can
// display, returning an empty string on success. It is deliberately free of any
// ImGui dependency so it can be tested exactly like this, with no window, no
// context and no render loop.
// ---------------------------------------------------------------------------

TEST(ElementCreationError, SuccessfulCreationReportsNoError)
{
    const auto sim = std::make_shared<Simulation>("creation-ok", 1.0, 0.0, 0.0);

    const std::string error = describeElementCreationFailure([&]
    {
        const element::ElementCommonParameters common{ "nf", 20 };
        const element::NeuralFieldParameters nfp{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } };
        sim->addElement(std::make_shared<element::NeuralField>(common, nfp));
    });

    EXPECT_TRUE(error.empty()) << "unexpected error: " << error;
    EXPECT_EQ(sim->getNumberOfElements(), 1);
}

TEST(ElementCreationError, InvalidSizeIsReportedInsteadOfEscaping)
{
    // The live trigger today: ElementDimensions rejects a non-positive size
    // (#96, already on main). Before this seam existed the throw propagated out
    // of the render loop.
    const auto sim = std::make_shared<Simulation>("creation-bad-size", 1.0, 0.0, 0.0);

    const std::string error = describeElementCreationFailure([&]
    {
        const element::ElementCommonParameters common{ element::ElementIdentifiers{ "nf" },
                                                       element::ElementDimensions{ 0, 1.0 } };
        const element::NeuralFieldParameters nfp{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } };
        sim->addElement(std::make_shared<element::NeuralField>(common, nfp));
    });

    EXPECT_FALSE(error.empty());
    EXPECT_EQ(sim->getNumberOfElements(), 0) << "nothing should have been added";
}

TEST(ElementCreationError, LibraryExceptionMessageIsPreserved)
{
    // The Exception carries the element name and ErrorCode, so the form can show
    // something specific rather than a generic "could not add element".
    const std::string error = describeElementCreationFailure([]
    {
        throw Exception(ErrorCode::ELEM_INVALID_SIZE, std::string("my field"));
    });

    ASSERT_FALSE(error.empty());
    EXPECT_NE(error.find("my field"), std::string::npos)
        << "the element name should survive into the message, got: " << error;
}

TEST(ElementCreationError, NonLibraryExceptionIsAlsoCaught)
{
    // Backstop: std::bad_alloc and anything else a constructor might throw must
    // not escape the render loop either.
    const std::string error = describeElementCreationFailure([]
    {
        throw std::runtime_error("something else went wrong");
    });

    ASSERT_FALSE(error.empty());
    EXPECT_NE(error.find("something else went wrong"), std::string::npos);
}

TEST(ElementCreationError, UnknownExceptionIsCaught)
{
    // Nothing at all may escape; a non-std throw still yields a usable message.
    const std::string error = describeElementCreationFailure([] { throw 42; });

    EXPECT_FALSE(error.empty());
}
