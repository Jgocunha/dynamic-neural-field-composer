# Why the perf tools never run in CI

`dnf_composer_benchmark`, `dnf_composer_deckbench` and `dnf_composer_profiler` are built
and run manually. None of the four CI workflow jobs (`build-and-test-linux/windows/macos`,
`sanitizers-linux`) build or invoke any of them, and `dnf_composer_deckbench --check`
is deliberately not wired into any workflow as a gate.

## Why this is deliberate, not an oversight

GitHub-hosted runners are shared, virtualized VMs with no guaranteed CPU model and no
guaranteed AVX2 availability. Run-to-run wall-clock spread on that kind of runner is
routinely 2-3x — far larger than the ~30% regression size that's actually worth catching.
A `--check` threshold loose enough to not false-positive on that spread would need to be
so wide it stops catching anything. Tightening it instead would make CI red on unrelated
noise regularly enough that people start ignoring the perf check the same week it ships —
and a check nobody trusts is worse than no check, because it still costs review attention.

Two of the six CI jobs (`sanitizers-linux`'s ASan/UBSan and TSan variants) are also
10-50x slower than a normal build, which would make a shared threshold meaningless across
jobs even before considering runner variance.

## What CI *does* still verify

The correctness suite (`dnf_composer_tests`, including the golden and field-dynamics
validation fixtures) runs in every CI job and is unaffected by any of this — an
optimization that changes behavior still fails CI the normal way. Only *speed* regression
detection is kept out of CI; correctness regression detection is not.