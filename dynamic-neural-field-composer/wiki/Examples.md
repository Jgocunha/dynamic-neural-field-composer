# Examples

All examples are in the [`examples/`](../examples/) directory and are built as standalone executables. Each follows the same pattern: create a simulation, build elements, wire interactions, set up plots, and run the application loop.

---

## ex_field_couplings

**Source:** `examples/ex_field_couplings.cpp`
**Executable:** `ex_field_couplings`

Demonstrates field-to-field coupling across three neural fields representing different temporal contexts: *past*, *present*, and *next*.

**Architecture:**
- Three `NeuralField` elements, each with its own lateral interaction kernel and Gaussian stimulus
- Two `FieldCoupling` elements bridging past→present and present→next
- Heatmap plots for both coupling weight matrices

**Key concepts:** multi-field architectures, field coupling topology, heatmap visualization of weight matrices

---

## ex_field_coupling_learning

**Source:** `examples/ex_field_coupling_learning.cpp`
**Executable:** `ex_field_coupling_learning`

Shows how `FieldCoupling` weights evolve online through co-activation of a source and target field.

**Key concepts:** Hebbian learning, enabling/disabling learning at runtime (`setLearning(true)`), watching weight matrices form over time

---

## ex_gauss_and_field_couplings

**Source:** `examples/ex_gauss_and_field_couplings.cpp`
**Executable:** `ex_gauss_and_field_couplings`

Combines `GaussFieldCoupling` (fixed Gaussian basis) and `FieldCoupling` (learned) in the same architecture.

**Key concepts:** comparing fixed vs. learnable coupling profiles

---

## ex_complementary_action_selection

**Source:** `examples/ex_complementary_action_selection.cpp`
**Executable:** `ex_comp_act_selection`

A classic DNF application: two competing neural fields that implement a winner-take-all (WTA) mechanism. When one field forms a stable bump, it suppresses the other through inhibitory coupling.

**Key concepts:** competitive dynamics, inhibitory field coupling, action selection via DNF

---

## ex_asymmetric_gauss_kernel

**Source:** `examples/ex_asymmetric_gauss_kernel.cpp`
**Executable:** `ex_asymmetric_gauss_kernel`

Demonstrates the `AsymmetricGaussKernel` and its effect on bump dynamics. The asymmetric component (controlled by `timeShift`) introduces directional drift in the activity bump.

**Key concepts:** asymmetric lateral interactions, bump propagation, `timeShift` parameter tuning

---

## ex_two_robot_team

**Source:** `examples/ex_two_robot_team.cpp`
**Executable:** `ex_two_robot_team`

A larger architecture modelling coordination between two robotic agents using DNF. Fields represent spatial locations and task-relevant dimensions; couplings encode the joint-task constraints.

**Key concepts:** multi-agent DNF architectures, spatial encoding, coupling-based coordination

---

## ex_grand_architecture

**Source:** `examples/ex_grand_architecture.cpp`
**Executable:** `ex_grand_architecture`

A large, comprehensive DNF architecture combining many element types across multiple coupled fields. Useful as a stress test and as a template for complex real-world architectures.

**Key concepts:** scaling to many elements, organizing large architectures, performance

---

## ex_packaging_task

**Source:** `examples/ex_packaging_task.cpp`
**Executable:** `ex_packaging_task`

A task-specific simulation modelling a packaging or manipulation scenario using DNF. Fields represent object properties and spatial locations; couplings encode task constraints between them.

**Key concepts:** task-oriented DNF design, domain-specific field architecture

---

## ex_simulation_file_manager

**Source:** `examples/ex_simulation_file_manager.cpp`
**Executable:** `ex_simulation_file_manager`

Demonstrates every way to persist and restore a simulation. The example runs two phases before opening the GUI:

**Phase 1 — explicit `SimulationFileManager` API**
1. A small architecture (neural field + Mexican-hat kernel + Gaussian stimulus + noise) is built programmatically.
2. `SimulationFileManager::saveElementsToJson()` writes it to `data/simulations/single-field-demo.json`.
3. `SimulationFileManager::loadElementsFromJson()` reads the file into a fresh simulation.

**Phase 2 — `Simulation` convenience methods**
4. `sim->save(dir)` re-saves the same architecture (equivalent to the SFM call, but without needing to instantiate `SimulationFileManager` directly).
5. `sim->read(filePath)` loads it back in one call — internally it clears the simulation, calls `loadElementsFromJson`, and then calls `init()`.
6. The bundled `and-test.json` architecture is loaded via `sim->read()` and run interactively with three activation plots.

**Key concepts:** `SimulationFileManager` explicit API, `Simulation::save` / `Simulation::read` convenience methods, JSON-based persistence, loading pre-built architectures from file

---

## Common pattern

Every example follows this structure:

```cpp
// 1. Simulation + visualization + app
auto simulation    = std::make_shared<Simulation>("name", deltaT);
auto visualization = std::make_shared<Visualization>(simulation);
Application app{ simulation, visualization };

// 2. Windows
app.addWindow<user_interface::MainWindow>();
// ... other windows

// 3. Elements
ElementFactory factory;
auto field  = factory.createElement(NEURAL_FIELD, commonParams, fieldParams);
auto kernel = factory.createElement(MEXICAN_HAT_KERNEL, commonParams, kernelParams);
// ...

// 4. Register
simulation->addElement(field);
simulation->addElement(kernel);

// 5. Wire
field->addInput(kernel);
kernel->addInput(field);

// 6. Plots
visualization->plot(commonPlotParams, LinePlotParameters{},
    { { field->getUniqueName(), "activation" } });

// 7. Loop
app.init();
while (!app.hasGUIBeenClosed())
    app.step();
app.close();
```