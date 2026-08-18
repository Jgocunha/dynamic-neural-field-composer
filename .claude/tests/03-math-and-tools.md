# 03 — Math and tools gaps

`tools/math.h` is the numerical backbone of every element, and a surprising
amount of it has zero tests — including the 2D convolution core that every 2D
kernel runs through each step. `tools/profiling.h` has no test file at all.

## `tools/math.h` — untested functions

Existing `tests/tools/test_math.cpp` covers: `conv`, `conv_valid`, `conv_same`
(size only), `gauss(rangeX,...)`, `gaussNorm`, `sigmoid`, `heaviside`,
`compareVectors`, `calculateVector{Sum,Avg,Norm}`, `computeKernelRange`,
`createExtendedIndex`, `generateNormalVector`, `hebbLearningRule`,
`normalize(scalar)`, `gaussian_2d`, `wrap`, `reduce2DAxis_into`,
`broadcast1DTo2D_into`. Everything below was untested.

### Convolution / circular indexing (highest value — element step-path code)

- [x] `conv_valid_into` (`math.h:97`) — must equal `conv_valid` on the same
  inputs (pre-sized out buffer); reuse the `[1,2,3,4,5]*[1,1,1] → [6,9,12]` case.
- [x] `conv_same_into` (`math.h:114`) — must equal `conv_same`; also add a
  *value* test for `conv_same` itself (currently only size-checked):
  `[1,2,3,4,5] same [1,1,1] → [3,6,9,12,9]` (zero-padded edges).
- [x] `obtainCircularVector` / `obtainCircularVector_into` (`math.h:261/132`) —
  indices are **1-based** (`contents[indices[i]-1]`); assert a known permutation
  and that the two variants agree.
- [x] `conv2d_separable` (`math.h:630`) — the 2D kernel work-horse:
  - delta kernels (`kx = ky = {0,1,0}`, non-circular) → output == input
    (identity, catches axis mix-ups);
  - separable box blur on a one-hot 3×3 field → hand-computed 3×3 result;
  - y-major orientation check: asymmetric kernels (`kx = {0,0,1}` shift) move
    mass along the intended axis.
- [x] `conv2d_separable_into` (`math.h:695`) — bit-identical to
  `conv2d_separable` on the same inputs (both circular and non-circular modes;
  circular mode uses `createExtendedIndex` outputs).

### Gauss family

- [x] `gauss(size, sigma, position)` overload (`math.h:172`) — positions are
  1-based (`x = i+1`): peak index == `position-1`, peak value 1.
- [x] `nonCircularGauss` (`math.h:186`) — same semantics; equals the
  `gauss(size,...)` overload on identical inputs.
- [x] `circularGauss` (`math.h:202`) — peak at `position`; wrap-around: for a
  position near the edge, the value at the opposite edge exceeds the value at
  mid-field (periodic distance); symmetric distance from both sides.
- [x] `gaussDerivative` (`math.h:227`) — analytic: zero at `x == position`,
  antisymmetric (`f(pos+d) == -f(pos-d)`), sign: negative for `x > position`
  (with positive amplitude).
- [x] `gaussDerivativeNorm` (`math.h:242`) — sum of |values| == 1; preserves
  the sign pattern of `gaussDerivative`; all-zero input (amplitude 0) does not
  divide by zero.
- [x] `sumGauss` (`math.h:308`) — element-wise sum of two vectors.
- [x] `absSigmoid` (`math.h:287`) — value at `x0` is 0.5, monotonic, bounded
  (0, 1), and at beta=100 agrees with `sigmoid` to ~1e-2 far from `x0`.
- [x] `gaussian_2d_periodic` (`math.h:518`) — peak at mean == A; wrap: point at
  distance `max_x - 1` equals point at distance 1 (periodic metric).
- [x] `circular_gaussian_2d` (`math.h:505`) — peak at mean == A; isotropic decay
  (equal at equal radius). (Name says "circular" but it is the *isotropic*, not
  periodic, variant — the test should pin that down.)

### Learning rules (only Hebb was tested)

Hand-computed one-step updates on 2×2 systems:

- [x] `ojaLearningRule` (`math.h:375`) — from zero weights, one step equals the
  Hebb update (the decay term is `-out*in*w = 0`); from non-zero weights the
  decay shrinks the update; matches
  `w += lr*(in*out - out*in*w)` computed by hand.
- [x] `deltaLearningRuleWidrowHoff` (`math.h:392`) — `w[i][j] += lr*err[j]*in[i]`
  with `err = target - actual`; zero error → no change; hand-computed 2×2 case.
- [x] `deltaLearningRuleKroghHertz` (`math.h:415`) — currently the same update
  formula but with `(input, targetOutput, actualOutput)` **argument order
  swapped vs Widrow-Hoff** (`target` is param 3, `actual` param 4) — a test
  pinning the order guards against a silent regression if the signature is
  "harmonised" later.

### Misc

- [x] `normalize(vector)` (`math.h:323`) — output min == 0, size preserved,
  order preserved (monotone transform); constant-input behaviour documented by
  the test.
- [x] `flattenMatrix` (`math.h:526`) — row-major order pinned:
  `{{1,2},{3,4}} → {1,2,3,4}`.
- [x] `resample` (`math.h:541`) — identity when sizes match; endpoints
  preserved (`out.front()==in.front()`, `out.back()==in.back()`); linear data
  stays exactly linear under up- and down-sampling; `outputSize==1` → middle
  element; empty input → empty.
- [x] `resampleInto` (`math.h:564`) — agrees with `resample` for the same sizes.
- [x] `resampleNearestInto` (`math.h:583`) — output values are a subset of
  input values (nearest never interpolates); size-match copy.
- [x] `resampleCubicInto` (`math.h:600`) — Catmull-Rom reproduces linear data
  exactly; interpolates through the sample points (values at integer positions
  == input); smooth upsample of a hat function stays within [min, max]+overshoot
  bound.

## `tools/profiling.h` — `Timer` (no test file existed)

RAII timer that prints `Signature: <sig> Duration (us): <n>` to a stream on
destruction (`src/tools/profiling.cpp:25-35`).

- [x] New file `tests/tools/test_profiling.cpp` (add to `tests/CMakeLists.txt`):
  - destruction writes to the injected `std::ostringstream` (not stdout);
  - output contains the signature and `Duration (us):`;
  - default-constructed signature works;
  - nothing is written *before* destruction (scope test).

## `tools/utils.h` — leftovers

Tested already: `countNumOfLinesInFile`, `saveVectorToFile`,
`replaceForwardSlashesWithBackslashes`, `resizeMatrix`, `generateRandomNumber`,
`fillMatrixWithRandomValues`.

- [ ] `getResourceRoot()` (`utils.h:15`) — returns a non-empty, existing
  directory; stable across calls. (Indirectly exercised by recorder tests but
  never asserted on its own.)
- [ ] `safe_localtime()` (`utils.h:56`) — round-trip a known `time_t` and check
  the returned tm fields; thread-safety smoke (call from 2 threads).
- [ ] `getProcessMemoryMb()` (`utils.h:65`) — returns > 0 on all platforms.
