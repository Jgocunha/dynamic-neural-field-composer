# Benchmark dashboard

A read-only Streamlit dashboard over every artifact the perf tools produce (see
[Performance Benchmarking](../../wiki/Performance%20Benchmarking.md) in the wiki
for what those tools are).

## Run it

Double-click `run_dashboard.bat` (Windows) or `run_dashboard.sh` (Linux/macOS). First
run creates a virtual environment at `<repo root>/.dashboard-venv/` and installs
`requirements.txt` into it; later runs skip straight to launching. It opens your
browser to `http://localhost:8501` automatically.

Or, if you already have a Python environment with `requirements.txt` installed:

```bash
streamlit run dashboard.py
```

## What it reads

| Source | Written by |
|---|---|
| `tests/benchmark/results/deckbench_*.json` | `dnf_composer_deckbench` (plain runs — not `--record`/`--check`, see below) |
| `tests/benchmark/results/*.json` (no prefix) | `dnf_composer_benchmark` |
| `tests/benchmark/results/profiler_deck_*.json` | `dnf_composer_profiler --deck`/`--decks` |
| `tests/benchmark/baselines/*.json` | `dnf_composer_deckbench --record` |
| `tests/benchmark/results.md` | Legacy `dnf_composer_benchmark` history, parsed |
| `tests/profiler/profile.md` | Legacy `dnf_composer_profiler` history, parsed |
| A kernelbench JSON you point it at | `dnf_composer_kernelbench --benchmark_format=json --benchmark_out=<path>` |

Nothing is launched from inside the dashboard — see [Why read-only](#why-read-only).

**`--check` writes no JSON of its own.** Looking at `deckbench_main.cpp::main()`, both
`runRecord()` and `runCheck()` `return` before the `writeJson()` call at the bottom of
`main()` — so a `--check` invocation only prints to stdout. The dashboard's Overview page
recomputes the same OK/REGRESSED/NOISY/DECK CHANGED/NO BASELINE verdict itself, from a
plain run's `results/*.json` compared against `baselines/*.json` (`bench_analysis.py`
mirrors `runCheck()`'s exact logic). **Run `dnf_composer_deckbench` with no `--record`/
`--check` flag** to leave a JSON file for the dashboard to show as a trend point.

## Why read-only

A benchmark measured while a Streamlit server and a browser are also running on the same
machine is competing for the exact CPU the hygiene wrapper (`scripts/bench.ps1`/`bench.sh`)
just pinned. The dashboard is for looking at results after the fact, not for producing
them — run the perf tools yourself, ideally through the wrapper script, then hit "Refresh"
in the sidebar.

## Refreshing

The sidebar's Refresh button clears the data cache and re-reads everything from disk.
Caching is keyed on every artifact file's mtime, so a JSON/markdown file that changed on
disk while the dashboard was already open is picked up on the next natural rerun too.

## Pages

| Page | Answers |
|---|---|
| Overview | Did the latest run regress, per deck, against its baseline? |
| Trends | How has each deck's ns/cell/step moved over time, including the legacy history? |
| Decks | Latest per-deck comparison, and the direct-vs-spectral dispatch crossover (`--paths`) |
| Elements | Which element accounts for a deck's time, and what changed since the last profiler run |
| Kernels | Google Benchmark results on individual kernels, direct vs spectral at matching sizes |
| Runs | Every discovered artifact, and a hygiene checklist (dirty tree / unwrapped / noisy) |

## Files

- `dashboard.py` — entry point, sidebar, page registration
- `bench_data.py` — discovery + parsing of all five artifact shapes into `pandas` DataFrames
- `bench_analysis.py` — verdict logic, deliberately mirroring `deckbench_main.cpp`'s `runCheck()`
- `bench_theme.py` — palette (from the logo + `resources/style_light_green_accent.json`), Plotly template, CSS
- `views.py` — the six pages
