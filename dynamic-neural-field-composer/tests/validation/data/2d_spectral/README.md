# 2D spectral-path golden fixtures

Six 128x128 simulations, hand-written to straddle `tools::math::kFFTTapThreshold`
(120) at grid size 128: some stay on the direct convolution path under `Auto`
dispatch, the rest cross into the spectral (FFTW) path. This is the first
golden fixture set in the repo at or above `kFFTMinAxisSize` (100) — the
existing `tests/validation/data/2d/` fixtures are 50x50 and never exercise the
spectral path.

| fixture | element | width(s) | dispatch under Auto @128x128 |
|---|---|---|---|
| `golden_001_gauss_narrow` | GaussKernel2D | 5.0 | direct (totalTaps=102) |
| `golden_002_gauss_wide` | GaussKernel2D | 6.5 | spectral (totalTaps=134) |
| `golden_003_gauss_clamped` | GaussKernel2D | 13.0 | spectral, full clamp (totalTaps=256) |
| `golden_004_asymmetric_gauss` | AsymmetricGaussKernel2D | 13.0, timeShift_x=1.5, timeShift_y=-0.7 | spectral |
| `golden_005_oscillatory` | OscillatoryKernel2D | decay=0.08 (default) | spectral |
| `golden_006_mexican_hat` | MexicanHatKernel2D | widthExc=8.0, widthInh=16.0 | spectral (2-term) |

`CorrelatedNormalNoise2D` is deliberately excluded — its input is stochastic
(fresh white noise every step), so a committed CSV would be meaningless for
drift detection. It is covered instead by the seeded differential tests in
`tests/elements/test_spectral_dispatch_2d.cpp` and the clamp-fix tests in
`tests/elements/test_correlated_normal_noise_2d.cpp`.

## How these were generated

```
cmake --build build/release --config Release --target dnf_composer_regen_spectral_golden
build/release/tests/Release/dnf_composer_regen_spectral_golden.exe
```

The tool (`tests/validation/tools/regenerate_spectral_golden.cpp`) reuses
`validation_common.h`'s `runProtocol()` (500 steps stimulus ON, 500 steps
stimulus OFF) and wraps the run in `ScopedConvolutionMode(ForceDirect)`, so
every committed CSV is, by construction, the **direct** convolution path's
answer — never the spectral path's. `test_spectral_golden_2d.cpp` then checks
the same CSVs against both `ForceDirect` (must still match — pins the direct
path against drift) and `Auto` (which takes the spectral branch on these
fixtures — pins the spectral path against drift, since it must reproduce the
direct-path reference within `kAbsTolerance` = 1e-4).

Regenerate only when the direct convolution path's numerics deliberately
change (e.g. a `conv2d_separable_into` optimization). Do not regenerate to
"fix" a failing `SpectralGolden2D` test — a failure there means the spectral
and direct paths have diverged, which is the bug this suite exists to catch.

Provenance of the current CSVs:
- Built from commit `9195e3a5ed03068f7ce94b3c2f0bcdfe93167a56` plus the
  in-progress hybrid-convolution changes (shared spectral dispatch extended
  from `MexicanHatKernel2D` to all five 2D convolution elements).
- Windows, MSVC (Visual Studio 17.11, CMake 3.30.4), x64 Release.
- Verified finite and stable (see the min/max sanity check run during
  generation) before committing.
