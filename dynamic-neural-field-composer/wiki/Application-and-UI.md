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

## UI scale

The UI scale (50–200%) is a global property persisted across sessions via `imgui.ini`:

```cpp
Application::setUiScalePct(125.0f);
float scale = Application::getUiScalePct();
```

---

## Registering windows

Windows are added before `app.init()` is called. The `addWindow<T>()` template detects automatically what arguments the window type needs — you never need to pass `simulation` or `visualization` manually, except for `StaticLayoutWindow` which requires both:

```cpp
// Recommended: static composite layout
app.addWindow<user_interface::MainMenuBar>();
app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

// Or: individual free-floating windows (dynamic layout)
app.addWindow<user_interface::MainMenuBar>();
app.addWindow<user_interface::LogWindow>();
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
1. Calls `simulation->step()`
2. Calls `gui->render()`, which renders all registered ImGui windows and presents the frame (skipped if GUI is inactive)

---

## GUI control

```cpp
app.toggleGUI();               // show / hide the GUI overlay
bool active = app.isGUIActive();
bool closed = app.hasGUIBeenClosed();
```

---

## Available windows

### MainMenuBar

Top-level menu bar providing access to file operations (save/load simulation), layout presets, and global settings. Always include this window.

```cpp
app.addWindow<user_interface::MainMenuBar>();
```

### StaticLayoutWindow

A composite window that arranges all panels — simulation controls, element inspector, field monitor, node graph, plots, plot control, and log — into a fixed tiled layout that adapts to the window size. This is the preferred layout for normal use.

```cpp
app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);
```

Internally it hosts: `SimulationWindow`, `ElementWindow`, `NodeGraphWindow`, `FieldMetricsWindow`, `PlotControlWindow`, `PlotsWindow`, and `LogWindow`.

### SimulationWindow

Controls for starting, pausing, stepping, and resetting the simulation. Each functional area (simulation parameters, run controls, element management, interaction editor) is rendered as a collapsible **Card**.

#### Export Data panel (tab 5 — download icon)

The **Export Data** pane unifies recording and snapshot export. Select an element and component, then:

| Section | Controls | Action |
|---|---|---|
| **Continuous Recording** | Interval input + ms/ticks unit | **● Record** starts writing a time-series CSV; **■ Stop** closes it |
| **Snapshot Export** | — | **📷 Export** writes a single-row CSV of the current state |

Files land in:
- Recordings: `data/<sim_name>/recordings/<id>_<component>_<timestamp>.csv`
- Snapshots: `data/<sim_name>/exports/<id>_<component>_<timestamp>.csv`

For **2D elements**, a `# size_x=W,size_y=H` comment line is written as the first line of the CSV so that downstream tools can reconstruct the spatial grid from the flat column sequence.

The **Record** button is red and disabled while already recording. The **Stop** button is disabled when no recording is active for the selected pair. The **Export** button is always available once an element and component are selected.

```cpp
app.addWindow<user_interface::SimulationWindow>();
```

### ElementWindow

A panel listing all elements in the simulation. Selecting an element shows and allows live editing of its parameters (tau, resting level, kernel widths, etc.) and its spatial dimensions. For kernel elements, **Output Size** and **Output Step** controls are also available to configure cross-dimension output (see [Cross-dimension kernels](Element-Reference#cross-dimension-kernels)). Changing dimensions severs existing connections; reconnect through the NodeGraphWindow or in code.

```cpp
app.addWindow<user_interface::ElementWindow>();
```

### FieldMetricsWindow

Real-time metrics for all `NeuralField` elements, rendered as a responsive card grid. Each card shows the field's stability status, lowest/highest activation, bump count, and per-bump details (centroid, amplitude, width, velocity, acceleration). Cards reflow automatically when the window is resized.

```cpp
app.addWindow<user_interface::FieldMetricsWindow>();
```

### PlotControlWindow

UI for adding and removing data series from existing plots at runtime. Lets you pick any element and component and attach it to a plot without recompiling.

```cpp
app.addWindow<user_interface::PlotControlWindow>();
```

### PlotsWindow

Hosts all active `implot` charts (line plots and heatmaps) registered through `Visualization::plot()`. Charts are arranged in a **tiled layout** that recomputes column widths and row heights whenever a plot is added/removed or the window is resized.

```cpp
app.addWindow<user_interface::PlotsWindow>();
```

### NodeGraphWindow

An interactive node-graph editor (via `imgui-node-editor`) that shows the element interaction graph. Nodes represent elements; edges represent connections. You can add elements and wire them visually.

```cpp
app.addWindow<user_interface::NodeGraphWindow>();
```

### LogWindow

A scrollable, filterable log output window showing messages from the logger at all levels (DEBUG, INFO, WARNING, ERROR, FATAL).

```cpp
app.addWindow<user_interface::LogWindow>();
```

---

## Widgets

Custom reusable widgets are available in the `dnf_composer::user_interface::widgets` namespace:

```cpp
#include "user_interface/widgets.h"
```

| Widget | Description |
|---|---|
| `renderHelpMarker(desc)` | A `(?)` tooltip trigger |
| `renderSidebarTab(icon, label, selected)` | An icon + label sidebar tab button, returns `true` when clicked |
| `renderIconTileButton(id, icon, label, ...)` | A large icon tile button with configurable colors |
| `Card` | A bordered, rounded child region with a title; use `beginCard()` / `endCard()` |
| `BeginHorizontal` / `EndHorizontal` | Group widgets in a horizontal layout |
| `BeginVertical` / `EndVertical` | Group widgets in a vertical layout |

### Card usage

```cpp
using namespace dnf_composer::user_interface::widgets;

Card card{ "##my_card", pos, size, "Card Title" };
if (card.beginCard(Application::getUiScalePct()))
{
    ImGui::Text("Card content here");
    Card::endCard();
}
```

---

## Fonts

The application loads the **Cera Pro** typeface in four weights (Light, Medium, Bold, Black) and three sizes, plus a **JetBrains Mono** monospace font and **Font Awesome 6** icon sets. Font globals are declared in `application.h`:

| Global | Weight | Size |
|---|---|---|
| `g_LightMediumFont` | Light | 18 px |
| `g_LightSmallFont` | Light | 12 px |
| `g_LightLargeFont` | Light | 24 px |
| `g_MediumSmallFont` | Medium | 12 px |
| `g_MediumMediumFont` | Medium (default font) | 18 px |
| `g_MediumLargeFont` | Medium | 24 px |
| `g_BoldSmallFont` | Bold | 12 px |
| `g_BoldMediumFont` | Bold | 18 px |
| `g_BoldLargeFont` | Bold | 24 px |
| `g_BlackSmallFont` | Black | 20 px |
| `g_BlackMediumFont` | Black | 24 px |
| `g_BlackLargeFont` | Black | 30 px |
| `g_MonoSmallFont` | JetBrains Mono | 16 px |
| `g_MonoMediumFont` | JetBrains Mono | 20 px |
| `g_MonoLargeFont` | JetBrains Mono | 26 px |
| `g_SmallIconsFont` | Font Awesome 6 | 12 px |
| `g_MediumIconsFont` | Font Awesome 6 | 18 px |
| `g_LargeIconsFont` | Font Awesome 6 | 24 px |

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
