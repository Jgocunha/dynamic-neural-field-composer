"""Comparison rules mirrored from the C++ gate, so the dashboard's verdicts agree
with `dnf_composer_deckbench --check` exactly rather than reinventing the logic.

Mirrors:
  - tests/common/bench_report.h      kNoisyRelSpread (3%)
  - tests/benchmark/deckbench_main.cpp  kDefaultThresholdPct (5%), runCheck()

`--check` writes no JSON of its own (see deckbench_main.cpp: runRecord/runCheck
both `return` before the writeJson() call at the bottom of main()) -- so the
dashboard recomputes the same verdict from a plain run's results/*.json plus
baselines/*.json, rather than parsing a --check invocation's stdout.
"""

from __future__ import annotations

import pandas as pd

# Mirrors bench_report::kNoisyRelSpread in tests/common/bench_report.h.
NOISY_REL_SPREAD_PCT = 3.0

# Mirrors kDefaultThresholdPct in tests/benchmark/deckbench_main.cpp -- backed by
# the noise-floor measurement in .claude/notes/perf-noise-floor.md.
DEFAULT_THRESHOLD_PCT = 5.0

PATH_SUFFIXES = ("-auto", "-forcedirect", "-forcespectral")


def is_plain_deckbench_row(name: str) -> bool:
    """True for a deck's plain Auto-mode result (name == tier), false for the
    per-path rows a --paths run also produces (name == tier + one of PATH_SUFFIXES)."""
    return not any(name.endswith(s) for s in PATH_SUFFIXES)


def latest_deckbench_run(envelopes: pd.DataFrame) -> pd.DataFrame:
    """The most recent deckbench run's plain (non---paths) rows, one per deck."""
    if envelopes.empty:
        return envelopes
    df = envelopes[envelopes["source_kind"] == "deckbench"]
    if df.empty:
        return df
    df = df[df["name"].apply(is_plain_deckbench_row)]
    if df.empty:
        return df
    latest_ts = df["timestamp"].max()
    latest_file = df[df["timestamp"] == latest_ts]["source_file"].iloc[0]
    return df[df["source_file"] == latest_file].reset_index(drop=True)


def baseline_for_fingerprint(envelopes: pd.DataFrame, fingerprint: str) -> pd.DataFrame:
    if envelopes.empty:
        return envelopes
    df = envelopes[(envelopes["source_kind"] == "baseline") & (envelopes["fingerprint"] == fingerprint)]
    return df.reset_index(drop=True)


def compute_verdicts(current: pd.DataFrame, baseline: pd.DataFrame, threshold_pct: float = DEFAULT_THRESHOLD_PCT) -> pd.DataFrame:
    """One row per deck in `current`, joined against `baseline` by name, with the
    same verdict precedence as deckbench_main.cpp's runCheck(): a mismatch (no
    baseline entry, or a changed deck hash) refuses comparison; otherwise NOISY
    outranks REGRESSED, which outranks OK."""
    rows = []
    baseline_by_name = {r["name"]: r for _, r in baseline.iterrows()} if not baseline.empty else {}

    for _, r in current.iterrows():
        name = r["name"]
        b = baseline_by_name.get(name)

        if b is None:
            rows.append({**r.to_dict(), "baseline_median": None, "delta_pct": None, "verdict": "NO BASELINE"})
            continue

        b_hash = b.get("deck_hash")
        r_hash = r.get("deck_hash")
        if b_hash and r_hash and b_hash != r_hash:
            rows.append({**r.to_dict(), "baseline_median": None, "delta_pct": None, "verdict": "DECK CHANGED"})
            continue

        # A result whose stats block was absent from the JSON arrives here as NaN, and
        # NaN is truthy while every NaN comparison is False -- so without this guard a
        # missing measurement would compute a NaN delta, fail the `> threshold` test and
        # be reported OK. Refusing the comparison is the same call runCheck() makes for a
        # deck it cannot line up against the baseline.
        b_median = b.get("ns_per_cell_step_median")
        r_median = r.get("ns_per_cell_step_median")
        if pd.isna(b_median) or pd.isna(r_median) or not b_median:
            rows.append({**r.to_dict(), "baseline_median": None, "delta_pct": None, "verdict": "NO BASELINE"})
            continue

        delta_pct = 100.0 * (r_median - b_median) / b_median
        noisy_here = bool(r.get("noisy")) or bool(b.get("noisy"))
        regressed = delta_pct > threshold_pct

        verdict = "NOISY" if noisy_here else ("REGRESSED" if regressed else "OK")
        rows.append({**r.to_dict(), "baseline_median": b_median, "delta_pct": delta_pct, "verdict": verdict})

    return pd.DataFrame(rows)


def overall_exit_code(verdicts: pd.DataFrame) -> int:
    """Mirrors runCheck()'s exit-code precedence: mismatch (2) > noisy (3) > regressed (1) > ok (0)."""
    if verdicts.empty:
        return 2
    v = set(verdicts["verdict"])
    if "NO BASELINE" in v or "DECK CHANGED" in v:
        return 2
    if "NOISY" in v:
        return 3
    if "REGRESSED" in v:
        return 1
    return 0


EXIT_CODE_LABEL = {
    0: "PASS",
    1: "REGRESSED",
    2: "REFUSED",
    3: "INCONCLUSIVE",
}
