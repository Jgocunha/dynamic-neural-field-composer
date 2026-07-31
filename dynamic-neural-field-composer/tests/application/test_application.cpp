#include <gtest/gtest.h>
#include <memory>
#include <imgui.h>

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

// ---------------------------------------------------------------------------
// enableKeyboardShortcuts()/appendFonts() must mutate the *real* ImGui IO
// (issue #114) — see .claude/tests/05-gui-headless.md for the project's usual
// "never touch ImGui::*" boundary. This is a deliberate, narrow exception:
// ImGui::CreateContext()/DestroyContext() need no window, OpenGL, or rendering
// backend — we never call NewFrame()/Render(), so this stays fully headless.
// Regression for: `auto io = ImGui::GetIO();` copies the IO struct by value,
// so writes through `io` land on a discarded temporary instead of the real
// global IO that the rest of the app reads.
// ---------------------------------------------------------------------------

namespace {
    struct ScopedImGuiContext
    {
        ImGuiContext* ctx = ImGui::CreateContext();
        ScopedImGuiContext()  { ImGui::SetCurrentContext(ctx); }
        ~ScopedImGuiContext() { ImGui::DestroyContext(ctx); }
    };
}

TEST(ApplicationIOReference, EnableKeyboardShortcutsPersistsNavFlagOnGlobalIO)
{
    ScopedImGuiContext guard;

    ASSERT_FALSE(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)
        << "precondition: flag must start unset, otherwise the check below would "
           "pass vacuously even with the pre-fix bug";

    Application::enableKeyboardShortcuts();

    EXPECT_TRUE(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)
        << "enableKeyboardShortcuts() must set the flag on the REAL global IO, "
           "not on a locally-copied ImGuiIO value";
}

TEST(ApplicationIOReference, AppendFontsPersistsFontDefaultOnGlobalIO)
{
    ScopedImGuiContext guard;

    ImGuiIO& io = ImGui::GetIO();
    for (std::size_t i = 0; i < dnf_composer::g_FontCount; ++i) {
        io.Fonts->AddFontDefault();
    }
    ASSERT_EQ(io.FontDefault, nullptr)
        << "precondition: no default font set yet";

    Application::appendFonts();

    EXPECT_NE(ImGui::GetIO().FontDefault, nullptr)
        << "appendFonts() must set io.FontDefault on the REAL global IO, not on "
           "a locally-copied ImGuiIO value";
    EXPECT_EQ(ImGui::GetIO().FontDefault, dnf_composer::g_MediumMediumFont);
}
