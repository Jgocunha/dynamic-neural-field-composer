# 05 — GUI layer: headless-testable logic

The GUI layer cannot be rendered in CI, but a useful slice of it is pure logic.
The pattern (established by `tests/application/test_application.cpp`): construct
objects and exercise state/logic, but **never** call anything that reaches
`ImGui::*` / OpenGL — no `Application::init()/step()/close()`, no
`Plot::render()`, no `Visualization::render()/renderTile()`. No `#ifdef`, no
xvfb — the tests simply stay on the safe side of the API.

## Directly testable today (no refactor)

### `Visualization` plot management — `src/visualization/visualization.cpp`

The whole plot add/remove/data-source API is public, ImGui-free logic on a
`plots` map exposed via `getPlots()`. New file
`tests/visualization/test_visualization.cpp` (added to `tests/CMakeLists.txt`):

- [x] Constructor: null simulation → throws `Exception` with
  `ErrorCode::VIS_INVALID_SIM` (`visualization.cpp:7-17`); valid sim →
  `getSimulation()` returns it, `getPlots()` empty.
- [x] `plot(PlotType)` (`:19`) — adds a LinePlot / a Heatmap with no data
  sources; `getPlots()` grows; the new plot's `getType()` matches.
- [x] `plot(data)` / `plot(name, component)` (`:33/:39`) — adds a LinePlot
  holding exactly the given (element, component) pairs.
- [x] `plot(parameters, specificParameters, data)` (`:45-75`) — LINE_PLOT with
  `LinePlotParameters` honoured (thickness/autofit via `LinePlot` getters);
  HEATMAP with `HeatmapParameters` honoured (scale via `getScale()`);
  **mismatched** params (LINE_PLOT type + HeatmapParameters) → logged, plot NOT
  added (map size unchanged).
- [x] `plot(plotId, data)` (`:83-102`) — appends sources to an existing plot;
  unknown id → no-op (no throw, map unchanged).
- [x] `removePlot` (`:110-128`) — removes by id; unknown id → no-op.
- [x] `removeAllPlots` (`:130`) — empties the map.
- [x] `removePlottingDataFromPlot` (`:136-161`) — removes exactly the given
  pair; unknown id → no-op; pair not in plot → no-op (data intact).
- [x] Plot identity: `Plot::getUniqueIdentifier()` strictly increases across
  created plots (`plot.h:17-18`).

### Plot parameter structs — `include/visualization/plot_parameters.h`, `lineplot.h`, `heatmap.h`

- [x] `PlotDimensions::isLegal()` — default is legal; inverted ranges / zero
  steps are not.
- [x] `PlotDimensions`, `PlotAnnotations`, `PlotCommonParameters`,
  `LinePlotParameters`, `HeatmapParameters`: `operator==` (equal + unequal) and
  `toString()` non-empty / contains the salient values.
- [x] `LinePlot` getters (`setLineThickness`, `setAutoFit`) and `Heatmap`
  `setScale`/`getScale` round-trips — no rendering involved.

### `Application` — `include/application/application.h`

- [x] Already covered: static UI-scale accessors, mismatched sim/vis ctor throw
  (`test_application.cpp`).
- [ ] `addWindow` SFINAE traits (`application.h:22-37`,
  `has_simulation_constructor` / `has_visualization_constructor`) —
  compile-time `static_assert` tests: e.g.
  `static_assert(has_simulation_constructor<user_interface::FieldMetricsWindow>::value)`.
  Zero runtime cost; pins the trait logic that decides constructor forwarding.

## Testable only after extraction (document now, refactor later)

Pure logic currently trapped inside `render()` bodies, entangled with ImGui
calls and `static` locals. Each needs its predicate pulled into a free function
(e.g. into a small `user_interface/logic.h`) before it can be unit-tested:

- [ ] **Element search/filter predicate** — `src/user_interface/element_window.cpp:370-388`
  (lowercased substring match against element name OR category name; query
  lowercasing at `:225-228`). Extract
  `bool matchesElementFilter(const Element&, const std::string& lowercaseQuery)`.
  Same pattern appears three more times in `simulation_window.cpp`
  (`:1743-1778` remove-element, `:1833-1934` connections, `:2225-2274` log-params) —
  one extracted predicate serves all four call sites.
- [ ] **Quick-populate dedup** — `src/user_interface/plot_control_window.cpp:38-73`:
  builds an `alreadyPlotted` set and adds one plot per neural field not yet
  plotted. Extract the "which fields need plots" computation
  (`vector<string> fieldsNeedingPlots(sim, plots)`).
- [ ] **UI-scale ini parse/format** — lambdas inside
  `Application::registerSettingsHandler()` (`src/application/application.cpp:43-73`):
  `sscanf("UiScale=%f")` / `appendf` round-trip. Extract parse+format free
  functions; the ImGui handler then just forwards.
- [ ] **Node-graph pin-ID arithmetic** — `src/user_interface/node_graph_window.cpp:588-674`:
  pin-id ↔ element-index mapping (`pinId - startingOutputPinId`, bounds check
  against `getHighestElementIndex()`). Extract a `resolvePin(pinId) -> (elementIdx, kind)`
  helper and test the boundary cases (first pin, last valid, one-past).
- [ ] **`updateHeatmapDimensionHint`** — `src/visualization/visualization.cpp:166-189`:
  static free function, already pure (reads component sizes → `setDimensionHint`).
  Promote it to a declared function (e.g. in `visualization.h` or an internal
  header) and test: weights component present → hint = (input size, output size);
  no weights component → untouched; non-heatmap plot → untouched.
- [ ] **Stale-plot pruning** — the remove-plots-whose-components-vanished logic
  inside `Visualization::render()` (`visualization.cpp:230-243`). The same check
  exists in `renderTile()` (`:199-204`). Extract
  `bool plotDataStillExists(sim, data)` so pruning is testable without ImGui.

## Not worth GUI-layer tests

- `field_metrics_window.cpp` — computes nothing; it renders values from
  `NeuralField::getBumps()/isStable()/...`, which are core-tested (see
  [02-field-metrics.md](02-field-metrics.md)). Testing the card-layout math would
  test ImGui, not the project.
- Node-graph *connection/rename semantics* — these delegate to core
  (`Simulation::createInteraction`, `Simulation::renameElement`,
  `Element::removeInput`), covered in [04-simulation-and-exceptions.md](04-simulation-and-exceptions.md).
- `plots_window.cpp`, `control_bar_window`, `status_bar_window`, `main_menu_bar`,
  `log_window`, `help_window`, `static_layout`, `widgets`, `node_utilities/*`,
  `fonts/*`, `application/style.cpp` — pure rendering/glue.
