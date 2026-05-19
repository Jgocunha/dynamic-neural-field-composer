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
build\x64-release\Release\dnf_composer_tests.exe

# Windows (Debug)
build\x64-debug\Debug\dnf_composer_tests.exe

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
