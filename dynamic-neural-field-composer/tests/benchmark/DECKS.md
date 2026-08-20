# Benchmark deck selection

`decks.json` names four simulations, drawn from the 606 already committed under
`tests/validation/data/`, that `dnf_composer_deckbench` (see `deckbench_main.cpp`) times as
the reproducible performance baseline for this project. They are correctness fixtures first —
every one has a committed reference CSV — so a deck that regresses in speed can be handed
straight to the matching validation suite (`test_field_dynamics_1d.cpp`,
`test_field_dynamics_2d.cpp`, `test_spectral_golden_2d.cpp`) to confirm its numerics did not
also move. Authoring new benchmark-only architectures would have thrown that property away.

## Why these four

| Tier | Deck | What it uniquely covers |
|---|---|---|
| `small` | `1d/simulations/sim_001_sigmoid_b100.json` (1D, size 100) | The 1D convolution path and the AVX2 sigmoid activation kernel. Cheapest deck — the fast inner loop for quick pre-PR checks |
| `medium` | `2d/simulations/sim_049_sigmoid_b100.json` (2D, 50×50) | A 2-term MexicanHat kernel: two `conv2d_separable_into` calls per step, a different code shape from a 1-term Gauss. Below `kFFTMinAxisSize` (100), so permanently on the direct path regardless of dispatch |
| `large-a` | `2d_spectral/simulations/golden_001_gauss_narrow.json` (128×128) | 2D direct path at a realistic grid. 102 taps — just under `kFFTTapThreshold` (120) |
| `large-b` | `2d_spectral/simulations/golden_002_gauss_wide.json` (128×128) | 2D spectral/FFTW path. 134 taps — just over the threshold |

`large-a` and `large-b` are the point of the set. They are the **same architecture at the same
grid**, differing only in `GaussKernel2D` width (5.0 vs 6.5) — the sole reason one lands on the
direct path and the other on the spectral path under `kFFTTapThreshold`. That makes the pair a
controlled A/B across the dispatch boundary: everything else held constant, it isolates
direct-vs-spectral cost directly, and a change that moves the crossover shows up as the pair's
relative timing inverting rather than as a single number drifting. Tap counts and dispatch
outcomes for both are already documented in `tests/validation/data/2d_spectral/README.md`.

The `sigmoid_b100` variants of `sim_001`/`sim_049` are chosen over the corresponding
`heaviside` decks deliberately — the sigmoid activation function exercises the AVX2 kernel in
`src/elements/activation_function.cpp`; heaviside does not.

## Coverage

1D and 2D, direct and spectral convolution, 1-term and 2-term kernels, the AVX2 sigmoid path,
and the direct/spectral dispatch boundary itself (`tools::math::kFFTTapThreshold`,
`kFFTMinAxisSize` in `include/tools/fft_convolution.h`).

## Changing the set

**Changing a deck invalidates every historical comparison recorded against it** — a baseline in
`tests/benchmark/baselines/` is only meaningful when compared against a run over the exact same
deck bytes (`dnf_composer_deckbench` hashes each deck file into its JSON output precisely to
catch a silent edit). Do not edit the deck JSONs themselves — they are validation fixtures with
committed reference CSVs, and an edit breaks that suite as well as this one.

Changing which four decks appear in `decks.json` is therefore a deliberate act, not a tweak:
state the reason in the PR, and note it here with a date so a later reader knows the historical
baselines before that point were measured against a different deck.

| Date | Change | Reason |
|---|---|---|
| 2026-08-20 | Initial selection: `sim_001_sigmoid_b100`, `sim_049_sigmoid_b100`, `golden_001_gauss_narrow`, `golden_002_gauss_wide` | Smallest set covering 1D + 2D, direct + spectral, 1-term + 2-term kernels, and the dispatch boundary — see rationale above |
