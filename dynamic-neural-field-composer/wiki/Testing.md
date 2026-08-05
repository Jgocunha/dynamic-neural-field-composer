# Testing

The project uses **Google Test (GTest)** for unit testing. The test suite covers all major library modules and is integrated into the CMake build via `gtest_discover_tests`, so tests are automatically registered with CTest.

---

## Setup

Google Test is installed automatically via vcpkg. No manual setup is needed — just build the project normally.

Tests are compiled into a single executable: `dnf_composer_tests`.

The `DNF_COMPOSER_BUILD_TESTS` CMake option controls whether tests are built (default: `ON`):

```bash
# Explicitly enable (already the default)
cmake -B build -DDNF_COMPOSER_BUILD_TESTS=ON

# Disable if you want a build without tests
cmake -B build -DDNF_COMPOSER_BUILD_TESTS=OFF
```

---

## Running tests

### Via CTest

CTest is the standard way to run tests in a CMake project. From your build directory:

```bash
# Run all tests (Release build)
ctest --build-config Release --output-on-failure

# Run all tests (Debug build)
ctest --build-config Debug --output-on-failure

# Verbose output
ctest --build-config Release -V

# Run in parallel (faster on multi-core machines)
ctest --build-config Release -j4
```

### Via the test executable directly

Running the executable directly gives you full Google Test output and filter support:

```bash
# Windows (Release)
build\x64-release\dnf_composer_tests.exe

# Windows (Debug)
build\x64-debug\dnf_composer_tests.exe

# Linux
./build/dnf_composer_tests

# macOS
./build/macos-release/dnf_composer_tests
```

### Filtering tests

Google Test supports `--gtest_filter` for running a subset of tests:

```bash
# Run all tests in a fixture group
dnf_composer_tests.exe --gtest_filter="NeuralFieldConstruction.*"

# Run a single named test
dnf_composer_tests.exe --gtest_filter="SimulationLifecycle.StepAdvancesTime"

# Run multiple groups
dnf_composer_tests.exe --gtest_filter="NeuralField*:Simulation*"

# Exclude a group
dnf_composer_tests.exe --gtest_filter="-MathUtils*"

# List all registered test names without running them
dnf_composer_tests.exe --gtest_list_tests
```

---

## Writing new tests

All test files are in the `tests/` directory. The project uses the standard Google Test fixture and macro pattern:

```cpp
#include <gtest/gtest.h>
#include "elements/neural_field.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// Standalone test
TEST(MyFixture, DoesWhatItShouldDo)
{
    const auto field = std::make_shared<NeuralField>(...);
    field->init();
    EXPECT_EQ(field->getLabel(), ElementLabel::NEURAL_FIELD);
    EXPECT_NO_THROW(field->getComponent("activation"));
}

// Test with shared setup via fixture class
class NeuralFieldFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ElementCommonParameters cp{ "test field", 100 };
        NeuralFieldParameters nfp{ 25.0, -5.0, SigmoidFunction(0.0, 10.0) };
        field = std::make_shared<NeuralField>(cp, nfp);
        field->init();
    }

    std::shared_ptr<NeuralField> field;
};

TEST_F(NeuralFieldFixture, ActivationStartsAtRestingLevel)
{
    const auto activation = field->getComponent("activation");
    for (const double v : activation)
        EXPECT_NEAR(v, -5.0, 1e-9);
}
```

To add a new test file:

1. Create `tests/test_my_module.cpp`
2. Add it to the `add_executable(dnf_composer_tests ...)` list in `tests/CMakeLists.txt`
3. Rebuild — `gtest_discover_tests` will pick it up automatically

**Exception — auto-discovered directories.** Files dropped into `tests/golden/`,
`tests/user_interface/` and `tests/visualization/` do **not** need a `CMakeLists.txt`
edit: those directories are globbed with `CONFIGURE_DEPENDS`, so a new `.cpp` is picked
up on the next build. (This keeps contributors working in parallel from all editing the
same list.) You still need to re-run `cmake` for the glob to be re-evaluated.

---

## Headless UI tests

`user_interface/` and `visualization/` code is exercised *for real* — the tests call the
actual `render()` methods, with no window and no OpenGL context — via the harness in
`tests/user_interface/ui_test_harness.h`:

```cpp
#include "ui_test_harness.h"
#include "user_interface/log_window.h"

TEST(LogWindowTest, RendersWithoutAWindow)
{
    test::HeadlessImGui gui;                   // headless ImGui context (RAII)
    user_interface::LogWindow window;
    gui.frame([&] { window.render(); });       // one real render call

    gui.frames(2, [&] { window.render(); });   // or several — catches state
                                               // that only appears after frame 1
}
```

Notes:

- `HeadlessImGui` sets `io.DisplaySize`, enables
  `ImGuiBackendFlags_RendererHasTextures`, and assigns the font globals — ImGui
  asserts without these. It creates and destroys the context itself, so just declare
  one per test.
- Anything drawing ImGui commands must run **inside** a frame.
- ImGui uses 16-bit vertex indices, so keep test fields small (a 20×20 2D field can
  exceed the 64k index limit when rendered as a heatmap; 10×10 is safe).

---

## Global state in tests

A few pieces of process-wide state outlive an individual test, so a test that changes
one and does not put it back changes the behaviour of every test that runs after it —
which makes suites pass or fail depending only on the order they happen to run in.

The logger threshold is the main one. Do **not** call `Logger::setMinLogLevel()` bare;
use the RAII guard in `tests/common/scoped_min_log_level.h`, which restores the previous
value on scope exit (including when a test fails or throws):

```cpp
#include "scoped_min_log_level.h"

TEST(MySuite, StaysQuietWithoutLeaking)
{
    const dnf_composer::test::ScopedMinLogLevel quiet{ LogLevel::FATAL };
    // ... noisy work ...
}   // previous level restored here
```

For the validation suite, `silenceLogging()` returns the guard — keep it alive:
`const auto quiet = silenceLogging();`

The registered UI sink is the other one: `Logger::setUiSink()` is process-wide
state too, and this test binary never calls `Application::init()` (constructing
`Application` throws without a display), so no sink is registered by default.
Use the RAII guard in `tests/common/scoped_ui_sink.h` if a test needs to assert
on GUI-destined messages:

```cpp
#include "scoped_ui_sink.h"

TEST(MySuite, GuiMessageReachesTheSink)
{
    std::vector<std::string> received;
    const dnf_composer::test::ScopedUiSink guard{
        [&received](LogLevel, const std::string& message) { received.push_back(message); } };

    log(LogLevel::INFO, "hello", LogOutputMode::GUI);
    // received == { "...hello" }
}   // sink cleared here
```
