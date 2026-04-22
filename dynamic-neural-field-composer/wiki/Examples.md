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

## ex_cross_dimension_kernels

**Source:** `examples/ex_cross_dimension_kernels.cpp`
**Executable:** `ex_cross_dimension_kernels`

Demonstrates kernels that couple two neural fields of different spatial dimensions. A cross-dimension kernel projects activity from a source field onto a target field of different size, enabling multi-scale or heterogeneous field architectures.

**Key concepts:** cross-dimension coupling, kernel resampling, multi-scale DNF architectures

---

## ex_boost_stimulus

**Source:** `examples/ex_boost_stimulus.cpp`
**Executable:** `ex_boost_stimulus`

Demonstrates the `BoostStimulus` element as a global gain control mechanism. The boost raises the entire field's resting level, pushing it closer to or above the detection threshold without providing any spatial information.

**Architecture:**
- One `NeuralField` with a `MexicanHatKernel` for lateral interactions
- One `GaussStimulus` providing a localized spatial input
- One `BoostStimulus` providing a spatially uniform activation boost
- `NormalNoise` for sub-threshold fluctuations

**Key concepts:** global input vs. spatial input, resting-level control, on/off gating with `isActive`, how a boost interacts with detection threshold

---

## ex_memory_trace

**Source:** `examples/ex_memory_trace.cpp`
**Executable:** `ex_memory_trace`

Demonstrates the `MemoryTrace` element as a working memory mechanism. A transient stimulus drives the neural field into a self-sustained peak; after the stimulus is removed, the memory trace retains an excitatory footprint that facilitates re-activation at the same location.

**Architecture:**
- One `NeuralField` with a `MexicanHatKernel` for lateral interactions and `NormalNoise`
- One `GaussStimulus` as a transient cue
- One `MemoryTrace` receiving the field's sigmoid output
- One `GaussKernel` projecting the memory trace back into the field as weak excitatory input

**Key concepts:** dual-timescale learning, working memory through trace feedback, `tauBuild` / `tauDecay` parameter tuning, history-dependent field dynamics

---

## Common pattern

Every example follows this structure:

```cpp
// 1. Simulation + visualization + app
auto simulation    = std::make_shared<Simulation>("name", deltaT);
auto visualization = std::make_shared<Visualization>(simulation);
Application app{ simulation, visualization };

// 2. Windows
app.addWindow<user_interface::MainMenuBar>();
app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

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