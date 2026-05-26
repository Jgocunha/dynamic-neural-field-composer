# Examples

All examples are in the [`examples/`](../examples/) directory and are built as standalone executables. Each follows the same pattern: create a simulation, build elements, wire interactions, set up plots, and run the application loop.

---

## Instability examples

### detection_instability

**Source:** `examples/detection_instability.cpp`
**Executable:** `example_detection_instability`

Demonstrates the **detection instability**: a localized stimulus drives the neural field above threshold, but once the stimulus is removed the field returns to its resting level. No self-sustained peak forms.

**Architecture:**
- One `TimedGaussStimulus` — active 0–500 ms (sigma 3.0, amplitude 5.0, position 50)
- One `NeuralField` with a sub-critical `GaussKernel` (negative global inhibition) and `NormalNoise`

**Key concepts:** detection threshold, transient response, sub-critical lateral interactions, `TimedGaussStimulus`

---

### detection_instability_2d

**Source:** `examples/detection_instability_2d.cpp`
**Executable:** `example_detection_instability_2d`

2D version of the detection instability example on a 100×100 field.

**Architecture:**
- One `TimedGaussStimulus2D` centered at (50, 50) — active 0–500 ms
- One `NeuralField2D` with `GaussKernel2D` (sub-critical) and `NormalNoise2D`
- Heatmap plots for field activation and stimulus output

**Key concepts:** 2D detection dynamics, transient spatial activation

---

### memory_instability

**Source:** `examples/memory_instability.cpp`
**Executable:** `example_memory_instability`

Demonstrates the **memory instability**: the same timed stimulus drives a peak, but after it switches off the peak **persists** — a self-sustained working-memory representation.

**Architecture:**
- One `TimedGaussStimulus` — active 0–500 ms (same timing as detection example)
- One `NeuralField` with a `MexicanHatKernel` (enables self-sustaining recurrent dynamics) and `NormalNoise`

**Key concepts:** working memory, self-sustaining peaks, `MexicanHatKernel` bistability, contrast with detection instability

---

### memory_instability_2d

**Source:** `examples/memory_instability_2d.cpp`
**Executable:** `example_memory_instability_2d`

2D version of the memory instability example on a 50×50 field.

**Architecture:**
- One `TimedGaussStimulus2D` centered at (25, 25) — active 0–500 ms
- One `NeuralField2D` with `MexicanHatKernel2D` and `NormalNoise2D`
- Heatmap plots for field activation and stimulus output

**Key concepts:** 2D working memory, persistent spatial representation

---

### selection_instability

**Source:** `examples/selection_instability.cpp`
**Executable:** `example_selection_instability`

Demonstrates the **selection instability**: three equal-amplitude stimuli drive the field simultaneously. Lateral inhibition prevents co-existing peaks — small noise fluctuations break the symmetry and a single winner is selected.

**Architecture:**
- Three `GaussStimulus` elements at positions 25, 50, and 75
- One `NeuralField` with a `GaussKernel` (strong lateral inhibition) and `NormalNoise`
- Line plots for field activation/output/input and all stimulus outputs

**Key concepts:** competitive dynamics, winner-take-all selection, symmetry breaking via noise, lateral inhibition

---

### selection_instability_2d

**Source:** `examples/selection_instability_2d.cpp`
**Executable:** `example_selection_instability_2d`

2D version of the selection instability example on a 100×100 field.

**Architecture:**
- Three `GaussStimulus2D` elements at (25, 25), (50, 50), and (75, 75)
- One `NeuralField2D` with `GaussKernel2D` and `NormalNoise2D`
- Heatmap plots for field activation and each stimulus

**Key concepts:** 2D winner-take-all selection, spatial competition

---

## Boost examples

### boost_detection

**Source:** `examples/boost_detection.cpp`
**Executable:** `example_boost_detection`

Demonstrates how a spatially uniform `BoostStimulus` can raise the field's global operating point, increasing its sensitivity to a localized input stimulus.

**Architecture:**
- One `BoostStimulus` providing spatially uniform drive
- One `GaussStimulus` providing a localized spatial cue
- One `NeuralField` with a `MexicanHatKernel` and `CorrelatedNormalNoise`

**Key concepts:** global vs. spatial input, detection threshold control, `BoostStimulus`, sensitivity modulation

---

### boost_detection_2d

**Source:** `examples/boost_detection_2d.cpp`
**Executable:** `example_boost_detection_2d`

2D version of the boost detection example on a 50×50 field with temporally gated stimuli.

**Architecture:**
- One `BoostStimulus2D` (uniform drive)
- One `TimedGaussStimulus2D` active during 0–500 ms and 800–1200 ms
- One `NeuralField2D` with `GaussKernel2D`
- Heatmap plots for field activation, boost, and timed stimulus

**Key concepts:** 2D boost dynamics, temporally gated stimuli, repeated detection

---

## Memory examples

### memory_trace

**Source:** `examples/memory_trace.cpp`
**Executable:** `example_memory_trace`

Demonstrates the `MemoryTrace` element as a working-memory mechanism. A timed stimulus drives a self-sustained peak; the memory trace captures the field's sigmoid output and feeds it back via a slow `GaussKernel`, maintaining an excitatory footprint that facilitates re-activation at the same location.

**Architecture:**
- One `TimedGaussStimulus` — active 0–500 ms (sigma 12.0, amplitude 5.0, position 50)
- One `NeuralField` with a `MexicanHatKernel` and `NormalNoise`
- One `MemoryTrace` monitoring the field's output, projecting back via a `GaussKernel`

**Key concepts:** dual-timescale dynamics, `MemoryTrace`, `tauBuild` / `tauDecay` tuning, history-dependent field state

---

### memory_trace_2d

**Source:** `examples/memory_trace_2d.cpp`
**Executable:** `example_memory_trace_2d`

2D version of the memory trace example on a 50×50 field.

**Architecture:**
- One `TimedGaussStimulus2D` centered at (12, 12) — active 0–500 ms
- One `NeuralField2D` with `MexicanHatKernel2D` and `NormalNoise2D`
- One `MemoryTrace2D` feeding back via `GaussKernel2D`
- Heatmap plots for field activation, stimulus, and memory trace output

**Key concepts:** 2D working memory via trace feedback, spatial history encoding

---

## Learning examples

### hebbian_learning

**Source:** `examples/hebbian_learning.cpp`
**Executable:** `example_hebbian_learning`

Demonstrates online Hebbian (multiplicative) learning between two neural fields via a `FieldCoupling` element. Both fields are driven simultaneously to co-activate patterns; after a fixed number of iterations the learning is disabled and the association is read out.

**Architecture:**
- Source field (200 units): `NeuralField` + `MexicanHatKernel` + `NormalNoise` + `GaussStimulus`
- Output field (400 units): `NeuralField` + `GaussKernel` + `NormalNoise` + two `GaussStimulus` elements
- One `FieldCoupling` (200→400) with Hebbian learning rule
- Heatmap of the 400×200 learned weight matrix

**Key concepts:** Hebbian learning, `FieldCoupling`, online weight formation, enabling/disabling learning at runtime

---

## Movement examples

### travelling_bump

**Source:** `examples/travelling_bump.cpp`
**Executable:** `example_travelling_bump`

Demonstrates a traveling bump: the `AsymmetricGaussKernel` introduces a directional bias into the lateral interactions, causing a stable activity peak to drift continuously across the field after it is initialized by a transient stimulus.

**Architecture:**
- One `TimedGaussStimulus` — active 0–500 ms (sigma 15.0, amplitude 5.0, position 50)
- One `NeuralField` with an `AsymmetricGaussKernel` (asymmetry 1.0) and `NormalNoise`
- Line plots for field activation, kernel shape, and kernel output

**Key concepts:** asymmetric lateral interactions, bump propagation, movement generation, `timeShift` parameter

---

### travelling_bump_2d

**Source:** `examples/travelling_bump_2d.cpp`
**Executable:** `example_travelling_bump_2d`

2D version of the travelling bump example on a 50×50 field.

**Architecture:**
- One `TimedGaussStimulus2D` centered at (25, 25) — active 0–1000 ms
- One `NeuralField2D` with an `AsymmetricGaussKernel2D` and `NormalNoise2D`
- Heatmap plots for field activation, kernel shape, and kernel output

**Key concepts:** 2D traveling bump, directional wave propagation

---

## Multi-peak examples

### multi_peak

**Source:** `examples/multi_peak.cpp`
**Executable:** `example_multi_peak`

Demonstrates that multiple stable activity peaks can co-exist in the same field. The `OscillatoryKernel` (damped-cosine profile) provides local excitation flanked by inhibitory rings, allowing several bumps to stabilize simultaneously when driven by spatially separated stimuli.

**Architecture:**
- Three `GaussStimulus` elements at positions 25, 50, and 75
- One `NeuralField` with an `OscillatoryKernel` and `NormalNoise`
- Line plots for field activation/output/input and stimulus outputs

**Key concepts:** oscillatory kernel, multi-stability, co-existing activity peaks, damped-cosine lateral interactions

---

### multi_peak_2d

**Source:** `examples/multi_peak_2d.cpp`
**Executable:** `example_multi_peak_2d`

2D version of the multi-peak example on a 50×50 field.

**Architecture:**
- Three `GaussStimulus2D` elements at (15, 15), (35, 35), and (15, 35)
- One `NeuralField2D` with an `OscillatoryKernel2D` (normalized, circular) and `NormalNoise2D`
- Heatmap plots for field activation and each stimulus

**Key concepts:** 2D multi-peak dynamics, spatial multi-stability

---

## Field coupling examples

### weighted_field_coupling

**Source:** `examples/weighted_field_coupling.cpp`
**Executable:** `example_weighted_field_coupling`

Demonstrates associative sequence memory across three coupled neural fields representing temporal contexts: *past*, *present*, and *next*. Two `FieldCoupling` elements bridge adjacent fields; their weight matrices encode the learned spatial associations.

**Architecture:**
- Three `NeuralField` elements, each with its own lateral interaction kernel, `GaussStimulus`, and `NormalNoise`
- Two `FieldCoupling` elements: past→present and present→next
- Heatmap plots of both weight matrices

**Key concepts:** multi-field architectures, sequence memory, `FieldCoupling` topology, weight heatmaps

---

### gaussian_field_coupling

**Source:** `examples/gaussian_field_coupling.cpp`
**Executable:** `example_gaussian_field_coupling`

Demonstrates `GaussFieldCoupling` — a fixed (non-learnable) coupling using a Gaussian basis — to project activity from a larger input field onto a smaller output field with different spatial resolution.

**Architecture:**
- Input field (200 pts, dx=0.5): `NeuralField` + `GaussKernel` + `GaussStimulus` + `NormalNoise`
- Output field (100 pts, dx=1.0): `NeuralField` + `MexicanHatKernel` + `NormalNoise`
- One `GaussFieldCoupling` with 3 fixed Gaussian connections: input positions {25, 50, 75} → output positions {25, 50, 75}, sigma 5.0
- Plots: input field line plot, output field line plot, coupling weight heatmap, coupling output line plot

**Key concepts:** fixed-basis inter-field projection, `GaussFieldCoupling`, heterogeneous field sizes and spacings

> **Note:** The CMakeLists entry for this example is commented out by default. Uncomment `example_gaussian_field_coupling` in `examples/CMakeLists.txt` to build it.

---

## Common pattern

Every example follows this structure:

```cpp
// 1. Simulation + visualization + app
auto sim = std::make_shared<Simulation>("name", deltaT);
auto viz = std::make_shared<Visualization>(sim);
Application app{ sim, viz };

// 2. Windows
app.addWindow<user_interface::MainMenuBar>();
app.addWindow<user_interface::StaticLayoutWindow>(sim, viz);

// 3. Elements (direct construction — no factory)
const element::ElementDimensions dims{ 100, 1.0 };

const auto nf_cp = element::ElementCommonParameters{ "field", dims };
const auto nf    = std::make_shared<element::NeuralField>(nf_cp, element::NeuralFieldParameters{});

const auto gk_cp = element::ElementCommonParameters{ "kernel", dims };
const auto gk    = std::make_shared<element::GaussKernel>(gk_cp, element::GaussKernelParameters{});

// 4. Register
sim->addElement(nf);
sim->addElement(gk);

// 5. Wire
nf->addInput(gk);
gk->addInput(nf);

// 6. Plots
viz->plot({ { nf->getUniqueName(), "activation" } });

// 7. Loop
app.init();
while (!app.hasGUIBeenClosed())
    app.step();
app.close();
```
