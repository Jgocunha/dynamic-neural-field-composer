# 04 — Simulation, recorder, and exception gaps

The simulation layer is broadly well tested (`test_simulation.cpp`,
`test_simulation_extended.cpp`, `test_simulation_file_manager.cpp` ~60 round-trip
cases, `test_simulation_recorder.cpp`), but grep confirms several public methods
are never referenced by any test.

## `Simulation` (`include/simulation/simulation.h`, `src/simulation/simulation.cpp`)

- [x] `renameElement` (`simulation.cpp:362-379`) — four branches, none tested:
  - happy path: element renamed, old name gone, new name resolves,
    connections intact (rename a stimulus wired into a field, step still works);
  - `oldName == newName` → early-return no-op;
  - `oldName` not found → warning + no-op (no throw — `getElement(name)`
    returns nullptr, it does not throw);
  - `newName` already exists → warning + no-op, both elements keep their names.
- [x] `getHighestElementIndex` (`simulation.cpp:483-494`) — equals the max
  `getUniqueIdentifier()` across elements; grows monotonically as elements are
  added; 0 for an empty simulation.
- [x] Timing accessors — currently only "doesn't crash" coverage:
  - `getLastStepDuration()` (`simulation.cpp:516`) > 0 after a step with
    measurement enabled (default);
  - `setMeasureStepDuration(false)` → `lastStepDuration` stays 0 across steps;
    `getMeasureStepDuration()` round-trips;
  - `getTotalRunDuration()` (`simulation.cpp:521-529`) non-decreasing across
    steps; pause freezes the returned value (paused branch returns the
    accumulator only).
- [x] `NeuralField::getSelfExcitationKernel` (`src/elements/neural_field.cpp:69-86`)
  — dynamic-cast/kernel-lookup logic, untested:
  - returns the GaussKernel that is wired field → kernel → field (true
    self-excitation loop);
  - returns nullptr when the field has a kernel input that is *not* fed by the
    field itself (kernel input from another element);
  - returns nullptr with no kernel inputs at all.
- [ ] `resetElement` (`simulation.cpp`, header `simulation.h:104`) — the
  preserve-connections contract is only thinly exercised; add: reset a field
  that has both an input and a downstream consumer, assert both interactions
  survive with the replacement element.

## `SimulationRecorder` (`include/simulation/simulation_recorder.h`)

- [x] `RecordingIntervalUnit::Milliseconds` — `update()`'s ms-based sampling
  path has zero coverage (only Ticks is asserted). With `deltaT=1` (1 tick =
  1 ms internally via `sim.t`), sample every 2 ms over 10 steps → same 5-row
  expectation as the Ticks test; proves the unit branch.
- [x] Duplicate `startRecording` for an identical pair → no-op (documented in
  the header): second call does not create a second file or duplicate rows.
- [x] `stopRecording` for a pair that was never started → no-op, no throw.
- [ ] `takeSnapshot` for a 2D element → export CSV carries the
  `# size_x=…` metadata comment (recordings are covered for 2D; snapshots not).

## `Exception` (`include/exceptions/exception.h`, `tests/exceptions/test_exception.cpp`)

The parametrized `getErrorMessage` suite (`test_exception.cpp:116-136`) omits
several `ErrorCode` values, so their message mapping is unverified (a missing
`case` returning an empty string would pass CI today):

- [x] Add to `INSTANTIATE_TEST_SUITE_P`: `SIM_ELEM_NOT_FOUND`, `SIM_ELEM_INDEX`,
  `SIM_ELEM_ALREADY_EXISTS`, `ELEM_INPUT_SIZE_MISMATCH`, `ELEM_SIZE_NOT_ALLOWED`,
  `ELEM_RENAME_NOT_ALLOWED`, `GAUSS_STIMULUS_POSITION_OUT_OF_RANGE`,
  `GAUSS_STIMULUS_SUM_MISMATCH`, `LOG_LOCAL_TIME_ERROR`.
  (The two shared `TEST_P` bodies — non-empty message, code round-trip — then
  cover them automatically.)

## Recipes

Rename with connections intact:

```cpp
Simulation sim("rename-test", 1.0, 0.0, 0.0);
sim.addElement(makeStimulus("stim"));
sim.addElement(makeField("field"));
sim.createInteraction("stim", "output", "field");
sim.init();
sim.renameElement("stim", "stim2");
EXPECT_EQ(sim.getElement("stim"), nullptr);
ASSERT_NE(sim.getElement("stim2"), nullptr);
EXPECT_NO_THROW(sim.step());          // interaction still drives the field
```

Self-excitation kernel: build field `u`, `GaussKernel k`, wire
`createInteraction("u","output","k")` **and** `createInteraction("k","output","u")`,
init → `u->getSelfExcitationKernel() == k`.
