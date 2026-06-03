
![logo](./dynamic-neural-field-composer/resources/images/logo.png)

<h1 align="center">dynamic-neural-field-composer</h1>

<p align="center">
  <strong>A C++ library and interactive application for building and simulating Dynamic Neural Field architectures.</strong>
</p>

<p align="center">
  <a href="https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/ci.yml"><img src="https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/ci.yml/badge.svg" alt="CI" /></a>
  <a href="https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/static-analysis.yml"><img src="https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/static-analysis.yml/badge.svg" alt="Static Analysis" /></a>
  <a href="https://github.com/Jgocunha/dynamic-neural-field-composer/releases/latest"><img src="https://img.shields.io/github/v/release/Jgocunha/dynamic-neural-field-composer" alt="Latest Release" /></a>
  <a href="https://codecov.io/gh/Jgocunha/dynamic-neural-field-composer"><img src="https://codecov.io/gh/Jgocunha/dynamic-neural-field-composer/graph/badge.svg" alt="Coverage" /></a>
  <a href="https://jgocunha.github.io/dynamic-neural-field-composer/"><img src="https://img.shields.io/badge/docs-doxygen-blue" alt="Docs" /></a>
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue" alt="C++20" />
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey" alt="Platform" />
</p>

<p align="center">
  <a href="https://jgocunha.github.io/dynamic-neural-field-composer/">Docs</a> ·
  <a href="https://github.com/Jgocunha/dynamic-neural-field-composer/wiki">Wiki</a> ·
  <a href="https://github.com/Jgocunha/dynamic-neural-field-composer/releases">Releases</a> ·
  <a href="CONTRIBUTING.md">Contributing</a>
</p>


---

## About

Dynamic Neural Fields model how neuron populations represent and transform information over continuous dimensions (position, direction, color). They produce emergent behaviours — working memory, winner-take-all selection, sequence generation — from local excitation and surround inhibition.

This library lets you design, connect, simulate, and visualize DNF architectures at runtime, either programmatically or through a visual node-graph editor. Simulations can be saved and reloaded as JSON. The application is built with real-time performance in mind, and straightforward to embed in any C++ application.

- **Compose architectures in C++** — wire up fields, kernels, stimuli, and couplings in a few lines
- **Visual node-graph editor** — connect and reconfigure elements at runtime through an ImGui-based interface
- **Real-time plots** — inspect field activation, output, and input live as the simulation runs
- **Save and reload** — serialize any architecture to JSON and resume it later
- **Embeddable** — link against the library and integrate DNF simulation into any C++ application

![Compose DNF architectures — add elements, define connections, simulate in real-time](./dynamic-neural-field-composer/resources/images/headline-summary.jpg)

---

## Download

Pre-compiled binaries are available on the [Releases](https://github.com/Jgocunha/dynamic-neural-field-composer/releases/latest) page. Download and run — no build tools or dependencies required. This is the quickest way to start composing and simulating DNF architectures without writing any code.

If you want to build from source, embed the library in your own project, or write custom examples, follow the steps below.

---

## Requirements

You must install the following manually:

| Platform | Requirements |
|---|---|
| **Windows** | Visual Studio 2022 with "Desktop development with C++" workload, Git |
| **Linux** | GCC 13+, CMake 3.20+, Git, OpenGL/X11 dev libraries |
| **macOS** | Xcode Command Line Tools, CMake 3.20+ |

Everything else — vcpkg, all library dependencies (`imgui`, `implot`, `imgui-node-editor`, `nlohmann-json`, `imgui-platform-kit`) — is installed automatically by the setup scripts.

## Building

Run setup once on a fresh machine, then build whenever you want to compile.

```bash
# Windows
scripts\setup.bat
scripts\build.bat

# Linux
chmod +x scripts/setup.sh scripts/build.sh
./scripts/setup.sh
./scripts/build.sh

# macOS
chmod +x scripts/setup.sh scripts/build_macos.sh
./scripts/setup.sh
./scripts/build_macos.sh
```

To install the library for use in another CMake project:

```bash
# Windows
scripts\install.bat

# Linux / macOS
./scripts/install.sh
```

---

## Quick start

Define a field architecture in a few lines of C++ and watch it run:

```cpp
#include "application/application.h"
#include "user_interface/static_layout.h"

int main()
{
    using namespace dnf_composer;

    const auto simulation = std::make_shared<Simulation>("Boost detection", 10.0, 0.0, 0.0);
    const auto visualization = std::make_shared<Visualization>(simulation);
    const Application app{ simulation, visualization };

    app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

    // Neural field with Mexican hat kernel — local excitation, surround inhibition
    const auto nf = std::make_shared<element::NeuralField>(
        element::ElementCommonParameters{ "Neural field" },
        element::NeuralFieldParameters{});
    const auto k = std::make_shared<element::MexicanHatKernel>(
        element::ElementCommonParameters{ "Mexican hat kernel" },
        element::MexicanHatKernelParameters{});
    const auto gs = std::make_shared<element::GaussStimulus>(
        element::ElementCommonParameters{ "Gauss stimulus" },
        element::GaussStimulusParameters{ 5.0, 4.0, 50.0 });

    simulation->addElement(nf);
    simulation->addElement(k);
    simulation->addElement(gs);

    nf->addInput(gs);
    nf->addInput(k);
    k->addInput(nf);

    visualization->plot({ {nf->getUniqueName(), "activation"} });

    app.init();
    while (!app.hasGUIBeenClosed())
        app.step();
    app.close();
}
```

More ready-to-run examples are in the [`examples/`](dynamic-neural-field-composer/examples/) folder, covering working memory, selection, sequence generation, 2D fields, Hebbian learning, and more.

![Monitoring and visualization — plot field components, monitor state, inspect parameters](./dynamic-neural-field-composer/resources/images/headline-plotting.jpg)


---

## Elements

| Category | Elements |
|---|---|
| Fields | `NeuralField`, `NeuralField2d` |
| Kernels | `GaussKernel`, `MexicanHatKernel`, `AsymmetricGaussKernel`, `OscillatoryKernel` (+ 2D variants) |
| Stimuli | `GaussStimulus`, `TimedGaussStimulus`, `BoostStimulus`, `BoostStimulus2d` (+ 2D variants) |
| Noise | `NormalNoise`, `CorrelatedNormalNoise` (+ 2D variants) |
| Couplings | `FieldCoupling`, `GaussFieldCoupling` |
| Memory | `MemoryTrace`, `MemoryTrace2d` |
| Resampling | `Resize`, `Resize2d` — interpolate an input field to a different spatial size (linear / nearest / cubic) |
| Dimensionality | `Collapse` — reduce a 2D field to 1D along an axis (sum / average / maximum / minimum); `Expand` — broadcast a 1D field into a 2D ridge |


![Element suite — neural fields, kernels, stimuli, couplings in 1D and 2D](./dynamic-neural-field-composer/resources/images/headline-elements.jpg)

---

## Two launchers, one library

Building the project produces two executables:

- **`dnf-composer-static`** — a single self-contained window with all panels in a fixed layout. Best for quickly running a simulation without any setup.
- **`dnf-composer-dynamic`** — a fully dockable ImGui application. Windows can be rearranged, detached, and dragged to secondary monitors.


 The GUI is powered by [Dear ImGui](https://github.com/ocornut/imgui), keeping it fast and lightweight.

![Flexible UI — dock, detach, and arrange windows however you need](./dynamic-neural-field-composer/resources/images/headline-gui.jpg)

You can also write your own launcher, link against the library, and choose exactly which windows and architectures to load. See the [Wiki](https://github.com/Jgocunha/dynamic-neural-field-composer/wiki/How-to-Create-and-Run-Your-Own-Example-Executable) for a step-by-step guide.

---

## Projects using dynamic-neural-field-composer

| Project | Publication |
|---|---|
| [NEAT-DNFs](https://github.com/Jgocunha/neat-dnfs) | *NEAT-DNFs: A NeuroEvolutionary Framework for Evolving Dynamic Neural Field Architectures* · GECCO 2026 · [10.1145/3795095.3805169](https://doi.org/10.1145/3795095.3805169) |
| [dynamic-neural-field-degeneration](https://github.com/Jgocunha/dynamic-neural-field-degeneration) | *Robustness and Adaptability in a Dynamic Neural Field Architecture Subject to Degeneration* · ROBOT 2025 · Springer LNNS |
| [vr-hr-joint-task](https://github.com/Jgocunha/vr-hr-joint-task) | *Dynamic Neural Field Based Anticipatory Action Selection for Human Robot Collaboration: A Virtual Reality Experiment* · ICSR + ART 2026 |
| [How We Can Use Dynamic Neural Fields in Human-Robot Joint Action](https://research.tue.nl/en/studentTheses/how-we-can-use-dynamic-neural-fields-in-human-robot-joint-action/) | *How We Can Use Dynamic Neural Fields in Human-Robot Joint Action* · Tessa H. Janssen · MSc thesis · TU/e |

---

## Contributing

Bug fixes, new elements, documentation improvements, and example architectures are all welcome. Open an issue before starting non-trivial work so direction can be agreed on first. See [CONTRIBUTING.md](CONTRIBUTING.md) for the full process.

## License

This project is licensed under the terms in [LICENSE](LICENSE).