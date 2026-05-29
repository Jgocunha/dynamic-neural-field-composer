# Architecture Overview

## Module map

```
dynamic-neural-field-composer/
├── application/        Application entry point and GUI lifecycle
├── simulation/         Time-stepped simulation engine + JSON persistence
├── elements/           All element types (neural fields, kernels, couplings, ...)
├── element_parameters/ Shared parameter structs and element identity types
├── visualization/      Plot management (line plots, heatmaps)
├── user_interface/     ImGui windows (node graph, inspector, metrics, ...)
├── tools/              Logger, math utilities, profiling, file dialogs
└── exceptions/         Custom exception hierarchy
```

---

## Namespaces

| Namespace | Contents |
|---|---|
| `dnf_composer` | `Simulation`, `Visualization`, `Application`, `Exception`, learning rules |
| `dnf_composer::element` | All element classes and their parameter structs, `ElementFactory` |
| `dnf_composer::user_interface` | All ImGui window classes |
| `dnf_composer::tools::logger` | Logging utilities |
| `dnf_composer::tools` | Math, profiling, utils |

---

## Layer design

```
┌─────────────────────────────────────────┐
│              Application                │  GUI lifecycle, window management
├──────────────────┬──────────────────────┤
│   Visualization  │    User Interface    │  Plotting, node graph, inspectors
├──────────────────┴──────────────────────┤
│               Simulation                │  Element management, time loop
├─────────────────────────────────────────┤
│               Elements                  │  Neural fields, kernels, couplings
├─────────────────────────────────────────┤
│          Tools / Exceptions             │  Logger, math, profiling, errors
└─────────────────────────────────────────┘
```

Each layer depends only on layers below it. The `Application` drives both the `Simulation` and the `Visualization`; neither of them knows about the other's internals.

---

## Execution flow

1. **Setup** — create `Simulation`, `Visualization`, `Application`; add windows; create elements and wire interactions; register plots
2. **Init** — `app.init()` initializes the GUI and calls `Simulation::init()` on all elements
3. **Loop** — `app.step()` calls `Simulation::step()` (advances all elements by `deltaT`) and `Visualization::render()` (updates plots), then renders the ImGui frame
4. **Shutdown** — `app.close()` tears down the GUI and frees resources

```
app.init()
    └─► simulation.init()
            └─► element.init()  [for each element]

app.step()  [called every frame]
    ├─► simulation.step()
    │       └─► element.step(t, deltaT)  [for each element]
    └─► visualization.render()
            └─► [update each plot from simulation components]
```

---

## Key design decisions

### UIMode and layout strategy

`Application` supports two layout modes controlled by `UIMode`:

- `UIMode::Dynamic` — each window is a free-floating, dockable ImGui panel
- `UIMode::Static` — all panels are hosted inside `StaticLayoutWindow`, which arranges them in a fixed tiled layout

`StaticLayoutWindow` is the preferred production layout: it owns instances of every sub-window and manages sizing/positioning in `drawPanels()`.

### Elements as nodes in a graph
Elements form a directed graph. Each element holds references to its input elements and reads from a named component (e.g. `"output"`) during `step()`. Interactions are explicit: calling `a->addInput(b)` registers `b` as a source for `a`.

### Components as named buffers
Every element exposes its internal state as named `std::vector<double>` buffers called **components** (e.g. `"activation"`, `"output"`, `"input"`, `"weights"`). The visualization and UI read from these buffers directly, with no coupling to the element's type.

### ElementFactory
All element construction goes through `ElementFactory::createElement()`, which takes an `ElementLabel` enum and typed parameter structs. This decouples callers from concrete constructors and is the construction path used by the GUI and JSON loader.

### JSON persistence
`SimulationFileManager` serializes and deserializes the full element graph to/from JSON. It is invoked via `Simulation::save()` and `Simulation::read()`. Each simulation's files are co-located under `data/<simulation_name>/`:

```
data/
└── <simulation_name>/
    ├── <simulation_name>.json          # element graph + parameters
    ├── <coupling_name>_weights.txt     # FieldCoupling weight matrix (one file per coupling element)
    ├── exports/                        # snapshot CSV files (one per takeSnapshot() call)
    └── recordings/                     # time-series CSV files (one per startRecording() session)
```

### Recording and snapshot export
`SimulationRecorder` (owned by `Simulation`, accessible via `getRecorder()`) manages ongoing time-series recordings and one-shot snapshot exports. It is called automatically from `Simulation::step()` and closed in `close()` / `clean()`.