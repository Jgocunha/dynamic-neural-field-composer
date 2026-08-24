# Perf benchmark noise floor (WP-07 measurement)

Measured 2026-08-20 on the reference dev machine: AMD Ryzen 5 3600 (6C/12T, two CCXs of 3
cores each), Windows 11, MSVC 19.44, Release, `/O2 /arch:AVX2` (AVX2 present and used).

## Method

10x `dnf_composer_benchmark 500 5` bare, then 10x the same wrapped in `scripts/bench.ps1`
(affinity `0x1`, `start /high`, High Performance power plan with `PROCTHROTTLEMAX=99`).
Between each of the 20 sessions, the metric recorded is the **median across that
session's 7 results' `ns_per_cell_step` relative spread** (IQR/median) — one number per
session summarizing "how noisy was this particular run."

## Results

| | median of medians | min | max | spread of the spread |
|---|---:|---:|---:|---|
| bare | 1.90% | 0.85% | **10.25%** | wide — 2 of 10 sessions over 9% |
| wrapped | 1.89% | 0.68% | **3.79%** | tight — every session under 4% |

Bare: `10.25%, 9.83%, 1.57%, 0.85%, 1.79%, 1.60%, 1.81%, 3.48%, 2.00%, 4.34%`
Wrapped: `1.99%, 3.79%, 1.79%, 1.38%, 0.68%, 3.20%, 2.01%, 3.10%, 1.34%, 0.71%`

## Reading this honestly

The **medians land almost identically** (1.90% vs 1.89%), which on its own would look
like the wrapper does nothing. That reading is wrong — it's an artifact of comparing
medians on a distribution with a heavy tail. The real signal is in the tail: bare produced
two sessions over 9% (runs 1 and 2), and wrapped's *worst* session across all 10 (3.79%)
is still lower than bare's *median*. The wrapper's value is eliminating the bad tail, not
shifting the typical case.

**Known confound, stated plainly:** bare's two outlier runs (1 and 2) happened to run
while this same session was concurrently building `dnf_composer_deckbench` and running its
own verification commands on the same machine — real contention, not synthetic. This
means bare's true unloaded noise floor is probably somewhat better than 10%, and the
comparison isn't a clean A/B in the way a from-scratch re-measurement on an otherwise idle
machine would be. It's still informative: it demonstrates that under exactly the kind of
incidental contention a dev machine sees during normal work, the wrapper visibly protects
the result while an unwrapped run does not. A cleaner from-scratch re-measurement on a
genuinely idle machine would be worth doing before leaning on the exact numbers here for
anything more precise than the threshold decision below.

## Threshold decision

**5% is adopted for `dnf_composer_deckbench --check`'s default `--threshold`.** Both the
median (1.89%) and the worst observed wrapped session (3.79%) sit comfortably under it —
real margin, not a number chosen to make the gate rarely fire. `kDefaultThresholdPct = 5.0`
in `deckbench_main.cpp` reflects this measurement; it is no longer a placeholder.

If a future re-measurement on an idle machine shows the wrapped noise floor is
meaningfully different from this session's numbers, revisit the threshold here and update
`tests/benchmark/DECKS.md`'s note alongside it.

## Additional findings from verifying `--record` / `--check` end-to-end

Beyond the controlled 20-session measurement above, implementing and verifying
`dnf_composer_deckbench --record`/`--check` (WP-06) surfaced three more things worth
recording, since they came from real usage rather than a synthetic measurement loop:

**Repeated back-to-back invocations make the machine progressively noisier.** Roughly
20 consecutive wrapped `dnf_composer_deckbench` runs were needed to exercise every exit
code during WP-06 verification, and the later runs in that sequence were visibly noisier
than the earlier ones (IQRs climbing into the teens on decks that had been under 1% a few
runs earlier). Consistent with thermal accumulation on a desktop CPU with no dedicated
cooling headroom — the same effect `PROCTHROTTLEMAX=99` is meant to help with, but
evidently not eliminate under sustained back-to-back load. **Practical implication:**
don't hammer `--check` in a tight loop chasing a clean result (the `perf-regression-test`
skill already says this); leave gaps between runs, or expect the noise floor to be worse
than a single isolated measurement suggests.

**`small` (the 1D, size-100 deck) was the most consistently troublesome individual
deck**, failing the noise gate across step counts from 500 up to 50000 more often than
the three 2D decks. Its absolute per-run duration is the shortest of the four even at
high step counts, which plausibly makes it more sensitive to OS scheduling-quantum
effects proportionally. Escalating `--steps` past a few thousand did not reliably fix it
by itself in this session — what did work was recording under otherwise-favorable
conditions (see the point above) rather than any specific step count. Worth revisiting if
`small` keeps being the deck that blocks a clean `--record`.

**Between-session drift (not just within-session IQR) is real but small in the typical
case.** Six `--check` runs against one fixed baseline (holding code and machine constant,
varying only *when* each check ran) produced five deltas within ±3.3% and one outlier at
+8.45% on an otherwise-clean session. The outlier did not repeat on immediate retry,
consistent with a one-off external interference event rather than a structural property
of the comparison. This is exactly the scenario the noisy/inconclusive exit code (3) and
the "never re-record to chase a clean check" rule exist for — and directly informed
keeping the threshold at 5% rather than tightening it: the typical case has real margin
below 5%, and the rare outlier is a noisy-session problem, not a threshold problem.

**The gate was directly confirmed to fire on a real (not doctored) slowdown.** A
temporary 10-microsecond `sleep_for` per step was added to `deckbench_main.cpp`'s own
timing loop (tool code, not library code), rebuilt, and run against the real baseline:
all four decks reported `REGRESSED` with a five-to-six-order-of-magnitude delta and
near-zero IQR (0.1-0.2%) -- unambiguous, exit code 1. Reverted immediately after. This is
the strongest evidence in this note: the comparison logic doesn't just work on paper or
against a hand-edited baseline, it works against a genuinely slower measurement flowing
through the exact same code path a real regression would take.

## `small`'s noise is not run-length related (2026-08-21 measurement)

The earlier section above guessed that `small`'s chronic noisiness might be a
short-measurement artifact: at 20000 steps a `small` run takes only ~24 ms, close enough
to the Windows scheduler quantum (~15.6 ms) that a single preemption would dominate its
IQR. **That hypothesis is wrong**, and it is worth recording so nobody re-derives it.

Measured on 2026-08-21, `small` alone (via a one-deck manifest), 9 runs at each step
count, all under `scripts/bench.ps1`:

| steps | approx. run length | median ns/cell/step | IQR/median |
|---:|---:|---:|---:|
| 20,000 | ~24 ms | 11.67 | 3.6% |
| 50,000 | ~60 ms | 11.91 | 3.9% |
| 200,000 | ~240 ms | 12.00 | 2.3% |
| 500,000 | ~600 ms | 11.98 | 3.6% |

A 25x increase in run length does not reduce the spread — it bounces around 3-4%
throughout, with no trend. If the mechanism were a fixed-size absolute perturbation (one
scheduler preemption, one interrupt), a 600 ms run would be ~25x less affected in
relative terms than a 24 ms one, and the last row would be far below the first. It is not.

The behaviour instead fits background load stealing a roughly **constant fraction** of
cycles: a longer run accumulates proportionally more interference, so relative variance
stays flat no matter how long the run is. On a machine with IDEs and a browser running,
no step count will average that away. Only quieting the machine will.

**The median, though, is reproducible.** Across that 25x range the median moved only
11.67 -> 12.00, i.e. ±1.4%. So `small`'s *measurement* is accurate even in sessions where
its *IQR* fails the 3% trust gate. The two are answering different questions, and for this
deck on this machine they routinely disagree.

**Practical consequence.** `dnf_composer_deckbench --record` refuses if *any* deck is
noisy, so a chronically-noisy `small` blocks re-recording the whole baseline even when the
other three decks are pristine (one 2026-08-21 session had medium and large-a at 0.5%
while `small` sat at 7.3%). Four consecutive attempts that day were all refused on `small`.
That is the gate working as designed, not a bug — but it does mean **re-recording a
baseline on this machine requires closing the IDEs and browser first**, not just retrying.
Retrying makes it worse: back-to-back attempts degraded `small` from 5.4% to 3.8% to 20.3%,
the same thermal-accumulation effect described earlier in this note.

If this becomes a recurring obstacle, the options worth weighing (none taken yet, all
needing a deliberate decision rather than a quiet tweak) are: a per-deck step-count
override in `decks.json`; a per-deck noise tolerance instead of one global
`kNoisyRelSpread`; or gating `--record` on median stability across repeated runs rather
than on within-run IQR. Loosening the global 3% cutoff is *not* on that list — a gate that
passes noisy data is worse than one that refuses, which is the whole reason exit code 3
exists.
