"""Discovery and parsing of every perf artifact this repo's tools produce.

Five shapes, all read-only:

  * deckbench run / benchmark run / baseline  -- tests/common/bench_report.h envelope
  * profiler deck-mode JSON                    -- tests/profiler/profiler_main.cpp
  * kernelbench (Google Benchmark) JSON         -- tests/benchmark/kernels/kernelbench_main.cpp
  * legacy results.md                          -- tests/benchmark/results.md
  * legacy profile.md                          -- tests/profiler/profile.md

Everything returns pandas DataFrames so the views never touch raw JSON/regex.
"""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import pandas as pd

BENCH_DIR = Path(__file__).resolve().parent.parent.parent / "tests" / "benchmark"
PROFILER_DIR = Path(__file__).resolve().parent.parent.parent / "tests" / "profiler"

RESULTS_DIR = BENCH_DIR / "results"
BASELINES_DIR = BENCH_DIR / "baselines"
RESULTS_MD = BENCH_DIR / "results.md"
PROFILE_MD = PROFILER_DIR / "profile.md"

# The setMeasureStepDuration/ns-per-cell-step/JSON-sidecar discontinuity documented in
# results.md's own header -- sessions before this are not comparable to later ones.
LEGACY_DISCONTINUITY_DATE = "2026-08-20"

STAT_FIELDS = ("min", "q1", "median", "q3", "max", "n")


# ── mtime-aware cache keys ───────────────────────────────────────────────────


def _tree_signature(*dirs_and_files: Path) -> tuple:
    """A cheap, order-independent fingerprint of file paths + mtimes, used as a
    Streamlit cache key so `@st.cache_data` invalidates when any artifact changes
    without needing a manual refresh button."""
    sig = []
    for p in dirs_and_files:
        if p.is_dir():
            for f in sorted(p.glob("*.json")):
                sig.append((str(f), f.stat().st_mtime_ns))
        elif p.is_file():
            sig.append((str(p), p.stat().st_mtime_ns))
    return tuple(sig)


def data_signature() -> tuple:
    return _tree_signature(RESULTS_DIR, BASELINES_DIR, RESULTS_MD, PROFILE_MD)


# ── bench_report.h envelope (deckbench / benchmark / baseline) ──────────────


def _stats_row(stats: dict | None) -> dict:
    stats = stats or {}
    return {f: stats.get(f) for f in STAT_FIELDS}


def _is_recorded(value) -> bool:
    """bench_env.h's fallback for an unset DNFC_BENCH_* variable is the string
    "unrecorded", so emptiness is not what distinguishes a wrapped run."""
    return bool(value) and str(value) != "unrecorded"


def _classify_envelope(path: Path, doc: dict) -> str:
    if "decks" in doc:
        return "profiler"
    if BASELINES_DIR in path.parents:
        return "baseline"
    config = doc.get("config", {})
    if "manifest" in config:
        return "deckbench"
    return "benchmark"


def _load_envelope(path: Path) -> list[dict]:
    """One row per (deck/config) result inside a bench_report.h envelope."""
    doc = json.loads(path.read_text(encoding="utf-8"))
    kind = _classify_envelope(path, doc)
    env = doc.get("env", {})
    config = doc.get("config", {})
    rows = []
    for r in doc.get("results", []):
        row = {
            "source_file": str(path),
            "source_kind": kind,
            "timestamp": doc.get("timestamp"),
            "fingerprint": doc.get("fingerprint"),
            "name": r.get("name"),
            "tier": r.get("tier"),
            "architecture": r.get("architecture"),
            "path": r.get("path"),
            "field_cells": r.get("field_cells"),
            "noisy": r.get("noisy", False),
            "deck_hash": r.get("deck_hash") or None,
            "hostname": env.get("hostname"),
            "cpu": env.get("cpu"),
            "compiler": env.get("compiler"),
            "cxx_flags": env.get("cxx_flags"),
            "os": env.get("os"),
            "build_type": env.get("build_type"),
            "avx2": env.get("avx2"),
            "git": env.get("git"),
            "git_dirty": env.get("git_dirty"),
            "dnfc_version": env.get("dnfc_version"),
            "affinity": env.get("affinity"),
            "priority": env.get("priority"),
            "power_state": env.get("power_state"),
            # bench_env.h stamps the literal string "unrecorded" (not an empty string)
            # when DNFC_BENCH_AFFINITY/PRIORITY are unset, i.e. when the run did NOT go
            # through scripts/bench.ps1 / bench.sh -- so a plain truthiness test would
            # call every unwrapped run wrapped.
            "wrapped": _is_recorded(env.get("affinity")) and _is_recorded(env.get("priority")),
            "timed_steps": config.get("timed_steps"),
            "runs": config.get("runs"),
        }
        for prefix, block in (("steps_per_sec", r.get("steps_per_sec")), ("ns_per_cell_step", r.get("ns_per_cell_step"))):
            for f, v in _stats_row(block).items():
                row[f"{prefix}_{f}"] = v
        rows.append(row)
    return rows


def load_envelopes() -> pd.DataFrame:
    """All deckbench runs, benchmark runs and baselines, one row per deck result."""
    rows: list[dict] = []
    for d in (RESULTS_DIR, BASELINES_DIR):
        if not d.is_dir():
            continue
        for f in sorted(d.glob("*.json")):
            try:
                rows.extend(_load_envelope(f))
            except (json.JSONDecodeError, KeyError):
                continue
    if not rows:
        return pd.DataFrame()
    df = pd.DataFrame(rows)
    df["timestamp"] = pd.to_datetime(df["timestamp"], errors="coerce", utc=True)
    return df.sort_values("timestamp", na_position="first").reset_index(drop=True)


# ── profiler deck-mode JSON ───────────────────────────────────────────────────


def load_profiler_decks() -> pd.DataFrame:
    rows: list[dict] = []
    if not RESULTS_DIR.is_dir():
        return pd.DataFrame()
    for f in sorted(RESULTS_DIR.glob("profiler_deck_*.json")):
        try:
            doc = json.loads(f.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            continue
        env = doc.get("env", {})
        for deck in doc.get("decks", []):
            for el in deck.get("elements", []):
                rows.append(
                    {
                        "source_file": str(f),
                        "fingerprint": doc.get("fingerprint"),
                        "tier": deck.get("tier"),
                        "deck_path": deck.get("path"),
                        "architecture": deck.get("architecture"),
                        "total_us": deck.get("total_us"),
                        "element_name": el.get("name"),
                        "element_type": el.get("type"),
                        "mean_us": el.get("mean_us"),
                        "share_pct": el.get("share_pct"),
                        "hostname": env.get("hostname"),
                        "cpu": env.get("cpu"),
                        "git": env.get("git"),
                        "mtime": f.stat().st_mtime,
                    }
                )
    if not rows:
        return pd.DataFrame()
    df = pd.DataFrame(rows)
    df["timestamp"] = pd.to_datetime(df["mtime"], unit="s", utc=True)
    return df.sort_values("timestamp").reset_index(drop=True)


# ── kernelbench (Google Benchmark) JSON ──────────────────────────────────────


def load_kernelbench(path: Path) -> pd.DataFrame:
    """Parses a --benchmark_format=json --benchmark_out=<path> file. Prefers the
    'median' aggregate row when --benchmark_repetitions produced aggregates;
    falls back to the raw iteration row otherwise."""
    doc = json.loads(Path(path).read_text(encoding="utf-8"))
    context = doc.get("context", {})
    by_run: dict[str, dict] = {}
    for b in doc.get("benchmarks", []):
        run_name = b.get("run_name", b.get("name"))
        aggregate_name = b.get("aggregate_name")
        entry = by_run.setdefault(run_name, {})
        if aggregate_name == "median":
            entry["median"] = b
        elif aggregate_name is None and "iteration" not in entry:
            entry["iteration"] = b

    known_keys = {
        "name", "run_name", "family_index", "per_family_instance_index", "run_type",
        "repetitions", "repetition_index", "threads", "iterations", "real_time",
        "cpu_time", "time_unit", "aggregate_name", "aggregate_unit",
    }

    rows = []
    for run_name, entry in by_run.items():
        b = entry.get("median") or entry.get("iteration")
        if b is None:
            continue
        parts = run_name.split("/")
        family = parts[0]
        args = parts[1:]
        counters = {k: v for k, v in b.items() if k not in known_keys}
        rows.append(
            {
                "run_name": run_name,
                "family": family,
                "args": "/".join(args) if args else "",
                "arg0": args[0] if len(args) > 0 else None,
                "arg1": args[1] if len(args) > 1 else None,
                "real_time": b.get("real_time"),
                "cpu_time": b.get("cpu_time"),
                "time_unit": b.get("time_unit", "ns"),
                "is_aggregate": "median" in entry,
                **counters,
            }
        )
    df = pd.DataFrame(rows)
    df.attrs["context"] = context
    return df


# ── legacy results.md ────────────────────────────────────────────────────────

_MD_SESSION_RE = re.compile(
    r"^## (?P<ts>\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\s+\(dnfc (?P<version>[\d.]+), "
    r"(?P<steps>\d+) steps x (?P<runs>\d+) runs\)\s*$",
    re.MULTILINE,
)
_MD_ENV_RE = re.compile(r"^\*\*Env:\*\*\s*(?P<env>.+)$", re.MULTILINE)
_MD_TABLE_RE = re.compile(
    r"\| dim \|.*?\n\|[-:| ]+\n((?:\|.*\n?)+)"
)
_MD_TABLE_ROW_RE = re.compile(
    r"^\|\s*(\w+)\s*\|\s*([\d.]+)\s*\|\s*([\d.]+)\s*\|\s*([\d.]+)\s*\|"
)
_MD_CAPTION_RE = re.compile(r"\*\*([^*]+)\*\*")


def _classify_caption(text_before_table: str) -> str:
    caption_match = None
    for m in _MD_CAPTION_RE.finditer(text_before_table):
        caption_match = m
    caption = caption_match.group(1) if caption_match else ""
    if "Calibration" in caption:
        return "ratio"
    if "ns/field-cell" in caption or "ns/cell" in caption:
        return "ns_per_cell_step"
    if "IQR" in caption:
        return "iqr_pct"
    return "steps_per_sec"


def load_legacy_results_md() -> pd.DataFrame:
    if not RESULTS_MD.is_file():
        return pd.DataFrame()
    text = RESULTS_MD.read_text(encoding="utf-8")

    headers = list(_MD_SESSION_RE.finditer(text))
    rows: list[dict] = []
    for i, h in enumerate(headers):
        block_start = h.end()
        block_end = headers[i + 1].start() if i + 1 < len(headers) else len(text)
        block = text[block_start:block_end]

        env_m = _MD_ENV_RE.search(block)
        env_str = env_m.group("env") if env_m else None

        session = {
            "timestamp": h.group("ts"),
            "version": h.group("version"),
            "steps": int(h.group("steps")),
            "runs": int(h.group("runs")),
            "env": env_str,
        }

        base = {"steps_per_sec": {}, "ratio": {}, "ns_per_cell_step": {}, "iqr_pct": {}}
        for tm in _MD_TABLE_RE.finditer(block):
            table_text = tm.group(0)
            kind = _classify_caption(block[: tm.start()])
            for line in table_text.splitlines():
                rm = _MD_TABLE_ROW_RE.match(line)
                if not rm:
                    continue
                dim, n10, n50, n100 = rm.groups()
                base[kind][dim] = {"N10": float(n10), "N50": float(n50), "N100": float(n100)}

        for dim in ("1D", "2D"):
            if dim not in base["steps_per_sec"]:
                continue
            for n_label in ("N10", "N50", "N100"):
                row = dict(session)
                row["dim"] = dim
                row["n_fields"] = n_label
                row["steps_per_sec"] = base["steps_per_sec"].get(dim, {}).get(n_label)
                row["ratio_to_calibration"] = base["ratio"].get(dim, {}).get(n_label)
                row["ns_per_cell_step"] = base["ns_per_cell_step"].get(dim, {}).get(n_label)
                row["iqr_pct"] = base["iqr_pct"].get(dim, {}).get(n_label)
                rows.append(row)

    if not rows:
        return pd.DataFrame()
    df = pd.DataFrame(rows)
    df["timestamp"] = pd.to_datetime(df["timestamp"], errors="coerce")
    df["is_legacy_pre_discontinuity"] = df["timestamp"] < pd.Timestamp(LEGACY_DISCONTINUITY_DATE)
    df["has_ns_per_cell_step"] = df["ns_per_cell_step"].notna()
    return df.sort_values("timestamp").reset_index(drop=True)


# ── legacy profile.md ─────────────────────────────────────────────────────────

_PROF_SESSION_RE = re.compile(
    r"^## (?P<ts>\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\s+\(dnfc (?P<version>[\d.]+), "
    r"(?P<iters>\d+) iters\)\s*$",
    re.MULTILINE,
)
_PROF_TYPE_SECTION_RE = re.compile(r"### Per element-type step\(\)\s*\n\n(.*?)(?=\n###|\Z)", re.DOTALL)
_PROF_TYPE_ROW_RE = re.compile(
    r"^\|\s*([^|]+?)\s*\|\s*([\d.]+)\s*\|\s*([\d.]+)\s*\|\s*([\d.]+)\s*\|\s*([\d.]+)\s*\|", re.MULTILINE
)
_PROF_SIM_SECTION_RE = re.compile(
    r"### Representative (?P<dim>1D|2D) detection sim\s+\(total (?P<total>[\d.]+) us/step\)\s*\n\n(?P<body>.*?)(?=\n###|\Z)",
    re.DOTALL,
)
_PROF_SIM_ROW_RE = re.compile(
    r"^\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*([\d.]+)\s*\|\s*([\d.]+)%\s*\|", re.MULTILINE
)


def load_legacy_profile_md() -> tuple[pd.DataFrame, pd.DataFrame]:
    """Returns (per_element_type, representative_sim) DataFrames."""
    if not PROFILE_MD.is_file():
        return pd.DataFrame(), pd.DataFrame()
    text = PROFILE_MD.read_text(encoding="utf-8")
    headers = list(_PROF_SESSION_RE.finditer(text))

    type_rows: list[dict] = []
    sim_rows: list[dict] = []

    for i, h in enumerate(headers):
        block_start = h.end()
        block_end = headers[i + 1].start() if i + 1 < len(headers) else len(text)
        block = text[block_start:block_end]
        ts = h.group("ts")
        version = h.group("version")
        iters = int(h.group("iters"))

        type_m = _PROF_TYPE_SECTION_RE.search(block)
        if type_m:
            for rm in _PROF_TYPE_ROW_RE.finditer(type_m.group(1)):
                element, mean_us, median_us, min_us, max_us = rm.groups()
                type_rows.append(
                    {
                        "timestamp": ts, "version": version, "iters": iters,
                        "element": element.strip(),
                        "mean_us": float(mean_us), "median_us": float(median_us),
                        "min_us": float(min_us), "max_us": float(max_us),
                    }
                )

        for sm in _PROF_SIM_SECTION_RE.finditer(block):
            dim = sm.group("dim")
            total_us = float(sm.group("total"))
            for rm in _PROF_SIM_ROW_RE.finditer(sm.group("body")):
                element, etype, mean_us, pct = rm.groups()
                sim_rows.append(
                    {
                        "timestamp": ts, "version": version, "iters": iters, "dim": dim,
                        "total_us": total_us,
                        "element": element.strip(), "type": etype.strip(),
                        "mean_us": float(mean_us), "share_pct": float(pct),
                    }
                )

    type_df = pd.DataFrame(type_rows)
    sim_df = pd.DataFrame(sim_rows)
    if not type_df.empty:
        type_df["timestamp"] = pd.to_datetime(type_df["timestamp"], errors="coerce")
        type_df = type_df.sort_values("timestamp").reset_index(drop=True)
    if not sim_df.empty:
        sim_df["timestamp"] = pd.to_datetime(sim_df["timestamp"], errors="coerce")
        sim_df = sim_df.sort_values("timestamp").reset_index(drop=True)
    return type_df, sim_df


# ── deck manifest ─────────────────────────────────────────────────────────────


def load_decks_manifest() -> pd.DataFrame:
    manifest_path = BENCH_DIR / "decks.json"
    if not manifest_path.is_file():
        return pd.DataFrame()
    doc = json.loads(manifest_path.read_text(encoding="utf-8"))
    return pd.DataFrame(doc.get("decks", []))


@dataclass
class DiscoveredData:
    envelopes: pd.DataFrame
    profiler_decks: pd.DataFrame
    legacy_results: pd.DataFrame
    legacy_profile_types: pd.DataFrame
    legacy_profile_sims: pd.DataFrame
    decks_manifest: pd.DataFrame

    @property
    def is_empty(self) -> bool:
        return (
            self.envelopes.empty
            and self.profiler_decks.empty
            and self.legacy_results.empty
            and self.legacy_profile_types.empty
        )


def load_all() -> DiscoveredData:
    legacy_types, legacy_sims = load_legacy_profile_md()
    return DiscoveredData(
        envelopes=load_envelopes(),
        profiler_decks=load_profiler_decks(),
        legacy_results=load_legacy_results_md(),
        legacy_profile_types=legacy_types,
        legacy_profile_sims=legacy_sims,
        decks_manifest=load_decks_manifest(),
    )
