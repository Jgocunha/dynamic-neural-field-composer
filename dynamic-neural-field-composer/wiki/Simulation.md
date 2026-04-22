# Simulation

The `Simulation` class is the central object of the library. It owns all elements, drives the time loop, and handles persistence.

```cpp
#include "simulation/simulation.h"
```

---

## Construction

```cpp
// Full constructor
Simulation sim(
    "my simulation",  // unique identifier (string)
    1.0,              // deltaT — time step size
    0.0,              // tZero — simulation start time
    0.0               // t — current time (usually starts at 0)
);

// Factory function (returns shared_ptr)
auto sim = dnf_composer::createSimulation("my simulation", 1.0, 0.0, 0.0);

// Shared pointer construction (recommended for use with Application)
auto sim = std::make_shared<Simulation>("my simulation");
```

---

## Lifecycle

```cpp
sim.init();          // initializes all registered elements
sim.step();          // advances all elements by deltaT
sim.run(1000.0);     // step N times (headless, no GUI)
sim.pause();         // suspends stepping
sim.resume();        // resumes stepping after pause
sim.close();         // tears down elements
sim.clean();         // removes all elements without closing
```

`init()` must be called before the first `step()`. When using `Application`, `app.init()` handles this automatically.

---

## Element management

```cpp
// Add
sim.addElement(std::make_shared<NeuralField>(...));

// Remove by name
sim.removeElement("element name");

// Replace an element while preserving its position in the graph
sim.resetElement("old element name", newElement);

// Wire elements (shorthand for element->addInput())
sim.createInteraction(
    "source element name",   // the element providing data
    "output",                // which component to read
    "target element name"    // the element receiving the data
);
```

---

## Querying elements and data

```cpp
// Get element by name or index
auto el = sim.getElement("field name");
auto el = sim.getElement(0);

// Get all elements
auto elements = sim.getElements();

// Read a component's data (copy)
std::vector<double> data = sim.getComponent("field name", "activation");

// Get a live pointer to a component buffer (no copy)
std::vector<double>* ptr = sim.getComponentPtr("field name", "activation");

// Check if a component exists
bool exists = sim.componentExists("field name", "activation");

// Get all elements that use a specified element as input
auto dependents = sim.getElementsThatHaveSpecifiedElementAsInput("field name");

// Counts
int n = sim.getNumberOfElements();
int maxIdx = sim.getHighestElementIndex();
```

---

## Time parameters

| Member | Description |
|---|---|
| `deltaT` | Time step size (public, writable) |
| `tZero` | Simulation start time (read via `getTZero()`) |
| `t` | Current simulation time (read via `getT()`) |

```cpp
sim.setDeltaT(0.5);
double currentTime = sim.getT();
```

---

## Persistence (save / load)

Simulations can be saved and loaded as JSON files via `SimulationFileManager`, which is invoked through these convenience methods:

```cpp
// Save to a file path (opens a file dialog if path is empty)
sim.save("path/to/file.json");
sim.save();   // opens file dialog

// Load from a file path (opens a file dialog if path is empty)
sim.read("path/to/file.json");
sim.read();   // opens file dialog
```

The JSON file captures all element types, their parameters, their spatial dimensions, and the interaction graph. Field coupling weights are stored separately via `FieldCoupling::writeWeights()` / `readWeights()`.

---

## Export component data

```cpp
// Write a component's time-series to a CSV file in the data/ directory
sim.exportComponentToFile("field name", "activation");
```

---

## Status flags

```cpp
bool ready   = sim.isInitialized();
```