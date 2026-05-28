#include <gtest/gtest.h>
#include <memory>

#include "application/application.h"
#include "simulation/simulation.h"
#include "visualization/visualization.h"
#include "exceptions/exception.h"

using namespace dnf_composer;

// ---------------------------------------------------------------------------
// Static scale accessors — no OpenGL context required
// ---------------------------------------------------------------------------

TEST(ApplicationScale, DefaultScaleIs100)
{
    EXPECT_FLOAT_EQ(Application::getUiScalePct(), 100.0f);
}

TEST(ApplicationScale, SetAndGetRoundTrip)
{
    Application::setUiScalePct(150.0f);
    EXPECT_FLOAT_EQ(Application::getUiScalePct(), 150.0f);
    Application::setUiScalePct(100.0f); // restore
}

// ---------------------------------------------------------------------------
// Construction — mismatched sim/vis detected before GUI initialisation
// ---------------------------------------------------------------------------

TEST(ApplicationConstruction, MismatchedSimulationThrows)
{
    auto sim1 = std::make_shared<Simulation>("s1", 1.0, 0.0, 0.0);
    auto sim2 = std::make_shared<Simulation>("s2", 1.0, 0.0, 0.0);
    auto vis  = std::make_shared<Visualization>(sim1);
    EXPECT_THROW(Application(sim2, vis), Exception);
}
