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
// DestroyEditor(), so the editor context and its settings-file handle leaked for
// the lifetime of the process.
//
// A leak is not directly observable from inside the test, so this test's job is
// to exercise the construct/destroy cycle repeatedly and let the Linux CI
// asan-ubsan job report the leak. Before the fix that job reports a definite
// leak of the editor context; after it, nothing.
//
// These construct the window but never render it, so no ImGui frame, window, or
// GL context is required.
// ---------------------------------------------------------------------------

TEST(NodeGraphWindowLifetime, ConstructAndDestroyReleasesTheEditorContext)
{
    const auto sim = std::make_shared<Simulation>("node-graph-lifetime", 1.0, 0.0, 0.0);
    {
        const user_interface::NodeGraphWindow window{ sim };
        (void)window;
    }
    SUCCEED();
}

TEST(NodeGraphWindowLifetime, RepeatedConstructAndDestroyDoesNotAccumulate)
{
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
