#include <gtest/gtest.h>
#include <memory>
#include <exception>
#include <filesystem>
#include <imgui.h>

#include "application/application.h"
#include "simulation/simulation.h"
#include "visualization/visualization.h"
#include "exceptions/exception.h"
#include "user_interface/main_menu_bar.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"

using namespace dnf_composer;

namespace fs = std::filesystem;

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
// Regression tests for issue #122: the Quit menu item and the Ctrl+Q shortcut
// (user_interface::MainMenuBar) used to call std::exit(0) directly from inside
// the render callback, terminating the process without unwinding the stack --
// no destructors, no Application::close(). The fix replaces that with
// Application::requestQuit(), which hasGUIBeenClosed() reports, plus a pure
// decision function (decideQuitAction) that is trivially testable.
//
// The request lives on Application (not on the menu bar) so that every existing
// main loop -- `while (!app.hasGUIBeenClosed())` -- exits on Quit without any
// source change. MainMenuBar needs no live ImGui/window context for any of the
// following, so these sit alongside Application's other headless-safe tests.
// ---------------------------------------------------------------------------

using dnf_composer::user_interface::MainMenuBar;
using dnf_composer::user_interface::QuitAction;
using dnf_composer::user_interface::decideQuitAction;

TEST(DecideQuitAction, NeitherTriggerProducesNone)
{
    EXPECT_EQ(decideQuitAction(false, false), QuitAction::None);
}

TEST(DecideQuitAction, MenuItemAloneProducesSaveAndQuit)
{
    EXPECT_EQ(decideQuitAction(true, false), QuitAction::SaveAndQuit);
}

TEST(DecideQuitAction, CtrlQAloneProducesQuitOnly)
{
    EXPECT_EQ(decideQuitAction(false, true), QuitAction::QuitOnly);
}

TEST(DecideQuitAction, BothTriggersPreferSaveAndQuit)
{
    // If both somehow fire in the same frame, the safer (save-first) behavior wins.
    EXPECT_EQ(decideQuitAction(true, true), QuitAction::SaveAndQuit);
}

namespace
{
    std::shared_ptr<Simulation> makeSimulationWithOneField(const std::string& identifier)
    {
        auto sim = std::make_shared<Simulation>(identifier, 1.0, 0.0, 0.0);
        const element::ElementCommonParameters cp{ "nf", 20 };
        const element::NeuralFieldParameters nfp{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } };
        sim->addElement(std::make_shared<element::NeuralField>(cp, nfp));
        return sim;
    }
}

class MainMenuBarQuitTest : public ::testing::Test
{
protected:
    void SetUp() override { Application::resetQuitRequestForTesting(); }
    // The request is process-wide (static); always leave it clean for later tests.
    void TearDown() override { Application::resetQuitRequestForTesting(); }
};

TEST_F(MainMenuBarQuitTest, StartsWithNoQuitRequested)
{
    EXPECT_FALSE(Application::isQuitRequested());
}

TEST_F(MainMenuBarQuitTest, RequestQuitEndsAnUnmodifiedMainLoop)
{
    // The backwards-compatibility guarantee: a main loop written before this fix
    // -- `while (!app.hasGUIBeenClosed())` -- must still terminate when the user
    // picks Quit.
    const auto sim = makeSimulationWithOneField("quit-main-loop");
    const auto vis = std::make_shared<Visualization>(sim);

    // Constructing an Application builds the platform GUI object, which on the
    // GLFW backends throws outright when there is no display (headless Linux CI).
    // Where that is the case there is no main loop to test, so skip rather than
    // report a failure that says nothing about this fix.
    std::unique_ptr<Application> app;
    try
    {
        app = std::make_unique<Application>(sim, vis);
    }
    catch (const std::exception& e)
    {
        GTEST_SKIP() << "no GUI available in this environment: " << e.what();
    }

    EXPECT_FALSE(Application::isQuitRequested());

    Application::requestQuit();

    // hasGUIBeenClosed() answers the quit request before touching the GUI, so this
    // is safe on an Application that was never init()'d -- the GLFW backend's
    // isShutdownRequested() reads a window that does not exist until then. This
    // line is exactly what an unmodified main loop evaluates.
    EXPECT_TRUE(app->hasGUIBeenClosed());
}

TEST_F(MainMenuBarQuitTest, NoneActionDoesNotRequestQuitOrTouchSimulation)
{
    const auto sim = makeSimulationWithOneField("menu-none");
    MainMenuBar menuBar{ sim };

    menuBar.executeQuitAction(QuitAction::None);

    EXPECT_FALSE(Application::isQuitRequested());
    EXPECT_EQ(sim->getElements().size(), 1u);
}

TEST_F(MainMenuBarQuitTest, QuitOnlyRequestsQuitWithoutTouchingSimulation)
{
    // Regression for the Ctrl+Q half of issue #122: previously this path called
    // std::exit(0) immediately, with no save/close/clean (an existing asymmetry
    // versus the Quit menu item that this fix intentionally preserves). It must
    // now only flip the quit-request flag -- the simulation is left exactly as
    // it was, and, crucially, this line finishing at all (instead of the test
    // binary vanishing) proves std::exit was NOT called.
    const auto sim = makeSimulationWithOneField("menu-ctrlq");

    MainMenuBar menuBar{ sim };
    menuBar.executeQuitAction(QuitAction::QuitOnly);

    EXPECT_TRUE(Application::isQuitRequested());
    EXPECT_EQ(sim->getElements().size(), 1u); // untouched: no close()/clean() ran
}

TEST_F(MainMenuBarQuitTest, SaveAndQuitRequestsQuitAndClosesAndCleansSimulation)
{
    // Regression for the Quit-menu-item half of issue #122: the save/close/clean
    // sequence must be identical to before, but a quit request replaces the
    // std::exit(0) call.
    const auto sim = makeSimulationWithOneField("menu-quit-save");

    MainMenuBar menuBar{ sim };
    menuBar.executeQuitAction(QuitAction::SaveAndQuit);

    EXPECT_TRUE(Application::isQuitRequested());
    EXPECT_TRUE(sim->getElements().empty());   // clean() cleared the elements
    EXPECT_FALSE(sim->isInitialized());        // close() + clean() both clear this

    const std::string savedFile = std::string(OUTPUT_DIRECTORY) + "/menu-quit-save/menu-quit-save.dnf";
    EXPECT_TRUE(fs::exists(savedFile)) << "save() should still have written the .dnf file, same as before this fix";
    fs::remove_all(std::string(OUTPUT_DIRECTORY) + "/menu-quit-save");
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
