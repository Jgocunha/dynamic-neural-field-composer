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
