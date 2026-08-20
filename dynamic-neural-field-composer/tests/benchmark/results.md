# dnf-composer throughput benchmark

Median steps/second for N independent fields (1D size 100, 2D 50x50). One section appended per run.

Steps/sec is machine-dependent (CPU, AVX2 dispatch, build type all affect it) --
only compare sessions with matching **Env:** lines directly. The calibration
figure and ratio table let you roughly compare sessions across machines.

Sessions before 2026-07-29 predate the **Env:** line and calibration; they all
ran on the reference dev machine (AMD Ryzen 5 3600, MSVC 19.44, /O2 /arch:AVX2,
Windows 11).

Sessions before 2026-08-20 predate `setMeasureStepDuration(false)`, so their steps/sec
numbers have a step discontinuity there (two fewer `steady_clock::now()` calls per step
from that point on), and predate the ns/field-cell/step, IQR, and JSON-sidecar additions.
The JSON sidecar (`results/<timestamp>_<fingerprint>.json`, see `tests/common/bench_report.h`)
is the machine-readable form of each session for programmatic diffing.

## 2026-06-25 09:34:13   (dnfc 2.9.3, 2000 steps x 3 runs)

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 26911.9 | 5164.9 | 2626.6 |
| 2D  | 785.7 | 144.2 | 73.6 |

_(values = median steps/sec)_

## 2026-06-25 09:36:53   (dnfc 2.9.3, 2000 steps x 3 runs)

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 27309.0 | 5284.9 | 2618.5 |
| 2D  | 726.9 | 141.8 | 73.3 |

_(values = median steps/sec)_

## 2026-06-25 10:14:14   (dnfc 2.9.3, 2000 steps x 3 runs)

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 26876.8 | 5284.9 | 2646.5 |
| 2D  | 791.7 | 142.6 | 73.2 |

_(values = median steps/sec)_

## 2026-06-25 11:34:21   (dnfc 2.9.3, 2000 steps x 3 runs)

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 33172.0 | 6489.3 | 3203.2 |
| 2D  | 1081.8 | 204.0 | 98.8 |

_(values = median steps/sec)_

## 2026-06-25 13:12:15   (dnfc 2.9.3, 2000 steps x 3 runs)

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 51000.8 | 10753.1 | 5437.8 |
| 2D  | 1463.9 | 315.3 | 148.2 |

_(values = median steps/sec)_

## 2026-06-25 13:50:18   (dnfc 2.9.3, 2000 steps x 3 runs)

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 61601.4 | 11743.7 | 5877.2 |
| 2D  | 2007.7 | 333.8 | 161.1 |

_(values = median steps/sec)_

## 2026-07-29 08:51:26   (dnfc 2.9.3, 2000 steps x 3 runs)

**Env:** AMD Ryzen 5 3600 6-Core Processor (12T) | Windows | MSVC 19.44 | Release | AVX2: yes | FFTW 3.3.10 | git 071704af

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 79651.8 | 12580.6 | 7875.6 |
| 2D  | 2918.0 | 546.6 | 220.8 |

**Calibration** (1 field, 1D size 100): 836575.1 steps/s

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 0.0952 | 0.0150 | 0.0094 |
| 2D  | 0.0035 | 0.0007 | 0.0003 |

_(values = median steps/sec; second table = ratio to calibration)_

## 2026-07-29 11:05:33   (dnfc 2.9.4, 2000 steps x 3 runs)

**Env:** AMD Ryzen 5 3600 6-Core Processor (12T) | Windows | MSVC 19.44 | Release | AVX2: yes | FFTW 3.3.10 | git cc4fb38b

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 84808.7 | 16019.4 | 7695.2 |
| 2D  | 2726.1 | 538.6 | 228.4 |

**Calibration** (1 field, 1D size 100): 846131.1 steps/s

| dim | N=10 | N=50 | N=100 |
|-----|-----:|-----:|------:|
| 1D  | 0.1002 | 0.0189 | 0.0091 |
| 2D  | 0.0032 | 0.0006 | 0.0003 |

_(values = median steps/sec; second table = ratio to calibration)_
