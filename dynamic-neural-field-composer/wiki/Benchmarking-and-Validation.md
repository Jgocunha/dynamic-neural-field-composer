# Benchmarking & Validation

[Jgocunha/dynamic-field-theory-software](https://github.com/Jgocunha/dynamic-field-theory-software)  compares dynamic-neural-field-composer against three other Dynamic Field Theory frameworks (Cedar, Cosivina, cosivina-python) on two axes: **runtime performance** and **numerical correctness**. dnf-composer is the fastest implementation tested at every problem size, and passes all algebraic-equivalence and behavioural-reliability checks.

---

## Throughput

Each benchmark creates *N* independent neural fields and measures wall-clock simulation steps per second (median of 3 runs × 5 000 steps). The relevant comparison is against **Cedar**, the next-fastest framework and the only other C++ implementation:

| N | dnf-composer | Cedar | dnf-composer gain |
|---|---:|---:|---:|
| 10 | 8 190 | 5 798 | +41% |
| 50 | 1 601 | 1 240 | +29% |
| 100 | 794 | 616 | +29% |
| 500 | 142 | 118 | +20% |
| 1000 | 70 | 61 | +15% |

dnf-composer is fastest at every problem size, delivering **~15–41% higher throughput than Cedar** — while running in full **float64** precision (Cedar uses float32). The MATLAB (Cosivina) and Python (cosivina-python) implementations are several times slower than both C++ frameworks.

---

## Algebraic equivalence & behavioural reliability

The validation study runs 100 simulations across 5 architectures (detection, selection, memory, insufficient, multi-peak), each in stimulus-ON and stimulus-OFF phases, and compares the resulting activation fields across frameworks.

- **Algebraic equivalence:** the Cedar-vs-dnf-composer pairs (same activation-function family) all **PASS**, with maximum deviation `max |Δu| ≤ 1×10⁻⁴` — bounded by Cedar's float32 rounding. The Cosivina / cosivina-python sigmoid pairs also pass within float64 tolerances.
- **Behavioural reliability:** **800 / 800 (100%)** comparisons show qualitative agreement across all frameworks, simulation types, phases, and activation-function families — no case where a bump forms in one framework but not another.

---

## Full data

The complete methodology, per-size statistics, cross-family deviation analysis, and figures live in the benchmark repository:

- [Jgocunha/dynamic-field-theory-software](https://github.com/Jgocunha/dynamic-field-theory-software) — performance benchmark and cross-platform validation.

> Note: the published comparison was run against dnf-composer v2.4.1.
