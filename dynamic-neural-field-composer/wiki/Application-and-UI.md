# Application & UI

The `Application` class ties together the `Simulation`, `Visualization`, and the ImGui-based GUI. It manages the platform window, the render loop, and all registered UI windows.

```cpp
#include "application/application.h"
```

---

## Construction

```cpp
auto simulation    = std::make_shared<Simulation>("sim");
auto visualization = std::make_shared<Visualization>(simulation);

const Application app{ simulation, visualization };
```

Both arguments are optional. You can pass `nullptr` for either if you do not need visualization or if you want a headless simulation.

---

## Registering windows

Windows are added before `app.init()` is called. The `addWindow<T>()` template detects automatically what arguments the window type needs — you never need to pass `simulation` or `visualization` manually:

```cpp
app.addWindow<user_interface::MainWindow>();
app.addWindow<imgui_kit::LogWindow>();
app.addWindow<user_interface::FieldMetricsWindow>();
app.addWindow<user_interface::ElementWindow>();
app.addWindow<user_interface::SimulationWindow>();
app.addWindow<user_interface::PlotControlWindow>();
app.addWindow<user_interface::PlotsWindow>();
app.addWindow<user_interface::NodeGraphWindow>();
```

---

## Application lifecycle

```cpp
app.init();                    // initialize GUI and simulation

while (!app.hasGUIBeenClosed())
    app.step();                // render one frame + advance simulation

app.close();                   // tear down GUI and simulation
```

`step()` does the following on each call:
1. Calls `simulation->step()` (if not paused)
2. Calls `visualization->render()`
3. Renders all registered ImGui windows
4. Presents the frame

---

## GUI control

```cpp
app.toggleGUI();               // show / hide the GUI overlay
bool active = app.isGUIActive();
bool closed = app.hasGUIBeenClosed();
```

---

## Available windows

### MainWindow

Menu bar providing access to file operations (save/load simulation), layout presets, and global settings. Always include this window.

```cpp
app.addWindow<user_interface::MainWindow>();
```

### SimulationWindow

Controls for starting, pausing, stepping, and resetting the simulation. Displays current simulation time and `deltaT`.

```cpp
app.addWindow<user_interface::SimulationWindow>();
```

### ElementWindow

A panel listing all elements in the simulation. Selecting an element shows and allows live editing of its parameters (tau, resting level, kernel widths, etc.).

```cpp
app.addWindow<user_interface::ElementWindow>();
```

### FieldMetricsWindow

Real-time metrics for all `NeuralField` elements: bump count, centroid positions, amplitudes, widths, min/max activation, and stability status.

```cpp
app.addWindow<user_interface::FieldMetricsWindow>();
```

### PlotControlWindow

UI for adding and removing data series from existing plots at runtime. Lets you pick any element and component and attach it to a plot without recompiling.

```cpp
app.addWindow<user_interface::PlotControlWindow>();
```

### PlotsWindow

Hosts all active `implot` charts (line plots and heatmaps) registered through `Visualization::plot()`.

```cpp
app.addWindow<user_interface::PlotsWindow>();
```

### NodeGraphWindow

An interactive node-graph editor (via `imgui-node-editor`) that shows the element interaction graph. Nodes represent elements; edges represent connections. You can add elements and wire them visually.

```cpp
app.addWindow<user_interface::NodeGraphWindow>();
```

### LogWindow

A scrollable log output window showing messages from the logger at all levels (DEBUG, INFO, WARNING, ERROR, FATAL).

```cpp
app.addWindow<imgui_kit::LogWindow>();
```

---

## Logging

The logger is a global utility available everywhere in the library:

```cpp
#include "tools/logger.h"
using namespace dnf_composer::tools::logger;

log(LogLevel::INFO,    "Simulation started");
log(LogLevel::WARNING, "Parameter out of expected range");
log(LogLevel::ERROR,   "Element not found: " + name);
log(LogLevel::FATAL,   "Unrecoverable error");

// Control output destination
log(LogLevel::INFO, "Console only", LogOutputMode::CONSOLE);
log(LogLevel::INFO, "GUI only",     LogOutputMode::GUI);
log(LogLevel::INFO, "Both",         LogOutputMode::ALL);   // default

// Set minimum level globally
Logger::setMinLogLevel(LogLevel::WARNING);
```

---

## Error handling

The library uses a custom `Exception` type with an associated `ErrorCode`. The main entry point pattern:

```cpp
try {
    // ... setup and loop
}
catch (const dnf_composer::Exception& ex) {
    log(LogLevel::FATAL,
        "Exception: " + std::string(ex.what()) +
        " ErrorCode: " + std::to_string(static_cast<int>(ex.getErrorCode())));
    return static_cast<int>(ex.getErrorCode());
}
catch (const std::exception& ex) {
    log(LogLevel::FATAL, "Exception: " + std::string(ex.what()));
    return 1;
}
```