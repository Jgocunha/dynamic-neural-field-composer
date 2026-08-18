# 02 — Field metrics (bump detection, centroid, width, velocity)

The issue's headline gap. All metric computation lives in the **core elements**
(`NeuralField::updateBumps` / `NeuralField2D::updateBumps`) — the GUI
`field_metrics_window.cpp` only *reads* `getBumps()`, so everything here is
plain unit-testable.

## Where the code lives

| What | Where |
|---|---|
| 1D bump struct (`centroid`, `startPosition`, `endPosition`, `amplitude`, `width`, `velocity`, `acceleration`) | `include/elements/neural_field.h:82-126` |
| 1D scan: threshold `0.00001`, start/end/centroid/width, velocity match `epsilon = 2.0` | `src/elements/neural_field.cpp:155-207` |
| 1D end-of-field finalize branch | `src/elements/neural_field.cpp:209-234` |
| 1D wrap-around merge branch (first+last bump joined across the periodic boundary) | `src/elements/neural_field.cpp:237-275` |
| 1D fused state pass (sum/avg/norm/min/max/stability) | `src/elements/neural_field.cpp:117-153` |
| 2D bump struct (`centroid_x/y`, `amplitude`, `area`, `velocity_x/y`) | `include/elements/neural_field_2d.h:68-91` |
| 2D BFS flood-fill, activation-weighted centroid, `area = cellCount*d_x*d_y` | `src/elements/neural_field_2d.cpp:90-173` |

Semantics worth knowing before writing assertions:

- Positions are 1-based-scaled: `startPosition = (i+1)*d_x`; the 2D centroid
  uses `(cx+1)*d_x` weights. With `d_x = 1`, a stimulus at position `p` yields a
  centroid near `p` (± discretisation).
- 1D `width` counts above-threshold samples then does `(width-1)*d_x`.
- Velocity/acceleration only computed when a previous-step bump's centroid is
  within `epsilon = 2.0`; otherwise 0. So a bump must *drift* (small per-step
  moves), not jump, to register velocity.
- 2D velocity match uses `std::hypot` distance < 2.0; there is **no**
  acceleration in 2D.

## 2D — `test_neural_field_2d.cpp` (was: bump metrics completely unverified)

Prior state: the only 2D "metrics" test inspected the raw activation peak index;
`getBumps()` was never called anywhere in the suite.

- [x] `NeuralField2DBumps.NoBumpsAtRestingLevel` — resting level −5, no input,
  step → `getBumps().empty()`.
- [x] `NeuralField2DBumps.SingleBumpDetected` — `GaussStimulus2D` (amp 20,
  sigma 2) at (15, 20) on a 30×30 field, ~200 steps → exactly 1 bump.
- [x] `NeuralField2DBumps.CentroidNearStimulusPosition` — same setup →
  `centroid_x ≈ 15`, `centroid_y ≈ 20` (tol 2.0).
- [x] `NeuralField2DBumps.AmplitudeMatchesHighestActivation` — bump amplitude
  == `getHighestActivation()` (the max is inside the bump) within 1e-9.
- [x] `NeuralField2DBumps.AreaIsPositiveAndBounded` — `0 < area ≤ size_x*size_y*d_x*d_y`;
  and grows with stimulus sigma (compare sigma 2 vs sigma 4).
- [x] `NeuralField2DBumps.TwinBumpsSeparatedByFloodFill` — two stimuli at
  (8, 8) and (22, 22) → exactly 2 bumps with distinct centroids (this exercises
  the visited-set/BFS separation logic).
- [x] `NeuralField2DBumps.VelocityNonZeroWhenStimulusMoves` — settle at
  (15, 15), then move the stimulus +3 in x (`setParameters`), step and take the
  max `|velocity_x|` over the transient → > 0; `velocity_y` stays ≈ 0 relative
  to `velocity_x`.
- [x] `NeuralField2DBumps.VelocityZeroForStationaryBump` — settled bump →
  both velocity components ≈ 0 at convergence.
- [ ] `NeuralField2DBumps.AreaScalesWithGridSpacing` — same field with
  `d_x = d_y = 0.5` → area ≈ quarter of the `d=1` case (verifies the
  `cellCount*d_x*d_y` formula, not just positivity).
- [ ] 2D `setComputeStateMetrics(false)` → `getBumps()` stays empty and
  min/max stay stale after steps (mirrors the 1D test below).

## 1D — `test_neural_field.cpp` additions

Existing coverage: single-bump detection, centroid, velocity-on-move, stability,
min/max. Missing branches:

- [x] `NeuralFieldBumps.TwoSeparatedBumps` — stimuli at 25 and 75 (sigma 5,
  amp 30) → exactly 2 bumps, centroids near 25 and 75. (Multi-bump was never
  tested in 1D either.)
- [x] `NeuralFieldBumps.BumpTouchingLastIndexIsFinalized` — non-circular
  stimulus (`circular=false`) centred at 100 on a 100-unit field → activation
  stays above threshold through the last index; the finalize branch
  (`neural_field.cpp:209-234`) must produce a bump whose
  `endPosition == size*d_x`.
- [x] `NeuralFieldBumps.WrapAroundBumpIsMerged` — *circular* stimulus centred
  at position ~100 (field boundary) → activation above threshold at both index 0
  and the last index → the merge branch (`neural_field.cpp:237-275`) collapses
  first+last into one bump; assert bump count and that the merged centroid is
  near the boundary (mod x_max).
- [x] `NeuralFieldBumps.AccelerationNonZeroDuringSpeedChange` — settle, move
  the stimulus, and track `|acceleration|` over the transient → > 0 at least
  once (velocity changes from 0 to non-zero, so acceleration must fire).
  `acceleration` was never asserted anywhere in the suite.
- [x] `NeuralFieldMetricsToggle.DisabledSkipsStateAndBumps` —
  `setComputeStateMetrics(false)`, drive with a strong stimulus → `getBumps()`
  stays empty, `getHighestActivation()` keeps its stale value;
  `getComputeStateMetrics()` round-trips.
- [x] `NeuralFieldStability.ThresholdAccessorsRoundTrip` —
  `setThresholdForStability(x)` / `getStabilityThreshold()`; and a huge
  threshold makes `isStable()` true after ~2 steps while a tiny threshold keeps
  it false under an active transient.
- [ ] `NeuralFieldBumps.WidthMatchesAboveThresholdExtent` — analytic check:
  for a settled sigmoid-output bump, `width ≈ endPosition - startPosition`
  (tie the three fields together instead of only `width > 0`).
- [ ] Velocity *sign* check: stimulus moved to the right → transient `velocity > 0`
  (current test only asserts magnitude).

## Recipes

Synthetic profiles via the standard wiring (copy the helpers at the top of each
test file):

```cpp
Simulation sim("name", 1.0, 0.0, 0.0);
auto stim  = makeStimulus("stim", /*pos*/ 50.0, /*amp*/ 30.0);  // GaussStimulusParameters{5.0, amp, pos, circular, false}
auto field = makeField("field", 100, 25.0, -5.0);
sim.addElement(stim); sim.addElement(field);
sim.createInteraction("stim", "output", "field");
sim.init();
for (int i = 0; i < 200; ++i) sim.step();   // converge
```

To *move* a bump: `std::dynamic_pointer_cast<GaussStimulus>(sim.getElement("stim"))
->setParameters(GaussStimulusParameters{5.0, 30.0, newPos, true, false});` then
step and sample `getBumps()` each step, accumulating max |velocity| / |acceleration|.

For 2D, `GaussStimulus2DParameters{sigma, amplitude, pos_x, pos_y, circular, normalized}`
and `ElementCommonParameters{"name", ElementDimensions(sx, sy, d_x, d_y)}`;
wire with `nf->addInput(stim)` directly or via a Simulation.
