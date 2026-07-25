# Parameter Tuning Guide

This page gives practical guidance for tuning the parameters of the most commonly used elements. It complements the [Element Reference](Element-Reference), which lists every parameter, its type, and its default — here the focus is on **what happens when you turn each knob**, and how parameters interact to produce the three classic DNF behaviors: detection, working memory, and selection (see [Home](Home) for a one-line description of each).

All parameter names below are the actual member names of the `*Parameters` structs declared under `include/elements/` (e.g. `NeuralFieldParameters::tau`, `GaussKernelParameters::amplitude`) — the same names you'll see in the Element Inspector and in a saved `.dnf` file (see [.dnf File Schema](DNF-File-Schema)).

---

## NeuralField — the core element

`NeuralFieldParameters` (`include/elements/neural_field.h`): `tau`, `startingRestingLevel`, `activationFunction`.

The field follows Amari-type dynamics:

```
tau * du/dt = -u + h + s(x, t) + (w * f(u))(x)
```

where `u` is activation, `h` is `startingRestingLevel`, `s` is the summed external input, and `w * f(u)` is the convolved kernel output.

| Parameter | Effect of increasing it | Effect of decreasing it |
|---|---|---|
| `tau` (default `25.0`) | Slower relaxation — the field responds more sluggishly to input changes, and takes longer to converge or decay. Too small a `tau` relative to `deltaT` can make Euler integration unstable (oscillation or blow-up). | Faster response, but very small values combined with a large `deltaT` risk the same instability from the other direction. |
| `startingRestingLevel` (default `-5.0`) | Less negative (or positive) resting level means the field sits closer to threshold — weaker stimuli suffice to push it into a supra-threshold bump, and existing bumps decay more slowly. | More negative resting level requires stronger/longer stimulation to detect anything, and any bump collapses quickly once input is removed (this is what makes `detection_instability.cpp` a "detection" and not a "memory" example). |
| `activationFunction` | See below. | |

### Choosing an activation function

Three concrete types exist (`include/elements/activation_function.h`), all constructed as `Type(x_shift, steepness_param)`:

| Type | Constructor | Notes |
|---|---|---|
| `SigmoidFunction(x_shift, steepness)` | Logistic sigmoid, uses `std::exp` | Smooth, most common default (`SigmoidFunction(0.0, 10.0)`) |
| `AbsSigmoidFunction(x_shift, beta)` | Rational sigmoid, no `exp()` | Cheaper to evaluate; indistinguishable from `SigmoidFunction` once `beta >= 20` |
| `HeavisideFunction(x_shift)` | Hard binary threshold | Use when you want a strictly binary output component, e.g. for gating logic |

`x_shift` sets *where* the transition happens relative to activation; `steepness`/`beta` sets *how sharp* it is. A very steep sigmoid behaves like a Heaviside function but stays differentiable — useful if downstream code (e.g. a learning rule) needs a smooth signal.

---

## Lateral interaction kernels

Kernels are what turn a field from a passive relay into a dynamical system capable of sustained or competing bumps. All kernel `step()`s convolve the connected field's `"output"` with a spatial profile and add `amplitudeGlobal` as a spatially uniform offset.

### GaussKernel — single-peak excitation

`GaussKernelParameters` (`include/elements/gauss_kernel.h`): `width`, `amplitude`, `amplitudeGlobal`, `circular`, `normalized`.

- **`amplitude`** controls the strength of local (self-)excitation. Increase it to make a bump easier to sustain; too high, combined with a shallow `amplitudeGlobal`, can cause runaway activation across the whole field.
- **`width`** (σ) sets how far the excitation reaches. Narrow kernels produce sharp, localized bumps; wide kernels blur multiple nearby inputs into one bump.
- **`amplitudeGlobal`** is typically small and negative (default `-0.01`) — it's the *global* inhibition that keeps the rest of the field suppressed while a local bump is active. Without it (or if it's ≥ 0), activity tends to spread rather than stay localized.
- A `GaussKernel` alone (excitation + global inhibition, no separate short-range inhibition) is what `examples/detection_instability.cpp` and `examples/selection_instability.cpp` use — combined with a low `amplitude`/strong `amplitudeGlobal` ratio, it produces winner-take-all competition between stimuli rather than persistence.

### MexicanHatKernel — the classic bistable kernel

`MexicanHatKernelParameters` (`include/elements/mexican_hat_kernel.h`): `widthExc`, `amplitudeExc`, `widthInh`, `amplitudeInh`, `amplitudeGlobal`, `circular`, `normalized`.

This is the difference of a narrow excitatory Gaussian and a wider inhibitory Gaussian — short-range excitation, longer-range inhibition. This is the standard profile for **self-sustained** bumps (`examples/memory_instability.cpp`).

- The **ratio** of `amplitudeExc` to `amplitudeInh`/`widthInh` determines whether a bump can sustain itself after the driving stimulus is removed. If excitation dominates too strongly, activity can spread uncontrolled; if inhibition dominates, bumps collapse as soon as input stops (back to detection-like behavior).
- `widthExc` should be noticeably smaller than `widthInh` — that's what gives the "Mexican hat" (sombrero) shape. If you make them close in width the kernel degenerates toward a net-uniform effect.
- This is the kernel to reach for whenever you want **working memory** — a peak that outlives its input.

### OscillatoryKernel — traveling waves / rhythmic activity

`OscillatoryKernelParameters` (`include/elements/oscillatory_kernel.h`): `amplitude`, `decay` (must be `> 0`), `zeroCrossings` (clamped to `[0, 1]`), `amplitudeGlobal`, `circular`, `normalized`.

- **`zeroCrossings`** controls the spatial frequency of the damped cosine — higher values pack more oscillation cycles into the kernel's spatial extent, which tends to produce **multiple co-existing bumps** rather than one winner (see `examples/multi_peak.cpp`, which uses `OscillatoryKernelParameters{0.7, 0.08, 0.3, -0.01, true, false}`).
- **`decay`** controls how quickly the oscillation's envelope falls off with distance — larger decay confines the oscillatory structure to a narrower neighborhood (fewer effective side-lobes); it must stay positive or the kernel is meaningless.
- Combine with `NormalNoise` (see below) to let noise break symmetry between the potential peak locations.

### AsymmetricGaussKernel — directional bias / traveling bumps

`AsymmetricGaussKernelParameters` (`include/elements/asymmetric_gauss_kernel.h`): `width`, `amplitude`, `amplitudeGlobal`, `timeShift`, `circular`, `normalized`.

- **`timeShift`** is the knob unique to this kernel: it scales a derivative-of-Gaussian term added to the symmetric Gaussian, biasing lateral interaction toward higher (positive `timeShift`) or lower (negative) positions. Use it to make a bump drift across the field (see `examples/travelling_bump.cpp`).
- Keep `timeShift` small relative to `amplitude` at first — too large a value overwhelms the symmetric component and the "bump" degenerates into a pure traveling ripple rather than a localized peak that drifts.

---

## Stimuli

### GaussStimulus — static localized input

`GaussStimulusParameters` (`include/elements/gauss_stimulus.h`): `width`, `amplitude`, `position`, `circular`, `normalized`.

- **`position`** must satisfy `0 <= position < x_max` (the field's size) — the constructor throws `Exception(ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE, ...)` immediately if it doesn't (see [Troubleshooting](Troubleshooting)).
- **`amplitude`** vs. the field's `startingRestingLevel`: the stimulus needs to be strong enough that `restingLevel + stimulus_amplitude` (plus any kernel contribution) crosses the activation function's `x_shift`. If nothing happens when you add a stimulus, this is the first ratio to check.
- **`width`** trades off spatial precision against amplitude when `normalized = true` (a normalized Gaussian's peak height is `amplitude / (sum of the unnormalized profile)`, so wider + normalized means a lower peak for the same `amplitude`).

### TimedGaussStimulus — input that switches on and off

`TimedGaussStimulusParameters` (`include/elements/timed_gauss_stimulus.h`) adds `onTimes`, a `std::vector<std::pair<double,double>>` of `[tStart, tEnd]` windows (inclusive) during which the output is non-zero; it's otherwise identical to `GaussStimulus`. This is what every instability example (`detection_instability.cpp`, `memory_instability.cpp`, `selection_instability.cpp`) uses to present a pulse and then observe what the field does once it's gone — the field's own kernel/resting-level configuration determines whether the resulting behavior is detection or memory.

### BoostStimulus — uniform global offset

`BoostStimulusParameters` (`include/elements/boost_stimulus.h`): `amplitude`, `isActive`.

Unlike `GaussStimulus`, this raises (or lowers, with a negative `amplitude`) the *entire* field uniformly — useful for global gain control or task-condition gating. Toggle `isActive = false` to zero the output without removing the element or its connections; per the header, `setParameters()` re-runs `init()` so the change is visible immediately, not on the next `step()`.

---

## Noise

### NormalNoise — independent per-position noise

`NormalNoiseParameters` (`include/elements/normal_noise.h`): `amplitude` (standard deviation, default `0.2`).

Small amounts of noise (the built-in examples typically use `0.01`–`0.1`) are what let a **selection instability** actually pick a winner: without noise, perfectly symmetric competing stimuli would leave the field in an unstable equilibrium indefinitely.

### CorrelatedNormalNoise — spatially smooth noise

`CorrelatedNormalNoiseParameters` (`include/elements/correlated_normal_noise.h`): `amplitude` (default `0.05`), `width` (correlation length, default `1.0`), `circular`.

White noise convolved with a Gaussian of the given `width`, then scaled by `amplitude / sqrt(deltaT)` so the effective noise level stays consistent across different `deltaT` choices. Use this instead of `NormalNoise` when you need noise that's smooth over a few grid positions (e.g. to more realistically perturb bump position) rather than uncorrelated per-position jitter. For width → 0 (uncorrelated), prefer plain `NormalNoise`.

---

## MemoryTrace — second-layer persistent memory

`MemoryTraceParameters` (`include/elements/memory_trace.h`): `tauBuild` (default `100.0`), `tauDecay` (default `1000.0`), `threshold` (default `0.5`).

The trace builds slowly wherever the connected field's supra-threshold (`"output"`, i.e. post-sigmoid) signal exceeds `threshold`, and decays slowly everywhere else:

```
if input[i] > threshold:  output[i] += deltaT * (1/tauBuild)  * (-output[i] + input[i])
else:                      output[i] += deltaT * (1/tauDecay) * (-output[i])
```

- **`tauBuild` vs `tauDecay`**: `tauDecay` is normally an order of magnitude larger than `tauBuild` (100 vs 1000 by default) so the trace accumulates faster than it fades — this is what gives it a "memory" character across many stimulus presentations.
- Both should be **much larger** than the driving `NeuralField`'s `tau` (typically 20–100) so the trace evolves on a slower timescale than the field's own dynamics — otherwise the trace just tracks the field instantaneously and adds nothing.
- Feed the trace the field's `"output"` component, not `"activation"` — see [Element Reference → MemoryTrace](Element-Reference#memorytrace) for why (the threshold check assumes an already-thresholded signal).
- `setParameters()` does **not** reset the trace (unlike most other elements) — call `init()` explicitly if you want to zero it.

---

## FieldCoupling — learned weight matrices

`FieldCouplingParameters` (`include/elements/field_coupling.h`): `inputFieldDimensions`, `learningRule`, `scalar` (default `1.0`), `learningRate` (default `0.01`).

- **`learningRate`**: higher values adapt faster but are noisier and can overshoot; lower values are more stable but need more training steps to converge.
- **`learningRule`**: `LearningRule::HEBB` (unbounded Hebbian growth — weights can grow without limit, so pair with a modest `learningRate`), `LearningRule::OJA` (normalized Hebbian — self-limiting, generally the safer default for long training runs), `LearningRule::DELTA` (error-correcting toward a target — note the *unsupervised* delta variant currently just logs `"Unsupervised delta learning rule is not implemented yet."` and does nothing, so don't rely on it yet).
- **`scalar`** rescales the coupling's output after the weight matrix is applied — use it to balance the coupling's contribution against a field's other inputs without altering the learned weights themselves.

---

## Spatial dimensions (`ElementDimensions`)

`x_max` (field size) and `d_x` (step size) aren't tuning parameters in the dynamical sense, but they interact with several of the above:

- Kernel/stimulus `width` and `position` values are in the same units as `d_x * x_max` — doubling `d_x` while keeping `width` fixed effectively narrows the kernel relative to the field.
- Changing `x_max` on an element that already has inputs of a different size will not resize automatically for arbitrary reconnection — see `Resize`/`Resize2D` in [Element Reference](Element-Reference#resize) for bridging fields of different sizes, and [Troubleshooting](Troubleshooting) for the `ELEM_INPUT_SIZE_MISMATCH` / `ELEM_INVALID_SIZE` errors this can trigger.

---

## General workflow

1. Start from an existing example that already exhibits the behavior you want (`examples/detection_instability.cpp` → `examples/memory_instability.cpp` → `examples/selection_instability.cpp` for a good progression), rather than tuning from scratch.
2. Change one parameter at a time and watch the `"activation"` plot — most of the interesting behavior lives in a fairly narrow band between "nothing happens" and "activity spreads everywhere."
3. Use the **Element Inspector** for live tuning (`setParameters()` on most elements takes effect immediately or on the next `step()` — check the "Usage note" in [Element Reference](Element-Reference) for the specific element), then bake the final values back into your `.cpp` file or save the whole architecture as a `.dnf` file (see [.dnf File Schema](DNF-File-Schema)).
