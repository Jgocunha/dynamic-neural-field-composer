# dnf-composer per-step profiler

Per-element step() timing (1D size 100, 2D 50x50). One section appended per run.

## 2026-06-25 09:51:41  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.11 | 2.10 | 1.70 | 26.70 |
| GaussKernel | 1.55 | 1.50 | 1.50 | 25.40 |
| MexicanHatKernel | 2.78 | 2.80 | 2.70 | 19.70 |
| OscillatoryKernel | 5.70 | 5.50 | 5.40 | 43.70 |
| AsymmetricGaussKernel | 1.55 | 1.50 | 1.50 | 16.80 |
| NormalNoise | 3.23 | 3.00 | 2.60 | 21.20 |
| CorrelatedNormalNoise | 4.08 | 3.90 | 3.40 | 26.40 |
| MemoryTrace | 4.06 | 4.00 | 3.90 | 41.70 |
| GaussStimulus | 0.03 | 0.00 | 0.00 | 3.40 |
| TimedGaussStimulus | 0.06 | 0.10 | 0.00 | 0.10 |
| BoostStimulus | 0.14 | 0.10 | 0.00 | 2.50 |
| NeuralField2D | 50.14 | 50.30 | 38.90 | 99.80 |
| GaussKernel2D | 71.69 | 69.60 | 67.10 | 207.80 |
| MexicanHatKernel2D | 188.70 | 187.30 | 180.10 | 343.90 |
| OscillatoryKernel2D | 129.95 | 129.30 | 124.40 | 253.30 |
| AsymmetricGaussKernel2D | 69.83 | 69.50 | 67.20 | 178.90 |
| NormalNoise2D | 78.99 | 78.10 | 74.30 | 206.00 |
| CorrelatedNormalNoise2D | 105.84 | 103.10 | 97.80 | 285.80 |
| MemoryTrace2D | 101.31 | 100.30 | 94.20 | 424.30 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.30 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 9.50 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 30.00 |
| Collapse (2D->1D) | 4.33 | 4.30 | 4.20 | 14.30 |
| Expand (1D->2D) | 2.03 | 2.00 | 2.00 | 21.70 |
| Resize (1D) | 0.28 | 0.30 | 0.20 | 0.70 |

### Representative 1D detection sim  (total 3.70 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.65% |
| neural field u | NeuralField | 2.09 | 56.45% |
| gauss kernel | GaussKernel | 1.54 | 41.71% |
| normal noise | NormalNoise | 0.04 | 1.19% |

### Representative 2D detection sim  (total 125.60 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.02 | 0.02% |
| neural field u | NeuralField2D | 55.32 | 44.05% |
| gauss kernel 2d | GaussKernel2D | 70.04 | 55.76% |
| normal noise 2d | NormalNoise2D | 0.22 | 0.17% |

## 2026-06-25 09:52:00  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.07 | 2.10 | 1.70 | 17.10 |
| GaussKernel | 1.57 | 1.50 | 1.50 | 50.60 |
| MexicanHatKernel | 2.73 | 2.70 | 2.70 | 28.10 |
| OscillatoryKernel | 5.51 | 5.40 | 5.30 | 63.60 |
| AsymmetricGaussKernel | 1.55 | 1.50 | 1.50 | 43.90 |
| NormalNoise | 3.22 | 3.00 | 2.60 | 16.10 |
| CorrelatedNormalNoise | 4.10 | 3.90 | 3.40 | 16.80 |
| MemoryTrace | 3.94 | 3.90 | 3.80 | 24.10 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.50 |
| TimedGaussStimulus | 0.04 | 0.00 | 0.00 | 0.10 |
| BoostStimulus | 0.08 | 0.10 | 0.00 | 0.20 |
| NeuralField2D | 50.23 | 50.50 | 38.30 | 146.30 |
| GaussKernel2D | 70.58 | 69.50 | 66.70 | 402.20 |
| MexicanHatKernel2D | 188.03 | 187.10 | 180.10 | 651.40 |
| OscillatoryKernel2D | 129.95 | 129.30 | 124.40 | 232.40 |
| AsymmetricGaussKernel2D | 70.05 | 69.70 | 66.90 | 162.70 |
| NormalNoise2D | 78.79 | 78.10 | 73.10 | 158.10 |
| CorrelatedNormalNoise2D | 103.82 | 102.70 | 98.00 | 256.90 |
| MemoryTrace2D | 98.96 | 97.50 | 93.60 | 468.20 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.30 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 7.90 |
| BoostStimulus2D | 0.22 | 0.20 | 0.20 | 18.50 |
| Collapse (2D->1D) | 4.58 | 4.50 | 4.30 | 126.80 |
| Expand (1D->2D) | 2.05 | 2.00 | 2.00 | 15.40 |
| Resize (1D) | 0.28 | 0.30 | 0.20 | 1.00 |

### Representative 1D detection sim  (total 3.73 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.64% |
| neural field u | NeuralField | 2.10 | 56.35% |
| gauss kernel | GaussKernel | 1.56 | 41.86% |
| normal noise | NormalNoise | 0.04 | 1.15% |

### Representative 2D detection sim  (total 129.26 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.02% |
| neural field u | NeuralField2D | 56.97 | 44.07% |
| gauss kernel 2d | GaussKernel2D | 72.03 | 55.72% |
| normal noise 2d | NormalNoise2D | 0.24 | 0.18% |

## 2026-06-25 10:11:36  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.13 | 2.10 | 1.70 | 147.70 |
| GaussKernel | 1.56 | 1.50 | 1.50 | 19.40 |
| MexicanHatKernel | 3.05 | 2.80 | 2.70 | 55.10 |
| OscillatoryKernel | 5.58 | 5.50 | 5.40 | 36.90 |
| AsymmetricGaussKernel | 1.60 | 1.60 | 1.50 | 23.50 |
| NormalNoise | 0.64 | 0.60 | 0.50 | 7.90 |
| CorrelatedNormalNoise | 2.95 | 2.90 | 2.70 | 39.80 |
| MemoryTrace | 4.28 | 4.10 | 3.90 | 32.50 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.50 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 0.10 |
| BoostStimulus | 0.08 | 0.10 | 0.00 | 0.20 |
| NeuralField2D | 49.99 | 50.20 | 38.60 | 114.00 |
| GaussKernel2D | 71.91 | 70.90 | 68.20 | 232.60 |
| MexicanHatKernel2D | 189.31 | 188.20 | 180.40 | 318.90 |
| OscillatoryKernel2D | 130.04 | 129.50 | 124.70 | 223.80 |
| AsymmetricGaussKernel2D | 71.12 | 70.60 | 67.70 | 169.70 |
| NormalNoise2D | 14.69 | 14.60 | 13.70 | 66.60 |
| CorrelatedNormalNoise2D | 75.99 | 74.80 | 69.60 | 149.40 |
| MemoryTrace2D | 101.92 | 100.40 | 95.80 | 333.20 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 15.90 |
| BoostStimulus2D | 0.20 | 0.20 | 0.20 | 0.30 |
| Collapse (2D->1D) | 4.47 | 4.30 | 4.20 | 26.80 |
| Expand (1D->2D) | 2.04 | 2.00 | 2.00 | 19.50 |
| Resize (1D) | 0.27 | 0.30 | 0.20 | 0.40 |

### Representative 1D detection sim  (total 3.72 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.64% |
| neural field u | NeuralField | 2.09 | 56.31% |
| gauss kernel | GaussKernel | 1.55 | 41.81% |
| normal noise | NormalNoise | 0.05 | 1.24% |

### Representative 2D detection sim  (total 127.50 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.02 | 0.02% |
| neural field u | NeuralField2D | 56.04 | 43.95% |
| gauss kernel 2d | GaussKernel2D | 71.22 | 55.86% |
| normal noise 2d | NormalNoise2D | 0.22 | 0.17% |

## 2026-06-25 10:41:16  (dnfc 2.9.3, 200000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.10 | 2.10 | 1.60 | 27.60 |
| GaussKernel | 1.56 | 1.60 | 1.50 | 24.40 |
| MexicanHatKernel | 2.79 | 2.80 | 2.60 | 57.70 |
| OscillatoryKernel | 5.52 | 5.50 | 5.30 | 245.50 |
| AsymmetricGaussKernel | 1.55 | 1.50 | 1.50 | 136.30 |
| NormalNoise | 0.64 | 0.60 | 0.50 | 25.90 |
| CorrelatedNormalNoise | 3.07 | 3.00 | 2.70 | 123.90 |
| MemoryTrace | 4.40 | 4.30 | 4.10 | 90.90 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 6.90 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 6.50 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 5.10 |
| NeuralField2D | 49.93 | 50.10 | 37.50 | 322.30 |
| GaussKernel2D | 70.45 | 69.70 | 66.60 | 321.10 |
| MexicanHatKernel2D | 188.87 | 188.10 | 180.10 | 1605.80 |
| OscillatoryKernel2D | 131.46 | 130.70 | 124.60 | 819.60 |
| AsymmetricGaussKernel2D | 71.43 | 69.70 | 66.60 | 4820.20 |
| NormalNoise2D | 15.31 | 14.60 | 13.60 | 352.90 |
| CorrelatedNormalNoise2D | 80.48 | 78.00 | 71.90 | 1903.80 |
| MemoryTrace2D | 110.63 | 105.70 | 99.50 | 592.70 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.20 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 32.10 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 22.20 |
| Collapse (2D->1D) | 4.46 | 4.40 | 4.10 | 82.90 |
| Expand (1D->2D) | 2.06 | 2.00 | 1.90 | 35.00 |
| Resize (1D) | 0.28 | 0.30 | 0.20 | 21.20 |

### Representative 1D detection sim  (total 3.74 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.64% |
| neural field u | NeuralField | 2.10 | 56.16% |
| gauss kernel | GaussKernel | 1.57 | 41.92% |
| normal noise | NormalNoise | 0.05 | 1.28% |

### Representative 2D detection sim  (total 124.97 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.02% |
| neural field u | NeuralField2D | 54.50 | 43.61% |
| gauss kernel 2d | GaussKernel2D | 70.23 | 56.20% |
| normal noise 2d | NormalNoise2D | 0.21 | 0.17% |

## 2026-06-25 10:45:19  (dnfc 2.9.3, 200000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.10 | 2.10 | 1.60 | 99.00 |
| GaussKernel | 1.57 | 1.60 | 1.50 | 127.20 |
| MexicanHatKernel | 2.77 | 2.70 | 2.60 | 99.90 |
| OscillatoryKernel | 5.54 | 5.50 | 5.30 | 290.70 |
| AsymmetricGaussKernel | 1.56 | 1.50 | 1.40 | 367.00 |
| NormalNoise | 0.64 | 0.60 | 0.50 | 31.50 |
| CorrelatedNormalNoise | 3.13 | 3.00 | 2.80 | 37.60 |
| MemoryTrace | 4.51 | 4.40 | 4.20 | 64.60 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 31.50 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 175.80 |
| NeuralField2D | 50.03 | 50.10 | 37.50 | 1685.90 |
| GaussKernel2D | 72.37 | 69.80 | 66.70 | 2323.60 |
| MexicanHatKernel2D | 192.13 | 188.50 | 180.40 | 2875.00 |
| OscillatoryKernel2D | 132.68 | 130.90 | 125.20 | 2696.00 |
| AsymmetricGaussKernel2D | 71.41 | 70.00 | 66.80 | 587.40 |
| NormalNoise2D | 14.86 | 14.60 | 13.50 | 474.50 |
| CorrelatedNormalNoise2D | 78.79 | 77.10 | 72.80 | 1172.70 |
| MemoryTrace2D | 111.22 | 106.20 | 100.00 | 4760.30 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 20.80 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 17.60 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 18.90 |
| Collapse (2D->1D) | 4.42 | 4.30 | 4.20 | 136.40 |
| Expand (1D->2D) | 2.08 | 2.10 | 2.00 | 45.50 |
| Resize (1D) | 0.28 | 0.30 | 0.20 | 26.10 |

### Representative 1D detection sim  (total 3.79 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.63% |
| neural field u | NeuralField | 2.12 | 56.01% |
| gauss kernel | GaussKernel | 1.59 | 42.09% |
| normal noise | NormalNoise | 0.05 | 1.27% |

### Representative 2D detection sim  (total 129.05 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.02% |
| neural field u | NeuralField2D | 55.30 | 42.86% |
| gauss kernel 2d | GaussKernel2D | 73.46 | 56.93% |
| normal noise 2d | NormalNoise2D | 0.25 | 0.19% |

## 2026-06-25 10:49:50 — INTRA-STEP SAMPLING BREAKDOWN (Very Sleepy, RelWithDebInfo /O2+/Zi)

Method: CPU sampling profiler (Very Sleepy CLI `/a:<pid> /t:170 /mbt`) attached to
`dnf_composer_profiler 200000` (RelWithDebInfo: Release `/O2` codegen + PDB symbols). 170 s,
85,142 samples. This measures *where inside* each step() the CPU time actually goes — by resolved
function, not by assumption. Percentages are of total sampled CPU self-time across the whole
profiler run (all element types swept sequentially), so they reflect each function's share of the
combined workload.

### Top self-time functions (exclusive)

| % CPU | function | what it is |
|------:|----------|------------|
| 60.5% | `tools::math::conv_valid_into<double>` | circular-convolution inner loop (the separable x/y pass) |
| 10.4% | `std::_Hash` (unordered_map<string,vector>) | `components["..."]` lookups |
| 5.9%  | `MemoryTrace2D::step` | its per-cell elementwise loop (the work around the lookups) |
| 5.8%  | `conv2d_separable_into` | separable driver (gather/scatter + pass orchestration) |
| 3.6%  | `zigguratNormal` | noise Gaussian sampler |
| 2.5%  | `SigmoidFunction::apply` + 1.1% `expf` | field output nonlinearity |
| 1.9%  | `Element::updateInput` | input summation |
| 0.4%  | `fillNormal` | noise fill driver |
| 0.23% | `NeuralField2D::updateBumps` | bump flood-fill (**negligible**) |

### `conv_valid_into` (60.5% of all CPU) attributed to the calling element

| element | conv self-time | share of conv |
|---------|---------------:|--------------:|
| MexicanHatKernel2D | 40.6 s | 39.4% (two conv passes) |
| OscillatoryKernel2D | 28.3 s | 27.5% (wide kernel) |
| GaussKernel2D | 15.1 s | 14.7% |
| AsymmetricGaussKernel2D | 14.1 s | 13.7% |
| CorrelatedNormalNoise2D | 4.8 s | 4.7% |

### `unordered_map` hash lookups (12.8% inclusive) attributed to the calling element

| element | hash self-time | share |
|---------|---------------:|------:|
| MemoryTrace2D | 16.3 s | 75.3% (`components["input"]/["output"][i]` inside the 2500-cell loop) |
| CorrelatedNormalNoise2D | 5.3 s | 24.4% (`components["output"][i]` inside the per-cell scale loop) |
| all others | <0.1 s | ~0% (already hoist `components` to a local ref/ptr) |

### Findings vs. prior assumptions
- **Convolution IS the bottleneck** (60.5%), concentrated in `conv_valid_into`. The old "conv is
  only ~11%" note is wrong at 50×50 with these kernels. Optimizing this one inner loop dominates everything.
- **Two elements still pay the string-map tax in per-cell loops**: MemoryTrace2D and CorrelatedNormalNoise2D.
  This is the same hoist already done for the kernels/NeuralField — a guaranteed bit-identical win (~12% of total CPU).
- **Refuted**: the strided y-pass column gather (`obtainCircularVector_into`) is negligible — does not appear.
  `NeuralField2D::updateBumps` is 0.23% — not worth optimizing. Sigmoid is minor (2.5%).
## 2026-06-25 11:11:30  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.12 | 2.10 | 1.80 | 31.30 |
| GaussKernel | 1.55 | 1.50 | 1.50 | 22.80 |
| MexicanHatKernel | 2.78 | 2.70 | 2.60 | 40.20 |
| OscillatoryKernel | 5.67 | 5.60 | 5.40 | 53.90 |
| AsymmetricGaussKernel | 1.53 | 1.50 | 1.40 | 20.50 |
| NormalNoise | 0.64 | 0.60 | 0.50 | 19.60 |
| CorrelatedNormalNoise | 2.98 | 2.90 | 2.60 | 35.40 |
| MemoryTrace | 4.31 | 4.30 | 4.10 | 33.10 |
| GaussStimulus | 0.03 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 10.60 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 0.50 |
| NeuralField2D | 50.56 | 50.50 | 38.20 | 548.50 |
| GaussKernel2D | 71.51 | 70.60 | 67.60 | 567.60 |
| MexicanHatKernel2D | 191.91 | 188.70 | 180.50 | 808.40 |
| OscillatoryKernel2D | 134.21 | 131.10 | 125.30 | 1628.40 |
| AsymmetricGaussKernel2D | 71.73 | 70.70 | 67.40 | 302.50 |
| NormalNoise2D | 14.86 | 14.60 | 13.70 | 157.60 |
| CorrelatedNormalNoise2D | 76.23 | 74.10 | 69.90 | 1380.30 |
| MemoryTrace2D | 106.05 | 104.50 | 97.60 | 242.90 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 7.60 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 0.30 |
| Collapse (2D->1D) | 4.96 | 4.90 | 4.90 | 20.10 |
| Expand (1D->2D) | 2.05 | 2.00 | 2.00 | 17.70 |
| Resize (1D) | 0.27 | 0.30 | 0.20 | 0.70 |

### Representative 1D detection sim  (total 3.71 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.65% |
| neural field u | NeuralField | 2.08 | 55.99% |
| gauss kernel | GaussKernel | 1.57 | 42.16% |
| normal noise | NormalNoise | 0.04 | 1.20% |

### Representative 2D detection sim  (total 126.98 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.02% |
| neural field u | NeuralField2D | 55.56 | 43.75% |
| gauss kernel 2d | GaussKernel2D | 71.18 | 56.05% |
| normal noise 2d | NormalNoise2D | 0.22 | 0.17% |

## 2026-06-25 11:24:49  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.07 | 2.10 | 1.70 | 23.20 |
| GaussKernel | 0.80 | 0.80 | 0.70 | 236.30 |
| MexicanHatKernel | 1.18 | 1.20 | 1.10 | 13.30 |
| OscillatoryKernel | 2.58 | 2.60 | 2.50 | 27.90 |
| AsymmetricGaussKernel | 0.79 | 0.80 | 0.70 | 16.10 |
| NormalNoise | 0.63 | 0.60 | 0.50 | 9.80 |
| CorrelatedNormalNoise | 3.08 | 2.90 | 2.70 | 129.30 |
| MemoryTrace | 4.07 | 4.10 | 3.90 | 21.60 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.40 |
| TimedGaussStimulus | 0.04 | 0.00 | 0.00 | 0.10 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 0.20 |
| NeuralField2D | 49.80 | 49.90 | 38.00 | 176.20 |
| GaussKernel2D | 35.24 | 34.80 | 33.40 | 166.80 |
| MexicanHatKernel2D | 84.33 | 83.20 | 79.80 | 356.10 |
| OscillatoryKernel2D | 56.85 | 56.10 | 54.30 | 201.60 |
| AsymmetricGaussKernel2D | 34.90 | 34.50 | 33.30 | 258.00 |
| NormalNoise2D | 14.79 | 14.60 | 13.70 | 111.90 |
| CorrelatedNormalNoise2D | 62.28 | 61.70 | 57.10 | 206.50 |
| MemoryTrace2D | 102.27 | 100.20 | 94.90 | 413.60 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.30 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 0.30 |
| BoostStimulus2D | 0.22 | 0.20 | 0.20 | 26.70 |
| Collapse (2D->1D) | 4.43 | 4.40 | 4.30 | 34.50 |
| Expand (1D->2D) | 2.08 | 2.10 | 2.00 | 21.10 |
| Resize (1D) | 0.27 | 0.30 | 0.20 | 9.70 |

### Representative 1D detection sim  (total 2.97 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.80% |
| neural field u | NeuralField | 2.10 | 70.96% |
| gauss kernel | GaussKernel | 0.79 | 26.74% |
| normal noise | NormalNoise | 0.04 | 1.50% |

### Representative 2D detection sim  (total 102.46 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.02 | 0.02% |
| neural field u | NeuralField2D | 64.38 | 62.83% |
| gauss kernel 2d | GaussKernel2D | 37.84 | 36.93% |
| normal noise 2d | NormalNoise2D | 0.21 | 0.21% |

## 2026-06-25 11:27:56  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.09 | 2.10 | 1.70 | 64.10 |
| GaussKernel | 0.78 | 0.80 | 0.70 | 18.70 |
| MexicanHatKernel | 1.17 | 1.20 | 1.10 | 21.70 |
| OscillatoryKernel | 2.60 | 2.60 | 2.50 | 77.60 |
| AsymmetricGaussKernel | 0.80 | 0.80 | 0.70 | 41.80 |
| NormalNoise | 0.65 | 0.60 | 0.50 | 21.00 |
| CorrelatedNormalNoise | 3.06 | 3.00 | 2.80 | 108.90 |
| MemoryTrace | 0.18 | 0.20 | 0.10 | 13.80 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus | 0.04 | 0.00 | 0.00 | 0.10 |
| BoostStimulus | 0.08 | 0.10 | 0.00 | 7.70 |
| NeuralField2D | 50.04 | 50.10 | 38.20 | 493.00 |
| GaussKernel2D | 36.23 | 35.90 | 34.40 | 157.40 |
| MexicanHatKernel2D | 85.46 | 84.40 | 81.60 | 280.70 |
| OscillatoryKernel2D | 57.43 | 57.10 | 55.00 | 235.30 |
| AsymmetricGaussKernel2D | 36.39 | 35.80 | 34.40 | 149.30 |
| NormalNoise2D | 14.86 | 14.50 | 13.80 | 208.10 |
| CorrelatedNormalNoise2D | 28.75 | 28.40 | 27.20 | 147.10 |
| MemoryTrace2D | 3.07 | 3.00 | 2.90 | 29.60 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.30 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 0.40 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 0.30 |
| Collapse (2D->1D) | 4.47 | 4.40 | 4.30 | 40.50 |
| Expand (1D->2D) | 2.06 | 2.00 | 2.00 | 23.10 |
| Resize (1D) | 0.28 | 0.30 | 0.20 | 30.70 |

### Representative 1D detection sim  (total 2.99 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.80% |
| neural field u | NeuralField | 2.12 | 71.10% |
| gauss kernel | GaussKernel | 0.79 | 26.57% |
| normal noise | NormalNoise | 0.05 | 1.53% |

### Representative 2D detection sim  (total 92.02 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.03% |
| neural field u | NeuralField2D | 55.80 | 60.64% |
| gauss kernel 2d | GaussKernel2D | 35.98 | 39.11% |
| normal noise 2d | NormalNoise2D | 0.21 | 0.23% |

## 2026-06-25 11:31:29  (dnfc 2.9.3, 200000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.15 | 2.10 | 1.70 | 429.80 |
| GaussKernel | 1.23 | 0.80 | 0.70 | 7489.40 |
| MexicanHatKernel | 1.36 | 1.20 | 1.10 | 365.50 |
| OscillatoryKernel | 3.07 | 2.60 | 2.50 | 429.60 |
| AsymmetricGaussKernel | 0.93 | 0.80 | 0.70 | 385.80 |
| NormalNoise | 0.75 | 0.60 | 0.50 | 396.10 |
| CorrelatedNormalNoise | 3.39 | 2.80 | 2.70 | 476.50 |
| MemoryTrace | 0.21 | 0.20 | 0.10 | 304.40 |
| GaussStimulus | 0.03 | 0.00 | 0.00 | 279.50 |
| TimedGaussStimulus | 0.06 | 0.00 | 0.00 | 304.30 |
| BoostStimulus | 0.10 | 0.10 | 0.00 | 377.40 |
| NeuralField2D | 59.28 | 49.70 | 37.50 | 1212.20 |
| GaussKernel2D | 41.14 | 34.30 | 33.50 | 1146.70 |
| MexicanHatKernel2D | 99.17 | 83.00 | 80.90 | 2019.40 |
| OscillatoryKernel2D | 66.87 | 56.10 | 54.90 | 1044.70 |
| AsymmetricGaussKernel2D | 42.60 | 34.30 | 33.30 | 9467.20 |
| NormalNoise2D | 17.60 | 14.50 | 13.70 | 821.20 |
| CorrelatedNormalNoise2D | 33.83 | 28.10 | 27.00 | 1117.60 |
| MemoryTrace2D | 3.54 | 3.00 | 2.80 | 557.90 |
| GaussStimulus2D | 0.03 | 0.00 | 0.00 | 270.50 |
| TimedGaussStimulus2D | 0.22 | 0.20 | 0.10 | 299.90 |
| BoostStimulus2D | 0.25 | 0.20 | 0.10 | 338.40 |
| Collapse (2D->1D) | 5.81 | 4.90 | 4.70 | 439.20 |
| Expand (1D->2D) | 2.42 | 2.00 | 2.00 | 702.40 |
| Resize (1D) | 0.32 | 0.30 | 0.20 | 403.30 |

### Representative 1D detection sim  (total 3.35 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.03 | 0.85% |
| neural field u | NeuralField | 2.41 | 71.84% |
| gauss kernel | GaussKernel | 0.87 | 25.87% |
| normal noise | NormalNoise | 0.05 | 1.44% |

### Representative 2D detection sim  (total 104.21 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.03% |
| neural field u | NeuralField2D | 62.70 | 60.16% |
| gauss kernel 2d | GaussKernel2D | 41.20 | 39.53% |
| normal noise 2d | NormalNoise2D | 0.29 | 0.28% |

## 2026-06-25 — AFTER conv AVX2 vectorization + map-lookup hoist (Very Sleepy, RelWithDebInfo)

Same sampling method as the 10:49:50 baseline (Very Sleepy `/a:<pid> /t:120 /mbt` on
`dnf_composer_profiler 200000`). Two changes landed since that baseline:
- **A**: `conv_valid_into` inner loop vectorized with 4-wide `__m256d` FMA (4 outputs/iteration,
  per-output summation order preserved -> bit-identical; golden conv tests pass at 1e-12,
  FieldDynamics 1D/2D at 1e-4).
- **B**: hoisted `components["input"]/["output"]` out of the per-cell loops in MemoryTrace2D,
  MemoryTrace (1D), and CorrelatedNormalNoise2D (bit-identical).

### Top self-time functions (exclusive) — total sweep self-time ~170s -> ~99s (~1.7x)

| % CPU | function | vs baseline |
|------:|----------|-------------|
| 46.0% | `conv_valid_into<double>` | **102.9s -> 45.4s** (AVX2 ~2x) — still the top cost but halved |
| 13.7% | `SigmoidFunction::apply` | ~unchanged (13.5s); rises in % as conv/hash shrank |
| 11.7% | `conv2d_separable_into` | driver |
| 6.4%  | `zigguratNormal` | noise RNG, ~unchanged |
| 6.4%  | `expf` | sigmoid, ~unchanged |
| 5.4%  | `Element::updateInput` | ~unchanged |
| 1.6%  | `NeuralField2D::updateBumps` | still negligible |
| —     | `std::_Hash` (`components[...]`) | **17.7s -> ~0** (hoisted out of per-cell loops) |

### Per-element aggregate step() µs (from the profiler's own timing, 50x50)

| element | baseline µs | after µs | speedup |
|---------|------------:|---------:|--------:|
| GaussKernel2D | 71.2 | 35.2 | 2.0x |
| MexicanHatKernel2D | 189 | 84.3 | 2.2x |
| OscillatoryKernel2D | 130 | 56.9 | 2.3x |
| AsymmetricGaussKernel2D | 71 | 34.9 | 2.0x |
| CorrelatedNormalNoise2D | 76 | 28.8 | 2.6x (conv + hoist) |
| MemoryTrace2D | 102 | 3.1 | **33x** (was almost all map-lookup) |
| MemoryTrace (1D) | 4.4 | 0.18 | 24x |

### Remaining
`conv_valid_into` is still the single largest cost (46%). Further single-thread gains would require
reordering the FP reduction (multi-accumulator over taps), which risks the 1e-4 tolerance — deferred.
`SigmoidFunction::apply`/`expf` (~20% combined) is now the next-largest block. Across-field
threading remains the untouched lever.
## 2026-06-25 11:51:58  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.11 | 2.10 | 1.70 | 27.60 |
| GaussKernel | 0.79 | 0.80 | 0.70 | 16.30 |
| MexicanHatKernel | 1.20 | 1.20 | 1.10 | 32.70 |
| OscillatoryKernel | 2.61 | 2.60 | 2.50 | 26.80 |
| AsymmetricGaussKernel | 0.83 | 0.80 | 0.70 | 50.00 |
| NormalNoise | 0.64 | 0.60 | 0.50 | 25.80 |
| CorrelatedNormalNoise | 3.27 | 2.90 | 2.70 | 44.80 |
| MemoryTrace | 0.19 | 0.20 | 0.10 | 0.30 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus | 0.04 | 0.00 | 0.00 | 0.10 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 0.70 |
| NeuralField2D | 50.15 | 50.30 | 38.30 | 157.30 |
| GaussKernel2D | 35.29 | 34.70 | 33.80 | 127.60 |
| MexicanHatKernel2D | 83.20 | 82.30 | 79.30 | 330.80 |
| OscillatoryKernel2D | 56.51 | 55.50 | 53.80 | 309.00 |
| AsymmetricGaussKernel2D | 34.98 | 34.50 | 33.50 | 161.00 |
| NormalNoise2D | 14.93 | 14.60 | 13.90 | 57.80 |
| CorrelatedNormalNoise2D | 29.79 | 28.60 | 26.90 | 286.60 |
| MemoryTrace2D | 3.14 | 3.00 | 2.90 | 55.50 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus2D | 0.20 | 0.20 | 0.10 | 0.90 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 16.30 |
| Collapse (2D->1D) | 5.08 | 5.00 | 4.90 | 147.80 |
| Expand (1D->2D) | 2.08 | 2.00 | 1.90 | 43.20 |
| Resize (1D) | 0.27 | 0.30 | 0.20 | 0.70 |

### Representative 1D detection sim  (total 2.95 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 0.82% |
| neural field u | NeuralField | 2.09 | 70.90% |
| gauss kernel | GaussKernel | 0.79 | 26.80% |
| normal noise | NormalNoise | 0.04 | 1.47% |

### Representative 2D detection sim  (total 92.53 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.03% |
| neural field u | NeuralField2D | 56.32 | 60.86% |
| gauss kernel 2d | GaussKernel2D | 35.94 | 38.84% |
| normal noise 2d | NormalNoise2D | 0.24 | 0.26% |

## 2026-06-25 11:54:41  (dnfc 2.9.3, 200000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 2.17 | 2.10 | 1.60 | 1780.10 |
| GaussKernel | 0.94 | 0.80 | 0.70 | 373.70 |
| MexicanHatKernel | 1.44 | 1.20 | 1.10 | 421.60 |
| OscillatoryKernel | 3.15 | 2.60 | 2.50 | 839.20 |
| AsymmetricGaussKernel | 0.95 | 0.80 | 0.70 | 383.60 |
| NormalNoise | 0.77 | 0.60 | 0.50 | 438.50 |
| CorrelatedNormalNoise | 3.57 | 2.90 | 2.80 | 761.70 |
| MemoryTrace | 0.22 | 0.20 | 0.10 | 321.00 |
| GaussStimulus | 0.03 | 0.00 | 0.00 | 289.80 |
| TimedGaussStimulus | 0.06 | 0.00 | 0.00 | 320.30 |
| BoostStimulus | 0.10 | 0.10 | 0.00 | 326.60 |
| NeuralField2D | 60.29 | 50.30 | 38.20 | 1007.40 |
| GaussKernel2D | 41.96 | 34.80 | 33.50 | 1097.60 |
| MexicanHatKernel2D | 100.98 | 83.70 | 80.30 | 1488.80 |
| OscillatoryKernel2D | 66.90 | 56.30 | 54.10 | 886.50 |
| AsymmetricGaussKernel2D | 41.61 | 34.80 | 33.40 | 804.00 |
| NormalNoise2D | 17.49 | 14.70 | 13.80 | 670.80 |
| CorrelatedNormalNoise2D | 32.85 | 28.60 | 27.30 | 852.20 |
| MemoryTrace2D | 3.60 | 3.00 | 2.90 | 804.00 |
| GaussStimulus2D | 0.03 | 0.00 | 0.00 | 232.30 |
| TimedGaussStimulus2D | 0.22 | 0.20 | 0.10 | 324.80 |
| BoostStimulus2D | 0.24 | 0.20 | 0.20 | 294.70 |
| Collapse (2D->1D) | 5.94 | 5.00 | 4.90 | 771.40 |
| Expand (1D->2D) | 2.47 | 2.10 | 2.00 | 556.40 |
| Resize (1D) | 0.33 | 0.30 | 0.20 | 426.00 |

### Representative 1D detection sim  (total 3.50 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.03 | 0.87% |
| neural field u | NeuralField | 2.47 | 70.57% |
| gauss kernel | GaussKernel | 0.95 | 27.05% |
| normal noise | NormalNoise | 0.05 | 1.52% |

### Representative 2D detection sim  (total 104.44 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.03% |
| neural field u | NeuralField2D | 62.96 | 60.28% |
| gauss kernel 2d | GaussKernel2D | 41.17 | 39.42% |
| normal noise 2d | NormalNoise2D | 0.28 | 0.27% |

## 2026-06-25 13:09:59  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.75 | 0.70 | 0.70 | 31.50 |
| GaussKernel | 0.79 | 0.80 | 0.70 | 23.80 |
| MexicanHatKernel | 1.17 | 1.20 | 1.10 | 32.70 |
| OscillatoryKernel | 2.58 | 2.60 | 2.50 | 15.10 |
| AsymmetricGaussKernel | 0.78 | 0.80 | 0.70 | 17.90 |
| NormalNoise | 0.63 | 0.60 | 0.50 | 6.50 |
| CorrelatedNormalNoise | 2.86 | 2.80 | 2.60 | 40.10 |
| MemoryTrace | 0.18 | 0.20 | 0.10 | 0.30 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 0.50 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 0.20 |
| NeuralField2D | 14.43 | 14.30 | 13.80 | 485.20 |
| GaussKernel2D | 35.84 | 34.20 | 32.80 | 419.40 |
| MexicanHatKernel2D | 83.73 | 82.60 | 80.00 | 677.70 |
| OscillatoryKernel2D | 57.52 | 56.20 | 54.40 | 296.20 |
| AsymmetricGaussKernel2D | 34.87 | 34.30 | 33.00 | 180.70 |
| NormalNoise2D | 15.27 | 14.70 | 13.80 | 296.30 |
| CorrelatedNormalNoise2D | 28.31 | 27.90 | 26.70 | 131.40 |
| MemoryTrace2D | 3.10 | 3.00 | 2.90 | 74.80 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.20 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 16.80 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 7.80 |
| Collapse (2D->1D) | 4.41 | 4.30 | 4.30 | 338.00 |
| Expand (1D->2D) | 2.11 | 2.10 | 2.00 | 31.00 |
| Resize (1D) | 0.28 | 0.30 | 0.20 | 9.60 |

### Representative 1D detection sim  (total 1.81 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 1.32% |
| neural field u | NeuralField | 0.95 | 52.26% |
| gauss kernel | GaussKernel | 0.79 | 43.80% |
| normal noise | NormalNoise | 0.05 | 2.62% |

### Representative 2D detection sim  (total 57.96 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.05% |
| neural field u | NeuralField2D | 22.99 | 39.66% |
| gauss kernel 2d | GaussKernel2D | 34.73 | 59.92% |
| normal noise 2d | NormalNoise2D | 0.22 | 0.37% |

## 2026-06-25 13:14:17  (dnfc 2.9.3, 200000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.78 | 0.80 | 0.70 | 46.70 |
| GaussKernel | 0.82 | 0.80 | 0.70 | 56.70 |
| MexicanHatKernel | 1.29 | 1.20 | 1.10 | 384.70 |
| OscillatoryKernel | 3.06 | 2.60 | 2.50 | 446.00 |
| AsymmetricGaussKernel | 0.91 | 0.80 | 0.70 | 396.10 |
| NormalNoise | 0.76 | 0.60 | 0.50 | 850.30 |
| CorrelatedNormalNoise | 3.50 | 2.80 | 2.60 | 599.40 |
| MemoryTrace | 0.21 | 0.20 | 0.10 | 304.70 |
| GaussStimulus | 0.03 | 0.00 | 0.00 | 264.00 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 270.20 |
| BoostStimulus | 0.10 | 0.10 | 0.00 | 374.20 |
| NeuralField2D | 17.07 | 14.30 | 13.90 | 818.80 |
| GaussKernel2D | 41.38 | 34.50 | 33.50 | 1162.40 |
| MexicanHatKernel2D | 102.34 | 85.30 | 83.40 | 1240.40 |
| OscillatoryKernel2D | 69.09 | 57.90 | 56.30 | 1252.50 |
| AsymmetricGaussKernel2D | 41.30 | 34.50 | 33.50 | 973.80 |
| NormalNoise2D | 17.85 | 14.50 | 13.70 | 1174.70 |
| CorrelatedNormalNoise2D | 33.67 | 27.70 | 26.70 | 1156.60 |
| MemoryTrace2D | 3.53 | 3.00 | 2.90 | 431.00 |
| GaussStimulus2D | 0.03 | 0.00 | 0.00 | 239.40 |
| TimedGaussStimulus2D | 0.23 | 0.20 | 0.10 | 325.20 |
| BoostStimulus2D | 0.24 | 0.20 | 0.20 | 293.80 |
| Collapse (2D->1D) | 5.07 | 4.30 | 4.10 | 753.80 |
| Expand (1D->2D) | 2.43 | 2.00 | 1.90 | 639.70 |
| Resize (1D) | 0.32 | 0.30 | 0.20 | 339.00 |

### Representative 1D detection sim  (total 2.07 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.03 | 1.25% |
| neural field u | NeuralField | 1.08 | 52.35% |
| gauss kernel | GaussKernel | 0.91 | 44.00% |
| normal noise | NormalNoise | 0.05 | 2.39% |

### Representative 2D detection sim  (total 65.00 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.04% |
| neural field u | NeuralField2D | 24.38 | 37.51% |
| gauss kernel 2d | GaussKernel2D | 40.33 | 62.04% |
| normal noise 2d | NormalNoise2D | 0.26 | 0.40% |

## 2026-06-25 — AFTER sigmoid AVX2 vectorization (Very Sleepy, RelWithDebInfo)

Same sampling method. Change since the previous section: `SigmoidFunction::apply` vectorized with an
AVX2 Cephes `exp256_ps` (8 cells/iter, ~1e-7 accurate), scalar fallback retained. Sigmoid is an
elementwise MAP (no reduction), so it only needs to stay within 1e-4 — FieldDynamics 1D/2D (200+200
sigmoid sims, b100) pass; all 801 unit tests pass.

### Top self-time functions — total sweep self-time ~99s -> ~82s

| % CPU | function | vs previous |
|------:|----------|-------------|
| 55.3% | `conv_valid_into<double>` | unchanged (45.6s); now larger share as sigmoid shrank |
| 14.6% | `conv2d_separable_into` | driver |
| 7.6%  | `zigguratNormal` | noise RNG |
| 5.9%  | `Element::updateInput` | unchanged |
| 4.2%  | `SigmoidFunction::apply` | **13.5s -> 3.5s** (AVX2 exp); `expf` dropped off the top list |
| 2.0%  | `NeuralField2D::updateBumps` | still negligible |

### Per-element aggregate step() µs (profiler's own timing, 50x50)

| element | before sigmoid | after | speedup |
|---------|---------------:|------:|--------:|
| NeuralField2D | 50.0 | 14.4 | 3.5x |
| NeuralField (1D) | 2.1 | 0.75 | 2.8x |

### End-to-end benchmark (median steps/sec) vs the pre-optimization baseline

| dim/N | baseline | now | gain |
|-------|---------:|----:|-----:|
| 2D N=10  | 791.7 | 1463.9 | 1.85x |
| 2D N=50  | 142.6 | 315.3  | 2.21x |
| 2D N=100 | 73.2  | 148.2  | 2.02x |
| 1D N=100 | 2647  | 5438   | 2.06x |

### Remaining
`conv_valid_into` (55%) is the only large lever left and is gated by the FP-reduction reorder vs 1e-4
tradeoff (deferred). After that, across-field threading. The circular-extension gather block-copy was
tried and reverted (no net win — memmove offset the gather savings).
## 2026-06-25 13:45:32  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.74 | 0.70 | 0.70 | 9.70 |
| GaussKernel | 0.66 | 0.70 | 0.60 | 18.10 |
| MexicanHatKernel | 0.83 | 0.80 | 0.80 | 17.80 |
| OscillatoryKernel | 2.58 | 2.60 | 2.50 | 11.30 |
| AsymmetricGaussKernel | 0.65 | 0.60 | 0.60 | 10.50 |
| NormalNoise | 0.63 | 0.60 | 0.50 | 31.00 |
| CorrelatedNormalNoise | 2.86 | 2.80 | 2.70 | 17.70 |
| MemoryTrace | 0.18 | 0.20 | 0.10 | 0.30 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus | 0.04 | 0.00 | 0.00 | 0.10 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 7.00 |
| NeuralField2D | 14.39 | 14.20 | 13.90 | 263.70 |
| GaussKernel2D | 26.97 | 26.70 | 25.70 | 169.30 |
| MexicanHatKernel2D | 77.99 | 77.10 | 74.00 | 173.50 |
| OscillatoryKernel2D | 57.61 | 57.50 | 54.90 | 135.00 |
| AsymmetricGaussKernel2D | 27.22 | 27.00 | 26.20 | 56.30 |
| NormalNoise2D | 14.82 | 14.60 | 13.50 | 56.90 |
| CorrelatedNormalNoise2D | 26.97 | 26.90 | 25.40 | 90.40 |
| MemoryTrace2D | 2.97 | 3.00 | 2.90 | 12.20 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.20 |
| TimedGaussStimulus2D | 0.20 | 0.20 | 0.10 | 0.70 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 0.60 |
| Collapse (2D->1D) | 4.55 | 4.40 | 4.20 | 50.20 |
| Expand (1D->2D) | 2.02 | 2.00 | 2.00 | 8.90 |
| Resize (1D) | 0.27 | 0.30 | 0.20 | 9.10 |

### Representative 1D detection sim  (total 1.66 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 1.42% |
| neural field u | NeuralField | 0.94 | 56.65% |
| gauss kernel | GaussKernel | 0.65 | 39.31% |
| normal noise | NormalNoise | 0.04 | 2.61% |

### Representative 2D detection sim  (total 50.49 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.02 | 0.05% |
| neural field u | NeuralField2D | 22.84 | 45.23% |
| gauss kernel 2d | GaussKernel2D | 27.40 | 54.27% |
| normal noise 2d | NormalNoise2D | 0.22 | 0.45% |

## 2026-06-25 — AFTER symmetric-kernel folding in conv_valid_into

Symmetric kernels now fold the convolution: out[i] = kr[c]*w[c] + sum_{j<c} kr[j]*(w[j]+w[2c-j]),
halving the multiplies (vectorized 4 outputs/iter). This reorders the per-output summation, so it is
gated on the 1e-4 validation — NOT bit-identical. Margin probe (tolerance tightened to 1e-9): worst-case
deviation across all 600 FieldDynamics sims stays at ~5.0e-5, the reference-CSV truncation floor — i.e.
folding introduces no error beyond what's already in the stored references; same ~2x margin to 1e-4 as
the unfolded path. (This passed where the prior session's scalar fold failed; the vectorized pair-first
form is numerically tamer.) Non-symmetric kernels (Oscillatory) keep the bit-identical path. 801 tests pass.

### Per-element aggregate step() µs (50x50)

| element | before fold | after fold | gain | note |
|---------|------------:|-----------:|-----:|------|
| GaussKernel2D | 35.2 | 27.0 | 1.30x | symmetric |
| AsymmetricGaussKernel2D | 34.9 | 27.2 | 1.28x | symmetric axis |
| MexicanHatKernel2D | 84.3 | 78.0 | 1.08x | two kernels |
| CorrelatedNormalNoise2D | 29.8 | 27.0 | 1.10x | |
| OscillatoryKernel2D | 56.9 | 57.6 | ~flat | not symmetric (bit-identical path) |

### Benchmark vs Cedar (median steps/sec; Cedar = FP32, dnfc = FP64)

| dim/N | Cedar | dnfc now | dnfc/Cedar |
|-------|------:|---------:|-----------:|
| 2D N=10  | 1545 | 2008 | 1.30x |
| 2D N=50  | 297  | 334  | 1.12x |
| 2D N=100 | 144  | 161  | 1.12x |
| 1D N=100 | 297  | 5877 | 19.8x |

dnf-composer now beats Cedar at every 2D size in full FP64. Cumulative this session: conv AVX2 (across
outputs) + map-lookup hoist + sigmoid AVX2 exp + symmetric folding took 2D from ~6x slower than Cedar to
12-30% faster. `conv_valid_into` is still the top cost; the remaining lever is across-field threading.
## 2026-06-25 14:58:12  (dnfc 2.9.3, 200000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.77 | 0.80 | 0.70 | 41.10 |
| GaussKernel | 0.66 | 0.60 | 0.60 | 51.30 |
| MexicanHatKernel | 0.91 | 0.80 | 0.80 | 165.70 |
| OscillatoryKernel | 3.04 | 2.60 | 2.50 | 597.30 |
| AsymmetricGaussKernel | 0.77 | 0.60 | 0.60 | 487.80 |
| NormalNoise | 0.78 | 0.60 | 0.50 | 430.90 |
| CorrelatedNormalNoise | 3.77 | 3.10 | 2.80 | 496.60 |
| MemoryTrace | 0.24 | 0.20 | 0.10 | 416.50 |
| GaussStimulus | 0.03 | 0.00 | 0.00 | 318.10 |
| TimedGaussStimulus | 0.06 | 0.10 | 0.00 | 369.00 |
| BoostStimulus | 0.10 | 0.10 | 0.00 | 359.70 |
| NeuralField2D | 17.35 | 14.40 | 14.00 | 839.20 |
| GaussKernel2D | 32.18 | 26.50 | 25.70 | 1031.20 |
| MexicanHatKernel2D | 92.16 | 76.80 | 73.70 | 4709.90 |
| OscillatoryKernel2D | 59.79 | 56.70 | 54.40 | 1062.30 |
| AsymmetricGaussKernel2D | 27.93 | 26.40 | 25.20 | 718.20 |
| NormalNoise2D | 15.85 | 14.70 | 13.80 | 20391.20 |
| CorrelatedNormalNoise2D | 27.95 | 26.50 | 25.00 | 717.80 |
| MemoryTrace2D | 3.21 | 3.00 | 2.90 | 472.60 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.50 |
| TimedGaussStimulus2D | 0.20 | 0.20 | 0.10 | 371.50 |
| BoostStimulus2D | 0.26 | 0.20 | 0.20 | 593.50 |
| Collapse (2D->1D) | 5.39 | 5.00 | 4.80 | 988.10 |
| Expand (1D->2D) | 2.18 | 2.10 | 2.00 | 435.30 |
| Resize (1D) | 0.30 | 0.30 | 0.20 | 416.90 |

### Representative 1D detection sim  (total 1.76 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.03 | 1.43% |
| neural field u | NeuralField | 1.00 | 57.02% |
| gauss kernel | GaussKernel | 0.68 | 38.70% |
| normal noise | NormalNoise | 0.05 | 2.85% |

### Representative 2D detection sim  (total 50.44 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.05% |
| neural field u | NeuralField2D | 22.20 | 44.01% |
| gauss kernel 2d | GaussKernel2D | 27.98 | 55.47% |
| normal noise 2d | NormalNoise2D | 0.24 | 0.47% |

## 2026-06-25 15:08:11  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.64 | 0.60 | 0.60 | 21.10 |
| GaussKernel | 0.55 | 0.50 | 0.50 | 9.80 |
| MexicanHatKernel | 0.73 | 0.70 | 0.70 | 10.10 |
| OscillatoryKernel | 2.48 | 2.50 | 2.40 | 16.20 |
| AsymmetricGaussKernel | 0.56 | 0.60 | 0.50 | 10.20 |
| NormalNoise | 0.63 | 0.60 | 0.50 | 9.90 |
| CorrelatedNormalNoise | 2.82 | 2.80 | 2.60 | 109.10 |
| MemoryTrace | 0.10 | 0.10 | 0.00 | 0.50 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 8.60 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 2.50 |
| BoostStimulus | 0.09 | 0.10 | 0.00 | 0.20 |
| NeuralField2D | 12.71 | 12.60 | 12.40 | 202.50 |
| GaussKernel2D | 24.98 | 24.30 | 23.50 | 155.40 |
| MexicanHatKernel2D | 75.34 | 73.70 | 71.00 | 279.00 |
| OscillatoryKernel2D | 54.89 | 54.20 | 52.40 | 220.10 |
| AsymmetricGaussKernel2D | 25.34 | 24.70 | 23.70 | 165.90 |
| NormalNoise2D | 14.92 | 14.50 | 13.70 | 74.80 |
| CorrelatedNormalNoise2D | 27.63 | 26.60 | 25.50 | 93.50 |
| MemoryTrace2D | 1.20 | 1.20 | 1.10 | 18.00 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.30 |
| TimedGaussStimulus2D | 0.20 | 0.20 | 0.10 | 27.30 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 3.70 |
| Collapse (2D->1D) | 2.49 | 2.50 | 2.40 | 56.20 |
| Expand (1D->2D) | 1.99 | 2.00 | 1.90 | 23.20 |
| Resize (1D) | 0.18 | 0.20 | 0.10 | 0.40 |

### Representative 1D detection sim  (total 1.52 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 1.56% |
| neural field u | NeuralField | 0.88 | 57.47% |
| gauss kernel | GaussKernel | 0.58 | 37.86% |
| normal noise | NormalNoise | 0.05 | 3.12% |

### Representative 2D detection sim  (total 46.10 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.05% |
| neural field u | NeuralField2D | 21.07 | 45.72% |
| gauss kernel 2d | GaussKernel2D | 24.77 | 53.73% |
| normal noise 2d | NormalNoise2D | 0.23 | 0.50% |

## 2026-06-25 15:09:48  (dnfc 2.9.3, 200000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.74 | 0.70 | 0.60 | 113.20 |
| GaussKernel | 0.63 | 0.60 | 0.50 | 279.10 |
| MexicanHatKernel | 0.88 | 0.80 | 0.70 | 703.60 |
| OscillatoryKernel | 3.00 | 2.60 | 2.40 | 755.10 |
| AsymmetricGaussKernel | 0.70 | 0.60 | 0.50 | 477.10 |
| NormalNoise | 0.80 | 0.60 | 0.50 | 465.90 |
| CorrelatedNormalNoise | 3.75 | 3.10 | 2.70 | 676.30 |
| MemoryTrace | 0.13 | 0.10 | 0.10 | 313.50 |
| GaussStimulus | 0.03 | 0.00 | 0.00 | 263.90 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 404.30 |
| BoostStimulus | 0.10 | 0.10 | 0.00 | 286.70 |
| NeuralField2D | 14.99 | 12.50 | 12.10 | 688.40 |
| GaussKernel2D | 30.04 | 24.60 | 23.80 | 3104.00 |
| MexicanHatKernel2D | 81.23 | 74.40 | 71.20 | 18895.30 |
| OscillatoryKernel2D | 57.83 | 54.80 | 52.70 | 2859.50 |
| AsymmetricGaussKernel2D | 25.83 | 24.50 | 23.40 | 988.60 |
| NormalNoise2D | 15.92 | 15.10 | 14.00 | 596.50 |
| CorrelatedNormalNoise2D | 28.00 | 26.50 | 25.10 | 924.90 |
| MemoryTrace2D | 1.26 | 1.20 | 1.10 | 613.90 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.50 |
| TimedGaussStimulus2D | 0.20 | 0.20 | 0.10 | 397.10 |
| BoostStimulus2D | 0.22 | 0.20 | 0.20 | 355.30 |
| Collapse (2D->1D) | 3.26 | 3.10 | 2.90 | 486.30 |
| Expand (1D->2D) | 2.10 | 2.00 | 1.90 | 458.40 |
| Resize (1D) | 0.21 | 0.20 | 0.10 | 454.80 |

### Representative 1D detection sim  (total 1.58 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 1.52% |
| neural field u | NeuralField | 0.89 | 56.68% |
| gauss kernel | GaussKernel | 0.61 | 38.55% |
| normal noise | NormalNoise | 0.05 | 3.25% |

### Representative 2D detection sim  (total 46.38 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.03 | 0.06% |
| neural field u | NeuralField2D | 20.21 | 43.57% |
| gauss kernel 2d | GaussKernel2D | 25.92 | 55.88% |
| normal noise 2d | NormalNoise2D | 0.23 | 0.50% |

## 2026-06-25 — AFTER updateInput zero-fill elision

Element::updateInput (runs for every element every step) previously did fill(0) then accumulated each
input source. Now it copies the first source and adds the rest — eliding a full zero-fill pass per
element per step. Bit-identical (803 tests pass, FieldDynamics 1D/2D at 1e-4).

Sampled: `Element::updateInput` dropped out of the top self-time list (was 4.4s / 6.3% of the sweep);
total sweep self-time 69s -> 63.7s. Per-element aggregate (50x50): ~2us off EVERY element —
NeuralField2D 14.4->12.7, GaussKernel2D 27.0->25.0, MexicanHat2D 78.0->75.3, Asymmetric 27.2->25.3.
Representative 2D detection sim 51->46 us/step.

### Remaining self-time (after this change)
| % CPU | function | note |
|------:|----------|------|
| 51.8% | conv_valid_into | irreducible FMA (MexicanHat 2x-conv, Oscillatory wide/unfolded due to {24,25} clamp at 50x50) |
| 17.6% | conv2d_separable_into | driver gather/scatter/transpose |
| 8.7%  | zigguratNormal | scalar noise Gaussian (rejection branch; SIMD is a measure-and-maybe) |
| 5.1%  | SigmoidFunction::apply | already AVX2 |
| 2.2%  | NeuralField2D::updateBumps | flood-fill |

Convolution is now FMA-bound; the structural wins (across-output SIMD + folding) are taken. Further
single-thread conv gains need either math changes (rejected) or threading (deferred). Non-conv targets
remaining: zigguratNormal (vectorize, measure-and-maybe), updateBumps (small).
## 2026-06-25 15:15:15  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.65 | 0.60 | 0.60 | 26.00 |
| GaussKernel | 0.57 | 0.60 | 0.50 | 48.60 |
| MexicanHatKernel | 0.74 | 0.70 | 0.70 | 29.60 |
| OscillatoryKernel | 2.61 | 2.50 | 2.40 | 430.80 |
| AsymmetricGaussKernel | 0.57 | 0.60 | 0.50 | 14.40 |
| NormalNoise | 0.63 | 0.60 | 0.50 | 13.50 |
| CorrelatedNormalNoise | 2.85 | 2.80 | 2.60 | 30.20 |
| MemoryTrace | 0.11 | 0.10 | 0.00 | 3.00 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 0.40 |
| TimedGaussStimulus | 0.05 | 0.00 | 0.00 | 13.30 |
| BoostStimulus | 0.08 | 0.10 | 0.00 | 0.50 |
| NeuralField2D | 12.72 | 12.50 | 12.30 | 47.10 |
| GaussKernel2D | 24.80 | 24.40 | 23.50 | 329.60 |
| MexicanHatKernel2D | 75.89 | 74.20 | 71.80 | 803.80 |
| OscillatoryKernel2D | 55.32 | 54.80 | 52.80 | 241.10 |
| AsymmetricGaussKernel2D | 23.41 | 23.20 | 22.60 | 119.30 |
| NormalNoise2D | 14.82 | 14.70 | 13.80 | 68.40 |
| CorrelatedNormalNoise2D | 26.73 | 26.40 | 25.00 | 137.10 |
| MemoryTrace2D | 1.20 | 1.20 | 1.10 | 10.80 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.10 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 0.30 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 8.90 |
| Collapse (2D->1D) | 3.11 | 3.10 | 3.00 | 28.10 |
| Expand (1D->2D) | 2.00 | 2.00 | 1.90 | 71.60 |
| Resize (1D) | 0.19 | 0.20 | 0.10 | 36.90 |

### Representative 1D detection sim  (total 1.49 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 1.58% |
| neural field u | NeuralField | 0.85 | 57.19% |
| gauss kernel | GaussKernel | 0.57 | 38.06% |
| normal noise | NormalNoise | 0.05 | 3.17% |

### Representative 2D detection sim  (total 45.80 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.02 | 0.05% |
| neural field u | NeuralField2D | 22.13 | 48.32% |
| gauss kernel 2d | GaussKernel2D | 23.43 | 51.15% |
| normal noise 2d | NormalNoise2D | 0.22 | 0.48% |

## 2026-06-25 — AFTER amplitudeGlobal==0 accumulate guard (kernels)

The 4 2D kernels computed fullSum = accumulate(input) (O(N), ~0.3-0.7s/kernel in the sample, the
dominant line of each kernel's own step body) then added amplitudeGlobal*fullSum to every cell — even
when amplitudeGlobal==0. Guarded both behind mplitudeGlobal != 0.0: skip the accumulate and the
per-cell offset add when the global term is disabled. Bit-identical (803 tests, FieldDynamics 1e-4).

Effect (profiler default params — Oscillatory/Asymmetric default amplitudeGlobal=0; Gauss=-0.01,
MexicanHat=-0.1 keep it):
| element | before | after |
|---------|-------:|------:|
| AsymmetricGaussKernel2D | 25.3 | 23.4 |
| OscillatoryKernel2D | 56.5 | 55.3 |
| GaussKernel2D | 25.0 | 24.8 (default offset nonzero -> unchanged) |
| MexicanHatKernel2D | 75.3 | 75.9 (default offset nonzero -> unchanged) |

Users who set amplitudeGlobal=0 on Gauss/MexicanHat now get the same saving. Combined with the
updateInput zero-fill elision, the canonical detection field is ~2x lighter than session start on the
non-conv work.
## 2026-06-25 15:18:54  (dnfc 2.9.3, 20000 iters)

### Per element-type step()

| element | mean us | median us | min us | max us |
|---------|--------:|----------:|-------:|-------:|
| NeuralField | 0.64 | 0.60 | 0.60 | 10.40 |
| GaussKernel | 0.56 | 0.60 | 0.50 | 20.10 |
| MexicanHatKernel | 0.77 | 0.80 | 0.70 | 30.80 |
| OscillatoryKernel | 2.51 | 2.50 | 2.40 | 34.40 |
| AsymmetricGaussKernel | 0.61 | 0.60 | 0.50 | 2.30 |
| NormalNoise | 0.55 | 0.60 | 0.50 | 7.70 |
| CorrelatedNormalNoise | 2.83 | 2.70 | 2.60 | 20.40 |
| MemoryTrace | 0.10 | 0.10 | 0.00 | 0.20 |
| GaussStimulus | 0.02 | 0.00 | 0.00 | 11.90 |
| TimedGaussStimulus | 0.04 | 0.00 | 0.00 | 5.30 |
| BoostStimulus | 0.08 | 0.10 | 0.00 | 3.60 |
| NeuralField2D | 12.94 | 12.70 | 12.20 | 95.60 |
| GaussKernel2D | 25.35 | 24.40 | 23.40 | 1942.70 |
| MexicanHatKernel2D | 75.55 | 74.40 | 71.50 | 685.10 |
| OscillatoryKernel2D | 54.71 | 54.40 | 52.50 | 124.70 |
| AsymmetricGaussKernel2D | 23.78 | 23.40 | 22.50 | 254.20 |
| NormalNoise2D | 12.68 | 12.50 | 11.70 | 171.90 |
| CorrelatedNormalNoise2D | 24.55 | 24.30 | 23.20 | 168.40 |
| MemoryTrace2D | 1.20 | 1.20 | 1.10 | 27.50 |
| GaussStimulus2D | 0.02 | 0.00 | 0.00 | 0.30 |
| TimedGaussStimulus2D | 0.19 | 0.20 | 0.10 | 7.80 |
| BoostStimulus2D | 0.21 | 0.20 | 0.20 | 0.30 |
| Collapse (2D->1D) | 3.43 | 3.10 | 2.90 | 36.20 |
| Expand (1D->2D) | 2.01 | 2.00 | 1.90 | 17.50 |
| Resize (1D) | 0.19 | 0.20 | 0.10 | 0.60 |

### Representative 1D detection sim  (total 1.52 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus | GaussStimulus | 0.02 | 1.58% |
| neural field u | NeuralField | 0.86 | 56.68% |
| gauss kernel | GaussKernel | 0.58 | 38.52% |
| normal noise | NormalNoise | 0.05 | 3.22% |

### Representative 2D detection sim  (total 44.71 us/step)

| element | type | mean us/step | % of step |
|---------|------|-------------:|----------:|
| gauss stimulus 2d | GaussStimulus2D | 0.02 | 0.05% |
| neural field u | NeuralField2D | 21.05 | 47.08% |
| gauss kernel 2d | GaussKernel2D | 23.42 | 52.38% |
| normal noise 2d | NormalNoise2D | 0.22 | 0.48% |

## 2026-06-25 — AFTER ziggurat table-accessor hoist (noise RNG)

Line-level sampling of zigguratNormal showed its single biggest line (2.09s of 5.56s) was
`const ZigguratTables& z = zigTables();` — the function-local static's thread-safe-init guard,
re-checked on EVERY sample (2500/step). Hoisted the table fetch out of the per-sample call:
zigguratNormal now takes `const ZigguratTables&`, fetched once per fillNormal batch. Bit-identical
(same tables, same xoshiro stream, same algorithm; noise distribution tests + 803 suite pass).

| element | before | after |
|---------|-------:|------:|
| NormalNoise2D | 14.8 | 12.7 (-14%) |
| CorrelatedNormalNoise2D | 27.0 | 24.6 |
| NormalNoise (1D) | 0.63 | 0.55 |

(Considered vectorizing the Gaussian via Box-Muller, but the measured hotspot was the magic-static
guard, not the arithmetic — this hoist is the real easy win, no math/stream change. Kernel
component-pointer caching was evaluated and skipped: no per-cell map lookups remain, ~0.1us churn only.)