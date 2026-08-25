#include <gtest/gtest.h>
#include <memory>

#include "user_interface/node_graph_window.h"
#include "simulation/simulation.h"

using namespace dnf_composer;

// ---------------------------------------------------------------------------
// Regression test for issue #115.
//
// NodeGraphWindow's constructor calls ImNodeEditor::CreateEditor(&config), but
// the destructor was `= default` and nothing in the codebase ever called
// DestroyEditor(), so the editor context leaked for the lifetime of the process.
// What leaks is memory, not a file handle: EditorContext::~EditorContext is what
// deletes the context's node/pin/link objects and frees its splitter memory, so
// skipping it leaks the whole graph. (config.SettingsFile is only a path string;
// the library opens a local ifstream/ofstream inside Load/SaveSettings and closes
// it there, and SaveSettings also runs during normal operation, so settings
// persistence was never affected -- verified in imgui-node-editor v0.9.3.)
//
// What these tests do NOT do: prove the leak is gone. The CI asan-ubsan job runs
// with ASAN_OPTIONS=detect_leaks=0 (see .github/workflows/ci.yml -- disabled
// because connected elements form a shared_ptr ownership cycle that LeakSanitizer
// flags on every connection test), so no job in this repo can currently observe a
// leak. Confirmed empirically: this file was pushed before the fix and asan-ubsan
// passed. Re-enabling leak detection is tracked separately; only then do these
// become true regression tests for the leak itself.
//
// What they DO cover: the construct/destroy cycle is now RAII-managed, and member
// destruction order matters -- `config` must outlive `context`, because
// CreateEditor() retains the pointer it is handed and DestroyEditor() reads it
// back. Get that ordering wrong and this is a use-after-free, which ASan reports
// regardless of detect_leaks. That is the hazard these tests actually guard.
//
// These construct the window but never render it, so no ImGui frame, window, or
// GL context is required.
// ---------------------------------------------------------------------------

TEST(NodeGraphWindowLifetime, ConstructAndDestroyReleasesTheEditorContext)
{
    // Without this the node editor writes imnode-window.json into whatever directory
    // the test binary was launched from.
    user_interface::NodeGraphWindow::disableLayoutPersistenceForTesting();
    const auto sim = std::make_shared<Simulation>("node-graph-lifetime", 1.0, 0.0, 0.0);
    {
        const user_interface::NodeGraphWindow window{ sim };
        (void)window;
    }
    SUCCEED();
}

TEST(NodeGraphWindowLifetime, RepeatedConstructAndDestroyDoesNotAccumulate)
{
    // Set here as well as in the test above: gtest gives no ordering guarantee, and
    // either test may be the one run in isolation under --gtest_filter.
    user_interface::NodeGraphWindow::disableLayoutPersistenceForTesting();
    // Every File->Open builds a fresh set of windows, so this cycle is the real
    // usage pattern -- the leak grew once per reopened simulation.
    const auto sim = std::make_shared<Simulation>("node-graph-lifetime-loop", 1.0, 0.0, 0.0);
    for (int i = 0; i < 10; ++i)
    {
        const user_interface::NodeGraphWindow window{ sim };
        (void)window;
    }
    SUCCEED();
}
