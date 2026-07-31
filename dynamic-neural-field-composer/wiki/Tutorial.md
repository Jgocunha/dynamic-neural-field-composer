# Tutorial: Your First Simulation

This walks through building a small Dynamic Neural Field simulation from scratch — a single field, a lateral-interaction kernel, and a stimulus — compiling it, running it, and saving it as a `.dnf` file. It assumes you've already built the project (see [Getting Started](Getting-Started)).

For the full element and API reference used along the way, see [Elements](Elements), [Element Reference](Element-Reference), and [Simulation](Simulation).

---

## 1. The pattern every example follows

Every executable in `examples/` (e.g. `examples/multi_peak.cpp`, `examples/detection_instability.cpp`) follows the same seven steps:

1. Create a `Simulation` and a `Visualization`
2. Create an `Application` and register UI windows
3. Create elements (fields, kernels, stimuli, noise, ...)
4. Add the elements to the simulation
5. Wire elements together with `addInput()`
6. Register plots
7. Run the `init()` / `step()` loop

We'll build a minimal architecture that reproduces the **detection instability**: a field with sub-critical lateral interactions that shows a bump only while a stimulus is present.

---

## 2. Write the source file

Create `examples/my_tutorial.cpp`:

```cpp
#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout.h"
#include "user_interface/main_menu_bar.h"

int main()
{
    try
    {
        using namespace dnf_composer;

        // 1. Simulation + Visualization
        const auto simulation    = std::make_shared<Simulation>("my tutorial", 5.0, 0.0, 0.0);
        const auto visualization = std::make_shared<Visualization>(simulation);

        // 2. Application + windows
        const Application app{ simulation, visualization };
        app.addWindow<user_interface::MainMenuBar>();
        app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

        // 3. Elements
        const auto stimulus = std::make_shared<element::GaussStimulus>(
            element::ElementCommonParameters{ "stimulus" },
            element::GaussStimulusParameters{ 5.0, 15.0, 50.0 });

        const auto field = std::make_shared<element::NeuralField>(
            element::ElementCommonParameters{ "field" },
            element::NeuralFieldParameters{ 25.0, -5.0, element::SigmoidFunction{ 0.0, 10.0 } });

        const auto kernel = std::make_shared<element::GaussKernel>(
            element::ElementCommonParameters{ "kernel" },
            element::GaussKernelParameters{ 3.0, 3.0, -0.01 });

        const auto noise = std::make_shared<element::NormalNoise>(
            element::ElementCommonParameters{ "noise" },
            element::NormalNoiseParameters{ 0.1 });

        // 4. Add to the simulation
        simulation->addElement(stimulus);
        simulation->addElement(field);
        simulation->addElement(kernel);
        simulation->addElement(noise);

        // 5. Wire interactions
        field->addInput(stimulus);   // external drive
        field->addInput(kernel);     // recurrent lateral interactions
        field->addInput(noise);      // background noise
        kernel->addInput(field);     // kernel reads the field's own output

        // 6. Plot
        visualization->plot({ { field->getUniqueName(), "activation" },
                               { field->getUniqueName(), "output" },
                               { field->getUniqueName(), "input" } });

        // 7. Run
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

A few things worth noting from this listing:

- `ElementCommonParameters{ "field" }` is the shorthand constructor — it fills in the default dimensions `{100, 1.0}` (100 positions, step size 1.0). Pass an explicit `ElementDimensions` if you need a different size.
- `field->addInput(kernel)` and `kernel->addInput(field)` together form the recurrent loop: the kernel reads the field's `"output"` component, convolves it, and feeds the result back as one of the field's inputs.
- The plot subscribes to three components of the same field — `"activation"` (raw membrane potential), `"output"` (after the sigmoid), and `"input"` (the summed drive from stimulus + kernel + noise). See [Elements → Components](Elements#components) for what each buffer means.

---

## 3. Register the executable

Add it next to the other examples in `examples/CMakeLists.txt`:

```cmake
add_example_executable(example_my_tutorial my_tutorial.cpp)
```

See [How to Create and Run Your Own Example Executable](How-to-Create-and-Run-Your-Own-Example-Executable) for more detail on this step.

---

## 4. Build and run

```bash
# Windows
scripts\build.bat
build\x64-release\example_my_tutorial.exe

# Linux
./scripts/build.sh
./build/linux-release/example_my_tutorial

# macOS
./scripts/build_macos.sh
./build/macos-release/example_my_tutorial
```

A static-layout ImGui window opens with the Node Graph, Element Inspector, Simulation Controls, and your plot docked together. Press play in **Simulation Controls** (or it may already be stepping, depending on build defaults) and watch the field's activation rise into a bump wherever the stimulus is centered (position 50 here).

---

## 5. Tune it live

With the GUI open:

- Select **field** in the **Element Inspector** and try lowering `startingRestingLevel` (more sub-threshold) or raising `tau` (slower response).
- Select **kernel** and increase `amplitude` — at some point the bump becomes self-sustaining even after you remove the stimulus (this crosses into the **memory instability** regime — compare with `examples/memory_instability.cpp`, which uses a `MexicanHatKernel` instead).
- See the [Parameter Tuning Guide](Parameter-Tuning-Guide) for a systematic walkthrough of what each parameter does and typical value ranges.

---

## 6. Save and reload the architecture

Once you're happy with an architecture, save it to disk instead of hand-writing it in code every time:

```cpp
simulation->save();                  // writes data/my tutorial/my tutorial.dnf
simulation->read("data/my tutorial/my tutorial.dnf");
```

Or from the GUI: **File → Save As...** / **File → Open**. The `.dnf` file is plain, pretty-printed JSON — see the [.dnf File Schema](DNF-File-Schema) reference for exactly what gets written and how to hand-edit it. You can also open any of the pre-built architectures under `data/` (e.g. `data/multi-peak/...`, `data/memory-trace/...`) the same way, as a starting point for your own experiments.

---

## Next steps

- [Elements](Elements) and [Element Reference](Element-Reference) — every element type and its constructor parameters
- [Simulation](Simulation) — the full `Simulation` API (lifecycle, querying, recording)
- [Examples](Examples) — a guided tour of all built-in example architectures
- [Parameter Tuning Guide](Parameter-Tuning-Guide) — practical advice for tuning field, kernel, and stimulus parameters
- [Troubleshooting](Troubleshooting) — fixes for common build and runtime errors
