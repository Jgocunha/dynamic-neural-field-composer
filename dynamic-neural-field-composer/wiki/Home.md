# dynamic-neural-field-composer

A C++20 library and interactive application for building and simulating **Dynamic Neural Field (DNF)** architectures.

---

## What are Dynamic Neural Fields?

Dynamic Neural Fields are a mathematical framework from computational neuroscience. They model how populations of neurons collectively represent, maintain, and transform information distributed over continuous feature dimensions — such as spatial position, orientation, or color.

A DNF is governed by a differential equation over a one-dimensional activation field `u(x, t)`. The field's dynamics emerge from the interplay between:

- A **resting level** that pulls activation downward in the absence of input
- A **lateral interaction kernel** (excitatory center, inhibitory surround) convolved with the field's output
- **External inputs** from stimuli or other fields

This produces characteristic behaviors that are useful for building cognitive and robotic architectures:

| Behavior | Description |
|---|---|
| **Detection** | A sufficiently strong input drives the field above threshold, forming a localized bump |
| **Working memory** | A bump persists self-sustainedly after the input is removed |
| **Selection** | Competing inputs resolve into a single winner-take-all bump |

---

## What this library provides

| Component | Description |
|---|---|
| **Simulation engine** | Time-stepped loop that manages and steps all elements |
| **Element library** | Neural fields, lateral interaction kernels, stimuli, field couplings, noise |
| **Learnable couplings** | Weight-matrix couplings with Hebbian, Oja, and Delta learning rules |
| **Real-time visualization** | Line plots and heatmaps rendered via ImPlot |
| **Interactive GUI** | Node-graph editor, element inspector, field metrics panel, plot controls |
| **Serialization** | Save and load complete simulation configurations as JSON |

The library is useful both as a simulation tool (run and inspect neural field dynamics interactively) and as a C++ framework (link it into your own application and drive fields programmatically).

It is also designed to be **easy to build from source** — a deliberate goal, since academic projects are often hard to compile. External dependencies are few and resolved automatically through [vcpkg](https://vcpkg.io/), so setup is a single script away.

---

## Namespace

All library code lives in the `dnf_composer` namespace. Elements are further nested under `dnf_composer::element`.

```cpp
using namespace dnf_composer;
using namespace dnf_composer::element;
```
