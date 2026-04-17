# Visualization

The `Visualization` class manages a collection of plots that display element components in real time. It reads directly from the simulation's component buffers on every frame.

```cpp
#include "visualization/visualization.h"
```

---

## Construction

```cpp
auto simulation    = std::make_shared<Simulation>("sim");
auto visualization = std::make_shared<Visualization>(simulation);
```

`Visualization` holds a shared reference to the simulation and queries its components during `render()`.

---

## Plot types

| Enum | Description |
|---|---|
| `PlotType::LINE_PLOT` | 1D line chart — one or more overlaid curves |
| `PlotType::HEATMAP` | 2D color map — one 2D data matrix (e.g. coupling weights) |

---

## Adding plots

All `plot()` overloads share the same general form: provide common parameters, type-specific parameters, and a list of `{element name, component name}` pairs that specify what data to display.

### Full-parameter overload

```cpp
visualization->plot(
    PlotCommonParameters{
        PlotType::LINE_PLOT,                            // plot type
        PlotDimensions{ xMin, xMax, yMin, yMax,         // axis limits
                        xStep, yStep },                  // tick steps
        PlotAnnotations{ "Title", "X label", "Y label" } // labels
    },
    LinePlotParameters{},
    {
        { "element name", "component name" },
        { "element name", "component name" },
        // ...
    }
);
```

### Heatmap example

```cpp
visualization->plot(
    PlotCommonParameters{
        PlotType::HEATMAP,
        PlotDimensions{ 0.0, 280, 0.0, 280, 1.0, 1.0 },
        PlotAnnotations{ "Coupling weights", "Output position", "Input position" }
    },
    HeatmapParameters{},
    { { "coupling element name", "weights" } }
);
```

### Shorthand overloads

```cpp
// Use all defaults
visualization->plot();

// Just specify the data (default common parameters)
visualization->plot({ { "field", "activation" } });
visualization->plot("field name", "activation");

// Add data to an existing plot by its ID
visualization->plot(plotId, { { "field", "output" } });
visualization->plot(plotId, "field name", "output");
```

---

## PlotCommonParameters

```cpp
struct PlotCommonParameters {
    PlotType       type;
    PlotDimensions dimensions;
    PlotAnnotations annotations;
};
```

### PlotDimensions

```cpp
struct PlotDimensions {
    double xMin, xMax;   // horizontal axis range
    double yMin, yMax;   // vertical axis range
    double xStep, yStep; // tick mark step sizes
};
```

### PlotAnnotations

```cpp
struct PlotAnnotations {
    std::string title;
    std::string xLabel;
    std::string yLabel;
};
```

---

## Removing plots

```cpp
visualization->removePlot(plotId);
visualization->removeAllPlots();
visualization->removePlottingDataFromPlot(plotId, { "field name", "activation" });
```

---

## Rendering

`render()` is called automatically by `Application::step()` every frame. You do not need to call it manually when using `Application`.

```cpp
visualization->render();
```

---

## Accessing plots programmatically

```cpp
auto plots = visualization->getPlots();
// Returns: unordered_map<shared_ptr<Plot>, vector<pair<string,string>>>
// Key: plot object, Value: list of {element, component} pairs
```