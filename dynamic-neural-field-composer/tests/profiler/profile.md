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