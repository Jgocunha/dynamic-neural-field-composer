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

Simulations can be saved and loaded as `.dnf` files via `SimulationFileManager`, which is invoked through these convenience methods:

```cpp
// Save to a file path (opens a file dialog if path is empty)
sim.save("path/to/file.dnf");
sim.save();   // opens file dialog

// Load from a file path (opens a file dialog if path is empty)
sim.read("path/to/file.dnf");
sim.read();   // opens file dialog
```

The `.dnf` file captures all element types, their parameters, their spatial dimensions, and the interaction graph. Field coupling weights are written alongside the `.dnf` file in the same simulation sub-folder.

**Default output layout** (`data/<simulation_name>/`):

| File | Description |
|---|---|
| `<name>.dnf` | Simulation element graph |
| `<coupling_name>_weights.txt` | FieldCoupling weight matrix |
| `exports/<id>_<component>_<ts>.csv` | Single-frame snapshots |
| `recordings/<id>_<component>_<ts>.csv` | Time-series recordings |

---

## Recording and exporting component data

The `SimulationRecorder` (accessed via `sim.getRecorder()`) supports two modes:

### Continuous recording

Records a time-series to a CSV file at a configurable sample interval. The file is written incrementally as the simulation steps.

```cpp
// Start recording "activation" of "nf 1" every 5 ticks
sim.getRecorder().startRecording(
    sim.getUniqueIdentifier(),   // simulation name (used as sub-folder)
    "nf 1",                      // element id
    "activation",                // component name
    5,                           // sample interval
    RecordingIntervalUnit::Ticks // or RecordingIntervalUnit::Milliseconds
);

// ... run the simulation ...

// Stop when done
sim.getRecorder().stopRecording("nf 1", "activation");
// or stop all active recordings:
sim.getRecorder().stopAll();
```

### Snapshot export

Writes a single row (current state) to a CSV file immediately:

```cpp
sim.getRecorder().takeSnapshot(
    sim.getUniqueIdentifier(), "nf 1", "activation", sim);
```

### CSV format

Both modes produce the same format:

```
ticks,ms,0,1,2,...,N-1
42,42.000000,0.123456,-0.012345,...
```

- `ticks` = derived as `round((t - tZero) / deltaT)`
- `ms` = `sim.t` (current simulation time in ms)
- Columns `0`…`N-1` = the component vector values

**2D elements** additionally emit a metadata comment line as the first line of the file, before the column header:

```
# size_x=10,size_y=10
ticks,ms,0,1,...,99
42,42.000000,...
```

The comment encodes the grid dimensions so downstream tools can reshape the flat column sequence back into a 2D array. The data is stored in row-major order: index `i` maps to grid position `x = i % size_x`, `y = i / size_x`.

---

## Status flags

```cpp
bool ready   = sim.isInitialized();
```