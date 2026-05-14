# Element Reference

All elements are created through `ElementFactory::createElement(label, commonParams, specificParams)`. See the [Elements](Elements) page for the factory and common parameter details.

---

## NeuralField

The core DNF element. Integrates its inputs over continuous time using a first-order differential equation governed by a time constant `tau`. The output is the activation passed through a non-linear activation function.

**Label:** `NEURAL_FIELD`

### Parameters

```cpp
NeuralFieldParameters{
    double tau,                              // time constant (default: 25.0)
    double startingRestingLevel,             // resting activation level (default: -5.0)
    const ActivationFunction& activationFunction  // transfer function (default: SigmoidFunction(0, 10))
}
```

| Parameter | Type | Default | Description |
|---|---|---|---|
| `tau` | `double` | `25.0` | Time constant — larger values slow field dynamics |
| `startingRestingLevel` | `double` | `-5.0` | Activation level when no input is present |
| `activationFunction` | `ActivationFunction` | `SigmoidFunction(0, 10)` | Non-linear transfer function applied to compute output |

### Activation functions

| Type | Parameters | Description |
|---|---|---|
| `SigmoidFunction(x_shift, steepness)` | `x_shift=0.0`, `steepness=10.0` | Smooth sigmoidal transfer function |
| `HeavisideFunction(x_shift)` | `x_shift=0.0` | Binary threshold function |

### Components

| Name | Description |
|---|---|
| `"activation"` | Current membrane activation level at each position |
| `"output"` | Activation passed through the activation function |
| `"input"` | Summed input from all connected elements |

### State & metrics

```cpp
// Bump detection
std::vector<NeuralFieldBump> bumps = field->getBumps();

// Per-bump data
bump.centroid;        // center position of the bump
bump.amplitude;       // peak activation
bump.width;           // spatial extent
bump.startPosition;   // left edge
bump.endPosition;     // right edge
bump.velocity;        // centroid change rate
bump.acceleration;    // velocity change rate

// Field-level state
bool stable = field->isStable();
double lo   = field->getLowestActivation();
double hi   = field->getHighestActivation();
double thr  = field->getStabilityThreshold();

field->setThresholdForStability(0.9);
```

---

## GaussKernel

A Gaussian lateral interaction kernel. Typically provides local excitation (positive `amplitude`) and a uniform global inhibition (`amplitudeGlobal`).

**Label:** `GAUSS_KERNEL`

### Parameters

```cpp
GaussKernelParameters{
    double width           = 3.0,
    double amplitude       = 3.0,
    double amplitudeGlobal = -0.01,
    bool   circular        = true,
    bool   normalized      = true
}
```

| Parameter | Default | Description |
|---|---|---|
| `width` | `3.0` | Width (sigma) of the Gaussian excitatory peak |
| `amplitude` | `3.0` | Amplitude of the local excitation |
| `amplitudeGlobal` | `-0.01` | Uniform global inhibition added to every position |
| `circular` | `true` | Whether the field wraps around at the boundaries |
| `normalized` | `true` | Whether the Gaussian is area-normalized |

### Components

| Name | Description |
|---|---|
| `"output"` | Convolved lateral interaction signal |
| `"input"` | Raw input from connected field |

---

## MexicanHatKernel

A Mexican Hat (difference-of-Gaussians) kernel providing short-range excitation and long-range inhibition. This is the classic kernel for producing stable localized activity bumps.

**Label:** `MEXICAN_HAT_KERNEL`

### Parameters

```cpp
MexicanHatKernelParameters{
    double widthExc        = 2.5,
    double amplitudeExc    = 11.0,
    double widthInh        = 5.0,
    double amplitudeInh    = 15.0,
    double amplitudeGlobal = -0.1,
    bool   circular        = true,
    bool   normalized      = true
}
```

| Parameter | Default | Description |
|---|---|---|
| `widthExc` | `2.5` | Width of the excitatory Gaussian |
| `amplitudeExc` | `11.0` | Amplitude of excitation |
| `widthInh` | `5.0` | Width of the inhibitory Gaussian |
| `amplitudeInh` | `15.0` | Amplitude of inhibition |
| `amplitudeGlobal` | `-0.1` | Uniform global inhibition |
| `circular` | `true` | Boundary wrapping |
| `normalized` | `true` | Area normalization |

### Components

| Name | Description |
|---|---|
| `"output"` | Convolved lateral interaction signal |
| `"input"` | Raw input from connected field |

---

## OscillatoryKernel

A kernel with an oscillatory (damped cosine) lateral interaction pattern. Useful for modelling travelling waves or rhythmic activity.

**Label:** `OSCILLATORY_KERNEL`

### Parameters

```cpp
OscillatoryKernelParameters{
    double amplitude       = 1.0,
    double decay           = 0.08,    // must be > 0
    double zeroCrossings   = 0.3,     // clamped to [0, 1]
    double amplitudeGlobal = -0.01,
    bool   circular        = true,
    bool   normalized      = false
}
```

| Parameter | Default | Description |
|---|---|---|
| `amplitude` | `1.0` | Peak amplitude of the oscillatory kernel |
| `decay` | `0.08` | Exponential decay rate (must be positive) |
| `zeroCrossings` | `0.3` | Frequency of oscillations — controls how many zero crossings occur (clamped to [0, 1]) |
| `amplitudeGlobal` | `-0.01` | Uniform global inhibition |
| `circular` | `true` | Boundary wrapping |
| `normalized` | `false` | Area normalization |

### Components

| Name | Description |
|---|---|
| `"output"` | Convolved oscillatory signal |
| `"input"` | Raw input from connected field |

---

## AsymmetricGaussKernel

A Gaussian kernel with an asymmetric component, combining a standard Gaussian and its derivative. The asymmetry introduces a directional bias in lateral interactions, enabling travelling-wave-like dynamics.

**Label:** `ASYMMETRIC_GAUSS_KERNEL`

### Parameters

```cpp
AsymmetricGaussKernelParameters{
    double width           = 3.0,
    double amplitude       = 3.0,
    double amplitudeGlobal = 0.0,
    double timeShift       = 0.0,
    bool   circular        = true,
    bool   normalized      = true
}
```

| Parameter | Default | Description |
|---|---|---|
| `width` | `3.0` | Width (sigma) of the underlying Gaussian |
| `amplitude` | `3.0` | Amplitude of the symmetric component |
| `amplitudeGlobal` | `0.0` | Uniform global inhibition |
| `timeShift` | `0.0` | Controls the strength of the asymmetric (derivative) component |
| `circular` | `true` | Boundary wrapping |
| `normalized` | `true` | Area normalization |

### Components

| Name | Description |
|---|---|
| `"output"` | Convolved asymmetric signal |
| `"input"` | Raw input from connected field |

---

## GaussStimulus

A static external input with a Gaussian spatial profile. Provides a fixed driving input at a specified position in the field.

**Label:** `GAUSS_STIMULUS`

### Parameters

```cpp
GaussStimulusParameters{
    double width      = 5.0,
    double amplitude  = 15.0,
    double position   = 50.0,
    bool   circular   = true,
    bool   normalized = false
}
```

| Parameter | Default | Description |
|---|---|---|
| `width` | `5.0` | Width (sigma) of the Gaussian |
| `amplitude` | `15.0` | Peak amplitude of the stimulus |
| `position` | `50.0` | Center position of the Gaussian in field coordinates |
| `circular` | `true` | Boundary wrapping |
| `normalized` | `false` | Area normalization |

### Components

| Name | Description |
|---|---|
| `"output"` | Gaussian stimulus profile (same every time step) |

---

## TimedGaussStimulus

A Gaussian stimulus that is active only during specified time windows. The spatial pattern is pre-computed in `init()` and written to output only while the simulation time falls within one of the `[tStart, tEnd]` intervals.

**Label:** `TIMED_GAUSS_STIMULUS`

### Parameters

```cpp
TimedGaussStimulusParameters{
    double width      = 5.0,
    double amplitude  = 15.0,
    double position   = 50.0,
    std::vector<std::pair<double,double>> onTimes = {},
    bool   circular   = true,
    bool   normalized = false
}
```

| Parameter | Default | Description |
|---|---|---|
| `width` | `5.0` | Width (sigma) of the Gaussian |
| `amplitude` | `15.0` | Peak amplitude when active |
| `position` | `50.0` | Center position of the Gaussian |
| `onTimes` | `{}` | List of `[tStart, tEnd]` intervals (inclusive) during which output is non-zero |
| `circular` | `true` | Boundary wrapping |
| `normalized` | `false` | Area normalization |

### Components

| Name | Description |
|---|---|
| `"output"` | Gaussian profile when active, zero otherwise |

---

## TimedGaussStimulus2D

2D variant of `TimedGaussStimulus`. The 2D Gaussian pattern is pre-computed and gated by the same `onTimes` mechanism.

**Label:** `TIMED_GAUSS_STIMULUS_2D`

### Parameters

```cpp
TimedGaussStimulus2DParameters{
    double width      = 5.0,
    double amplitude  = 15.0,
    double position_x = 25.0,
    double position_y = 25.0,
    std::vector<std::pair<double,double>> onTimes = {},
    bool   circular   = true,
    bool   normalized = false
}
```

| Parameter | Default | Description |
|---|---|---|
| `width` | `5.0` | Isotropic width (sigma) of the 2D Gaussian |
| `amplitude` | `15.0` | Peak amplitude when active |
| `position_x` | `25.0` | x-center of the Gaussian |
| `position_y` | `25.0` | y-center of the Gaussian |
| `onTimes` | `{}` | List of `[tStart, tEnd]` intervals during which output is non-zero |
| `circular` | `true` | Periodic boundary wrapping |
| `normalized` | `false` | Area normalization |

### Components

| Name | Description |
|---|---|
| `"output"` | Flat `size_x * size_y` 2D Gaussian profile when active, zero otherwise |

---

## NormalNoise

Adds zero-mean Gaussian white noise to the field at every time step.

**Label:** `NORMAL_NOISE`

### Parameters

```cpp
NormalNoiseParameters{
    double amplitude = 0.2
}
```

| Parameter | Default | Description |
|---|---|---|
| `amplitude` | `0.2` | Standard deviation of the noise distribution |

### Components

| Name | Description |
|---|---|
| `"output"` | Random noise vector, resampled every time step |

---

## FieldCoupling

A full weight-matrix coupling between two fields of potentially different sizes. The weight matrix is initialized to zero and updated online using one of three learning rules. Weights can be saved to and loaded from disk.

**Label:** `FIELD_COUPLING`

### Parameters

```cpp
FieldCouplingParameters{
    ElementDimensions inputFieldDimensions,          // dimensions of the source field
    LearningRule      learningRule  = LearningRule::HEBB,
    double            scalar        = 1.0,
    double            learningRate  = 0.01
}
```

| Parameter | Default | Description |
|---|---|---|
| `inputFieldDimensions` | — | Spatial dimensions of the source field |
| `learningRule` | `HEBB` | Learning rule used to update the weight matrix |
| `scalar` | `1.0` | Multiplicative scaling of the coupling output |
| `learningRate` | `0.01` | Step size for weight updates |

### Learning rules

| Rule | Enum | Description |
|---|---|---|
| Hebbian | `LearningRule::HEBB` | Weights grow when pre- and post-synaptic activity co-occur |
| Oja's rule | `LearningRule::OJA` | Normalized Hebbian — prevents unbounded weight growth |
| Delta rule | `LearningRule::DELTA` | Error-correcting — drives weights toward a target |

### Runtime control

```cpp
coupling->setLearning(true);              // enable / disable weight updates
coupling->setLearningRate(0.005);         // change learning rate on the fly
coupling->setParameters(newParams);

coupling->writeWeights();                 // save weights to disk
coupling->readWeights();                  // load weights from disk
coupling->clearWeights();                 // reset weight matrix to zero

coupling->setWeightsDirectory("path/");
std::string dir = coupling->getWeightsDirectory();
```

### Components

| Name | Description |
|---|---|
| `"weights"` | Flattened 2D weight matrix (rows = output positions, cols = input positions) |
| `"output"` | Weighted sum of the source field output |

---

## GaussFieldCoupling

A coupling between two fields using a fixed set of Gaussian basis functions. Unlike `FieldCoupling`, the coupling profile is specified analytically and does not learn.

**Label:** `GAUSS_FIELD_COUPLING`

### Parameters

```cpp
GaussFieldCouplingParameters{
    ElementDimensions inputFieldDimensions,
    bool normalized = true,
    bool circular   = false,
    std::vector<GaussCoupling> couplings = {}
}
```

Each `GaussCoupling` defines one Gaussian link between positions in the two fields:

```cpp
GaussCoupling{
    double x_i,        // position in the input field
    double x_j,        // position in the output field
    double amplitude,  // strength of the coupling
    double width       // width of the Gaussian
}
```

```cpp
GaussFieldCouplingParameters params{ inputDims };
params.addCoupling(GaussCoupling{ 50.0, 50.0, 1.0, 5.0 });
params.addCoupling(GaussCoupling{ 80.0, 80.0, 1.0, 5.0 });
```

### Components

| Name | Description |
|---|---|
| `"weights"` | Flattened coupling weight matrix |
| `"output"` | Weighted projection from the source field |

---

## BoostStimulus

A spatially uniform external input — the same constant value is added to every position of a connected field. Unlike `GaussStimulus` (which is spatially localized), `BoostStimulus` raises or lowers the entire field's activation level, making it useful for global gain control, resting-level shifts, or task-condition gating.

**Label:** `BOOST_STIMULUS`

### Parameters

```cpp
BoostStimulusParameters{
    double amplitude = 5.0,
    bool   isActive  = true
}
```

| Parameter | Default | Description |
|---|---|---|
| `amplitude` | `5.0` | Constant value broadcast to all field positions |
| `isActive` | `true` | When `false` the output is zeroed — acts as an on/off gate |

### Components

| Name | Description |
|---|---|
| `"output"` | Uniform vector of `amplitude` (or zeros when inactive) |

### Usage note

`setParameters()` calls `init()` internally, so the output vector is updated immediately — no need to wait for the next `step()` call.

---

## BoostStimulus2D

2D variant of `BoostStimulus`. Fills every position of a 2D output with a constant amplitude value.

**Label:** `BOOST_STIMULUS_2D`

### Parameters

```cpp
BoostStimulus2DParameters{
    double amplitude = 5.0,
    bool   isActive  = true
}
```

| Parameter | Default | Description |
|---|---|---|
| `amplitude` | `5.0` | Constant value broadcast to all 2D field positions |
| `isActive` | `true` | When `false` the output is zeroed |

### Components

| Name | Description |
|---|---|
| `"output"` | Flat `size_x * size_y` vector of `amplitude` (or zeros when inactive) |

---

## FieldProjection

Projects a 2D field onto one spatial dimension by summing over the other axis. Corresponds to `SumDimension` (cosivina) and `Projection` (cedar). Produces a 1D output from a 2D input.

**Label:** `FIELD_PROJECTION`

### Parameters

```cpp
FieldProjectionParameters{
    int projectionAxis = 0   // 0 = sum over Y, output varies along X
                             // 1 = sum over X, output varies along Y
}
```

| Parameter | Default | Description |
|---|---|---|
| `projectionAxis` | `0` | Axis to project onto: 0 → output is size_x; 1 → output is size_y |

### Components

| Name | Description |
|---|---|
| `"input"` | Flat `size_x * size_y` vector — receives the 2D field output |
| `"output"` | 1D vector of length `size_x` (axis 0) or `size_y` (axis 1) |

---

## FieldExpansion

Lifts a 1D field into a 2D field by repeating values along the added dimension. Corresponds to `ExpandDimension2D` (cosivina) and `Projection` (cedar). Produces a 2D output from a 1D input.

**Label:** `FIELD_EXPANSION`

### Parameters

```cpp
FieldExpansionParameters{
    int expansionAxis = 0   // 0 = input varies along X (repeated along Y)
                            // 1 = input varies along Y (repeated along X)
}
```

| Parameter | Default | Description |
|---|---|---|
| `expansionAxis` | `0` | Source axis of the 1D input: 0 → input size is size_x; 1 → input size is size_y |

### Components

| Name | Description |
|---|---|
| `"input"` | 1D vector of length `size_x` (axis 0) or `size_y` (axis 1) |
| `"output"` | Flat `size_x * size_y` vector — each row (axis 0) or column (axis 1) is a copy of the input |

---

## MemoryTrace

A second-layer dynamics element that accumulates a persistent trace of supra-threshold neural field activity. It builds up slowly where the connected field is active and decays slowly everywhere else, producing a representation of the history of peak activations.

**Label:** `MEMORY_TRACE`

### Dynamics

At each time step the trace follows a dual-rate Euler integration:

```
if input[i] > threshold:
    output[i] += deltaT * (1/tauBuild) * (-output[i] + input[i])   // builds toward input
else:
    output[i] += deltaT * (1/tauDecay) * (-output[i])              // decays toward 0
```

The trace should receive the **sigmoid output** (`"output"` component) of a `NeuralField` as its input, not the raw activation, so that the threshold check operates on an already-thresholded signal.

### Parameters

```cpp
MemoryTraceParameters{
    double tauBuild  = 100.0,
    double tauDecay  = 1000.0,
    double threshold = 0.5
}
```

| Parameter | Default | Description |
|---|---|---|
| `tauBuild` | `100.0` | Time constant for building the trace at active locations — smaller values make the trace grow faster |
| `tauDecay` | `1000.0` | Time constant for decay at inactive locations — larger values make the trace persist longer |
| `threshold` | `0.5` | Input level above which a location is considered active and the trace builds; below it the trace decays |

Both time constants should be much larger than the neural field's `tau` (typically 20–100) so that the trace evolves on a slower, learning timescale.

### Components

| Name | Description |
|---|---|
| `"output"` | Accumulated memory trace at each field position (values in [0, 1] when driven by sigmoid output) |
| `"input"` | Summed input from connected elements |

### Typical wiring

```cpp
// Field output drives the trace; trace feeds back into the field as a weak excitatory input
memTrace->addInput(field, "output");   // trace reads sigmoid output of the field
field->addInput(memTrace);             // field receives the trace as input (scaled by a coupling)
```

### Usage note

`setParameters()` does **not** call `init()` — the accumulated trace is preserved when parameters are changed at runtime. Call `init()` explicitly if you want to reset the trace to zero.