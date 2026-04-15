# dynamic-neural-field-composer

<img src="./dynamic-neural-field-composer/resources/images/logo.png" alt="logo" >

A C++20 library and interactive application for building and simulating **Dynamic Neural Field (DNF)** architectures.

===============================================

## Description

Dynamic Neural Fields are a mathematical framework from computational neuroscience that describes how populations of neurons represent, sustain, and transform information distributed over continuous feature dimensions — such as spatial position, direction, or color. A DNF is governed by a differential equation that produces rich emergent behaviors including self-sustained activation bumps (working memory), winner-take-all selection, and sequence generation, all arising from the interplay of local excitation and surround inhibition.

This library provides a complete toolkit for designing, connecting, simulating, and visualizing DNF architectures at runtime. Simulations are defined programmatically or via a visual node-graph editor, and can be saved and reloaded as JSON. Researchers use it to prototype neural field models; engineers use it to drive cognitive architectures for robotic tasks.

## Functionalities

- **Flexible Element Management**: Dynamically add, modify, and remove neural field elements during simulations.
- **Advanced Simulation Controls**: Initiate, pause, and step through simulations with precise timing control.
- **Interactive GUI**: Utilize integrated GUI controls for managing simulations and visualizing data in real time.
- **Data Export and Logging**: Export neural field data and configurations, facilitating detailed analysis and reproducibility.
- **Extensible Architecture**: Easily extend with new neural field models, learning rules, or simulation protocols.
- **Learning and Adaptation**: Implement learning algorithms to adjust synaptic weights based on predefined or dynamic rules.
- **Plotting and Visualization**: Integrated plotting tools to visualize neural activity and simulation metrics.
- **Centroid Monitoring**: Track and display the centroid of neural activations to study stability and activity patterns.

---

## Requirements

- **CMake** 3.20 or later
- **C++20** compiler (MSVC, GCC, or Clang)
- **vcpkg** (set `VCPKG_ROOT` environment variable)

vcpkg dependencies (installed automatically):

| Package | Purpose |
|---|---|
| `imgui` | Immediate-mode GUI |
| `implot` | Real-time plotting |
| `unofficial-imgui-node-editor` | Visual node graph |
| `nlohmann-json` | Simulation serialization |
| `imgui-platform-kit` | Platform/window abstraction |

---

## Building

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

Or manually with CMake:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

To install the library for use in another project:

```bash
# Windows
install.bat

# Linux / macOS
./install.sh
```

---

## Getting started

1. Clone this repository to your local machine using Git.
2. Run the ```build.bat``` file. This will install all the necessary dependencies and build the project. Make sure you have VCPKG installed and the VCPKG_ROOT environment variable defined.
3. You can run the example executables to see the library in action.

The best way to get familiar with the library is to take a look at the ```examples``` folder!

---

## Contributing

Contributions to the Dynamic Neural Field Composer are welcomed. Whether it involves fixing bugs, adding new features, or improving the documentation, your help is appreciated to make this project even better.
