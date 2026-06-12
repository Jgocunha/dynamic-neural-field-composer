# How to Create and Run Your Own Example Executable

This guide walks you through adding a custom example executable to the project.

---

## Step 1 — Create your example file

Navigate to the `examples/` directory and create a new `.cpp` file. Base it on any existing example in that folder — they all follow the same seven-step pattern (simulation → visualization → app → elements → wire → plots → loop). See the [Examples](Examples) page for a full walkthrough of each one.

```cpp
// examples/my_example.cpp
#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        const auto simulation    = std::make_shared<Simulation>("my example", 10.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);
        const Application app{ simulation, visualization };

        app.addWindow<user_interface::MainMenuBar>();
        app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

        // --- build your architecture here ---

        app.init();
        while (!app.hasGUIBeenClosed())
            app.step();
        app.close();
    }
    catch (const dnf_composer::Exception& ex)
    {
        log(dnf_composer::tools::logger::LogLevel::FATAL,
            "Exception: " + std::string(ex.what()),
            dnf_composer::tools::logger::LogOutputMode::CONSOLE);
        return static_cast<int>(ex.getErrorCode());
    }
    catch (const std::exception& ex)
    {
        log(dnf_composer::tools::logger::LogLevel::FATAL,
            "Exception: " + std::string(ex.what()),
            dnf_composer::tools::logger::LogOutputMode::CONSOLE);
        return 1;
    }
}
```

---

## Step 2 — Register in CMakeLists.txt

Open `examples/CMakeLists.txt` and add your executable using the `add_example_executable` helper:

```cmake
add_example_executable(example_my_example my_example.cpp)
```

Replace `example_my_example` with your desired executable name and `my_example.cpp` with your source filename. By convention all example targets are prefixed with `example_` so they group together with the built-in examples.

---

## Step 3 — Build

If you haven't built the project yet, run setup first (see [Getting Started](Getting-Started)). Then rebuild:

```bash
# Windows
scripts\build.bat

# Linux
./scripts/build.sh

# macOS
./scripts/build_macos.sh
```

Your executable will appear alongside the other example binaries in the build output folder.

---

## Step 4 — Run

```bash
# Windows (Release)
build\x64-release\example_my_example.exe

# Linux
./build/linux-release/example_my_example

# macOS
./build/macos-release/example_my_example
```

---

## Tips

- **Start simple** — one field, one kernel, one stimulus. Add complexity once the basic loop works.
- **Check existing examples** in `examples/` for reference architectures.
- **Use the GUI** — the Element Inspector and Node Graph let you adjust parameters live without recompiling.
- **Use proven parameter values** — copy parameters from an existing example that behaves similarly to what you want, then tune from there.
