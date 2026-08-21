"""The six dashboard pages. Every page is read-only: it reads whatever JSON/
markdown bench_data.py discovered, and degrades to guidance (the exact command
to run) when a source is empty rather than raising."""

from __future__ import annotations

import json
from pathlib import Path

import pandas as pd
import plotly.graph_objects as go
import plotly.express as px
import streamlit as st

import bench_analysis
import bench_data
import bench_theme

# ── small shared helpers ─────────────────────────────────────────────────────


def _pill_html(text: str, color: str) -> str:
    return f'<span class="dnfc-pill" style="background:{color}">{text}</span>'


def _chip_html(label: str, value: str) -> str:
    return f'<span class="dnfc-chip"><b>{label}</b>&nbsp;{value}</span>'


def _empty_state(command: str, note: str = ""):
    st.info(
        f"No data yet. Run:\n\n```\n{command}\n```\n" + (note or ""),
        icon="🧪",
    )


def _env_chip_row(row: dict):
    chips = []
    if row.get("hostname"):
        chips.append(_chip_html("host", str(row["hostname"])))
    if row.get("cpu"):
        chips.append(_chip_html("cpu", str(row["cpu"])))
    if row.get("compiler"):
        chips.append(_chip_html("compiler", str(row["compiler"])))
    if row.get("git"):
        dirty = " (dirty)" if row.get("git_dirty") else ""
        chips.append(_chip_html("git", f"{row['git']}{dirty}"))
    wrapped = "yes" if row.get("wrapped") else "no"
    chips.append(_chip_html("wrapped", wrapped))
    if row.get("fingerprint"):
        chips.append(_chip_html("fingerprint", str(row["fingerprint"])))
    st.markdown(" ".join(chips), unsafe_allow_html=True)


def _mini_sparkline(df: pd.DataFrame, y_col: str, color: str, height: int = 70) -> go.Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["timestamp"], y=df[y_col], mode="lines",
            line=dict(color=color, width=2), fill="tozeroy",
            fillcolor=bench_theme.hex_to_rgba(color, 0.10),
        )
    )
    fig.update_layout(
        height=height, margin=dict(l=0, r=0, t=0, b=0),
        xaxis=dict(visible=False), yaxis=dict(visible=False),
        showlegend=False, paper_bgcolor="rgba(0,0,0,0)", plot_bgcolor="rgba(0,0,0,0)",
    )
    return fig


def _hero(title: str, subtitle: str):
    st.markdown(
        f"""
        <div class="dnfc-card" style="position:relative; overflow:hidden;">
            <div class="dnfc-hero"></div>
            <div class="dnfc-hero-content">
                <div>
                    <div style="font-size:1.4rem; font-weight:700;">{title}</div>
                    <div style="color:{bench_theme.TEXT_MUTED}; font-size:0.9rem;">{subtitle}</div>
                </div>
            </div>
        </div>
        """,
        unsafe_allow_html=True,
    )


def _interpretation(lines: list[str]):
    """A plain-language read of the data above, not just the charts. Skipped
    entirely when there's nothing yet to interpret."""
    if not lines:
        return
    st.write("")
    st.markdown('<div class="dnfc-section-title">Summary</div>', unsafe_allow_html=True)
    body = "".join(f"<li style='margin-bottom:0.35rem;'>{line}</li>" for line in lines)
    st.markdown(f'<div class="dnfc-card"><ul style="margin:0; padding-left:1.1rem;">{body}</ul></div>', unsafe_allow_html=True)


# ── 1. Overview ───────────────────────────────────────────────────────────────


def overview(data: bench_data.DiscoveredData):
    st.markdown('<div class="dnfc-section-title">Overview</div>', unsafe_allow_html=True)

    latest = bench_analysis.latest_deckbench_run(data.envelopes)
    if latest.empty:
        _hero("No deckbench run found yet", "The regression gate has nothing to show until you record or run it.")
        _empty_state(
            "scripts/bench.ps1 build/release/tests/dnf_composer_deckbench.exe --record",
            "Then run a plain `dnf_composer_deckbench` (no --record/--check) to populate a trend point.",
        )
        return

    fingerprint = latest["fingerprint"].iloc[0]
    _hero(
        f"Latest run — {latest['timestamp'].iloc[0]:%Y-%m-%d %H:%M UTC}",
        f"dnfc {latest['dnfc_version'].iloc[0]}",
    )
    _env_chip_row(latest.iloc[0].to_dict())
    st.write("")

    threshold_pct = st.session_state.get("threshold_pct", bench_analysis.DEFAULT_THRESHOLD_PCT)
    baseline = bench_analysis.baseline_for_fingerprint(data.envelopes, fingerprint)
    verdicts = bench_analysis.compute_verdicts(latest, baseline, threshold_pct)

    if baseline.empty:
        st.warning(
            f"No baseline recorded for fingerprint `{fingerprint}` — every deck below shows NO BASELINE. "
            f"Run `--record` on a known-good tree first.",
            icon="⚠️",
        )
    else:
        # Neither --record nor --check writes a JSON sidecar (both return before the
        # writeJson() at the end of deckbench_main.cpp's main()), so the newest file on
        # disk can easily predate the baseline it is being compared against. That shows
        # up as a stale "REGRESSED" for a run that was superseded -- say so loudly,
        # because the terminal said the gate passed and this would flatly contradict it.
        latest_ts, baseline_ts = latest["timestamp"].iloc[0], baseline["timestamp"].iloc[0]
        if pd.notna(latest_ts) and pd.notna(baseline_ts) and latest_ts < baseline_ts:
            st.warning(
                f"**This run predates the baseline it is being compared against** "
                f"({latest_ts:%Y-%m-%d %H:%M} vs baseline {baseline_ts:%Y-%m-%d %H:%M} UTC), "
                "so the verdicts below are history, not the current state. `--record` and "
                "`--check` write no JSON sidecar, so a passing `--check` leaves nothing here. "
                "Run `dnf_composer_deckbench` with no `--record`/`--check` flag to refresh this page.",
                icon="🕰️",
            )

        # The environment fingerprint covers machine/compiler/flags, but NOT the run
        # configuration or the hygiene state -- so a short, unwrapped run will happily
        # compare against a long, pinned baseline and report the difference as a
        # regression. Say so rather than letting the delta stand unqualified.
        caveats = []
        cur_steps, base_steps = latest["timed_steps"].iloc[0], baseline["timed_steps"].iloc[0]
        cur_runs, base_runs = latest["runs"].iloc[0], baseline["runs"].iloc[0]
        if cur_steps and base_steps and cur_steps != base_steps:
            caveats.append(f"**{cur_steps} timed steps** vs the baseline's **{base_steps}**")
        if cur_runs and base_runs and cur_runs != base_runs:
            caveats.append(f"**{cur_runs} runs** vs the baseline's **{base_runs}**")
        if not latest["wrapped"].iloc[0] and baseline["wrapped"].iloc[0]:
            caveats.append("**not run under scripts/bench**, while the baseline was")
        if caveats:
            st.warning(
                "This run was measured differently from the baseline it is being compared against — "
                + "; ".join(caveats)
                + ". ns/cell/step is a rate, so it is nominally comparable, but short or unpinned runs "
                "carry a much wider noise floor. Treat the deltas below as indicative only.",
                icon="⚖️",
            )

    history = data.envelopes[
        (data.envelopes["source_kind"] == "deckbench")
        & (data.envelopes["fingerprint"] == fingerprint)
        & (data.envelopes["name"].apply(bench_analysis.is_plain_deckbench_row))
    ]

    cols = st.columns(len(verdicts))
    for col, (_, row) in zip(cols, verdicts.iterrows()):
        with col:
            with st.container(border=True):
                color = bench_theme.color_for_deck(row["name"])
                st.markdown(f"**{row['name']}**", unsafe_allow_html=True)
                st.markdown(_pill_html(row["verdict"], bench_theme.verdict_color(row["verdict"])), unsafe_allow_html=True)
                st.markdown(
                    f'<div class="dnfc-metric-value dnfc-mono">{row["ns_per_cell_step_median"]:.2f}</div>'
                    f'<div class="dnfc-metric-label">ns / cell / step</div>',
                    unsafe_allow_html=True,
                )
                if row["delta_pct"] is not None:
                    sign = "+" if row["delta_pct"] >= 0 else ""
                    st.caption(f"{sign}{row['delta_pct']:.2f}% vs baseline")
                if row.get("noisy"):
                    st.caption(f"⚠ IQR too wide to trust ({bench_analysis.NOISY_REL_SPREAD_PCT:.0f}% cutoff)")

                deck_hist = history[history["name"] == row["name"]].sort_values("timestamp")
                if len(deck_hist) >= 2:
                    st.plotly_chart(_mini_sparkline(deck_hist, "ns_per_cell_step_median", color), width="stretch", config={"displayModeBar": False})

    overall = bench_analysis.overall_exit_code(verdicts)
    label = bench_analysis.EXIT_CODE_LABEL[overall]
    st.write("")
    if overall == 0:
        st.success(f"Gate: {label} — every deck within {threshold_pct:.1f}%.", icon="✅")
    elif overall == 1:
        st.error(f"Gate: {label} — see the REGRESSED deck(s) above.", icon="⛔")
    elif overall == 3:
        st.warning(f"Gate: {label} — a result is too noisy to trust. Re-run under scripts/bench.", icon="🌫️")
    else:
        st.info(f"Gate: {label} — no baseline, or a deck changed since it was recorded.", icon="🧭")

    deck_paths = dict(zip(data.decks_manifest["tier"], data.decks_manifest["path"])) if not data.decks_manifest.empty else {}
    ok = verdicts[verdicts["verdict"] == "OK"]
    regressed = verdicts[verdicts["verdict"] == "REGRESSED"]
    noisy_v = verdicts[verdicts["verdict"] == "NOISY"]
    unresolved = verdicts[verdicts["verdict"].isin(["NO BASELINE", "DECK CHANGED"])]

    summary_lines = []
    if len(ok):
        summary_lines.append(
            f"<b>{len(ok)} of {len(verdicts)}</b> deck(s) are within the {threshold_pct:.1f}% threshold: "
            + ", ".join(f"<code>{n}</code>" for n in ok["name"]) + "."
        )
    for _, r in regressed.iterrows():
        cmd = f"dnf_composer_profiler --deck tests/validation/data/{deck_paths.get(r['name'], '<deck path>')}"
        summary_lines.append(
            f"<code>{r['name']}</code> regressed by <b>{r['delta_pct']:+.1f}%</b> "
            f"({r['baseline_median']:.2f} → {r['ns_per_cell_step_median']:.2f} ns/cell/step) — "
            f"profile it next: <code>{cmd}</code> to see which element moved."
        )
    for _, r in noisy_v.iterrows():
        summary_lines.append(
            f"<code>{r['name']}</code>'s result is too noisy to trust (IQR/median over the "
            f"{bench_analysis.NOISY_REL_SPREAD_PCT:.0f}% cutoff) — re-run under scripts/bench.ps1 / "
            f"bench.sh before trusting its verdict."
        )
    for _, r in unresolved.iterrows():
        reason = "no baseline has been recorded for this fingerprint yet" if r["verdict"] == "NO BASELINE" else "the deck file changed since the baseline was recorded"
        summary_lines.append(f"<code>{r['name']}</code> can't be compared — {reason}.")
    if len(ok) == len(verdicts) and len(verdicts) > 0:
        summary_lines.append("Nothing to act on right now — safe to proceed.")

    _interpretation(summary_lines)


# ── 2. Trends ─────────────────────────────────────────────────────────────────


def trends(data: bench_data.DiscoveredData):
    st.markdown('<div class="dnfc-section-title">Trends</div>', unsafe_allow_html=True)

    df = data.envelopes[
        (data.envelopes["source_kind"] == "deckbench")
        & (data.envelopes["name"].apply(bench_analysis.is_plain_deckbench_row))
    ] if not data.envelopes.empty else pd.DataFrame()

    if df.empty and data.legacy_results.empty:
        _empty_state("build/release/tests/dnf_composer_deckbench.exe", "Run it a few times (ideally on different commits) to build a trend.")
        return

    fingerprints = sorted(df["fingerprint"].dropna().unique()) if not df.empty else []
    metric_label = st.radio("Metric", ["ns / cell / step", "steps / sec"], horizontal=True)
    metric_col = "ns_per_cell_step" if metric_label.startswith("ns") else "steps_per_sec"

    selected_fp = None
    if fingerprints:
        default_fp = st.session_state.get("selected_fingerprint") or fingerprints[-1]
        selected_fp = st.selectbox("Fingerprint", fingerprints, index=fingerprints.index(default_fp) if default_fp in fingerprints else 0)
        df = df[df["fingerprint"] == selected_fp]

    if not df.empty:
        fig = go.Figure()
        baseline = bench_analysis.baseline_for_fingerprint(data.envelopes, selected_fp) if selected_fp else pd.DataFrame()
        threshold_pct = st.session_state.get("threshold_pct", bench_analysis.DEFAULT_THRESHOLD_PCT)

        for name, g in df.groupby("name"):
            g = g.sort_values("timestamp")
            color = bench_theme.color_for_deck(name)
            fig.add_trace(go.Scatter(
                x=pd.concat([g["timestamp"], g["timestamp"][::-1]]),
                y=pd.concat([g[f"{metric_col}_q3"], g[f"{metric_col}_q1"][::-1]]),
                fill="toself", fillcolor=bench_theme.hex_to_rgba(color, 0.10), line=dict(color="rgba(0,0,0,0)"),
                showlegend=False, hoverinfo="skip",
            ))
            fig.add_trace(go.Scatter(
                x=g["timestamp"], y=g[f"{metric_col}_median"], mode="lines+markers",
                name=name, line=dict(color=color, width=2), marker=dict(size=5),
            ))

            if not baseline.empty and metric_col == "ns_per_cell_step":
                b_row = baseline[baseline["name"] == name]
                if not b_row.empty:
                    b_median = b_row[f"{metric_col}_median"].iloc[0]
                    fig.add_hline(y=b_median, line=dict(color=color, dash="dash", width=1.5))
                    fig.add_hrect(
                        y0=b_median, y1=b_median * (1 + threshold_pct / 100.0),
                        fillcolor=color, opacity=0.06, line_width=0,
                    )

        fig.update_layout(
            height=460, yaxis_title=metric_label,
            legend=dict(orientation="h", yanchor="bottom", y=1.02),
        )
        st.plotly_chart(fig, width="stretch")
        st.caption("Shaded band = Q1–Q3. Dashed line = recorded baseline. Light shading above it = the regression threshold.")
    else:
        st.caption("No deckbench JSON runs yet — showing legacy history only below.")

    if not data.legacy_results.empty:
        st.markdown('<div class="dnfc-section-title">Legacy history (results.md)</div>', unsafe_allow_html=True)
        legacy = data.legacy_results.copy()
        y_col = "ns_per_cell_step" if metric_col == "ns_per_cell_step" else "steps_per_sec"
        if y_col == "ns_per_cell_step":
            legacy = legacy[legacy["has_ns_per_cell_step"]]
        if legacy.empty:
            st.caption("No legacy sessions carry this metric (ns/cell/step only exists from 2026-08-20 onward).")
        else:
            fig2 = go.Figure()
            for (dim, n_fields), g in legacy.groupby(["dim", "n_fields"]):
                g = g.sort_values("timestamp")
                label = f"{dim} {n_fields}"
                fig2.add_trace(go.Scatter(
                    x=g["timestamp"], y=g[y_col], mode="lines+markers", name=label,
                    line=dict(dash="dot", width=1.5), marker=dict(size=5, symbol="diamond"),
                ))
            fig2.add_vline(
                x=pd.Timestamp(bench_data.LEGACY_DISCONTINUITY_DATE).timestamp() * 1000,
                line=dict(color=bench_theme.TEXT_MUTED, dash="dot"),
                annotation_text="measurement changed shape",
            )
            fig2.update_layout(height=380, yaxis_title=y_col)
            st.plotly_chart(fig2, width="stretch")
            st.caption(
                "`dnf_composer_benchmark`'s synthetic N-fields sweep, parsed from results.md. "
                "Not directly comparable to the deckbench decks above (different architecture family)."
            )

    trend_lines = []
    if not df.empty:
        for name, g in df.groupby("name"):
            g = g.sort_values("timestamp")
            if len(g) < 2:
                continue
            first = g[f"{metric_col}_median"].iloc[0]
            last = g[f"{metric_col}_median"].iloc[-1]
            if not first:
                continue
            pct = 100.0 * (last - first) / first
            if metric_col == "ns_per_cell_step":
                verdict_word = "slower" if pct > 0 else "faster"
            else:
                verdict_word = "faster" if pct > 0 else "slower"
            flag = " — worth a closer look" if abs(pct) > bench_analysis.DEFAULT_THRESHOLD_PCT else ""
            trend_lines.append(
                f"<code>{name}</code>: {abs(pct):.1f}% {verdict_word} across {len(g)} runs "
                f"({first:.2f} → {last:.2f}){flag}."
            )
        if not trend_lines:
            trend_lines.append("Every deck only has one run so far at this fingerprint — run deckbench again to start a trend.")
    if not data.legacy_results.empty:
        pre = data.legacy_results[data.legacy_results["is_legacy_pre_discontinuity"]]
        if not pre.empty:
            trend_lines.append(
                f"The legacy history spans a measurement change on {bench_data.LEGACY_DISCONTINUITY_DATE} "
                "(setMeasureStepDuration, ns/cell/step, JSON sidecar) — don't compare sessions across that line."
            )
    _interpretation(trend_lines)


# ── 3. Decks ──────────────────────────────────────────────────────────────────


def decks(data: bench_data.DiscoveredData):
    st.markdown('<div class="dnfc-section-title">Decks</div>', unsafe_allow_html=True)

    latest = bench_analysis.latest_deckbench_run(data.envelopes)
    if latest.empty:
        _empty_state("build/release/tests/dnf_composer_deckbench.exe")
    else:
        fig = px.bar(
            latest.sort_values("name"), x="name", y="ns_per_cell_step_median", color="path",
            color_discrete_sequence=list(bench_theme.DECK_COLORS.values()),
            labels={"ns_per_cell_step_median": "ns / cell / step", "name": "deck", "path": "convolution path"},
        )
        fig.update_layout(height=400)
        st.plotly_chart(fig, width="stretch")

        _interpretation([
            f"<code>{r['name']}</code> uses the <b>{r['path']}</b> path ({r['ns_per_cell_step_median']:.2f} ns/cell/step)."
            for _, r in latest.sort_values("name").iterrows()
        ])

    if not data.decks_manifest.empty:
        st.markdown('<div class="dnfc-section-title">Deck manifest</div>', unsafe_allow_html=True)
        manifest = data.decks_manifest.copy()
        if not latest.empty:
            hash_by_tier = dict(zip(latest["tier"], latest["deck_hash"]))
            baseline_fp = latest["fingerprint"].iloc[0] if not latest.empty else None
            baseline = bench_analysis.baseline_for_fingerprint(data.envelopes, baseline_fp)
            baseline_hash_by_tier = dict(zip(baseline["tier"], baseline["deck_hash"])) if not baseline.empty else {}
            manifest["current_hash"] = manifest["tier"].map(hash_by_tier)
            manifest["baseline_hash"] = manifest["tier"].map(baseline_hash_by_tier)
            manifest["hash_matches"] = manifest.apply(
                lambda r: None if not r["baseline_hash"] else (r["current_hash"] == r["baseline_hash"]), axis=1
            )
        st.dataframe(manifest, width="stretch", hide_index=True)

    # Dispatch crossover: --paths runs leave rows named "<tier>-auto"/"-forcedirect"/"-forcespectral".
    paths_df = data.envelopes[
        (data.envelopes["source_kind"] == "deckbench")
        & (~data.envelopes["name"].apply(bench_analysis.is_plain_deckbench_row))
    ] if not data.envelopes.empty else pd.DataFrame()

    st.markdown('<div class="dnfc-section-title">Dispatch crossover (--paths)</div>', unsafe_allow_html=True)
    if paths_df.empty:
        _empty_state("build/release/tests/dnf_composer_deckbench.exe --paths", "Times each large-* deck under Auto, ForceDirect and ForceSpectral.")
        return

    latest_ts = paths_df["timestamp"].max()
    paths_df = paths_df[paths_df["timestamp"] == latest_ts].copy()
    paths_df["tier"] = paths_df["name"].str.replace(r"-(auto|forcedirect|forcespectral)$", "", regex=True)
    paths_df["mode"] = paths_df["name"].str.extract(r"-(auto|forcedirect|forcespectral)$")[0]

    crossover_lines = []
    for tier, g in paths_df.groupby("tier"):
        auto_row = g[g["mode"] == "auto"]
        direct_row = g[g["mode"] == "forcedirect"]
        spectral_row = g[g["mode"] == "forcespectral"]
        observed = auto_row["path"].iloc[0] if not auto_row.empty else "unknown"
        fig = go.Figure(go.Bar(
            x=g["mode"], y=g["ns_per_cell_step_median"],
            marker_color=[bench_theme.color_for_deck(tier) if m == "auto" else bench_theme.UNKNOWN_COLOR for m in g["mode"]],
        ))
        fig.update_layout(height=280, title=f"{tier} — Auto observed: {observed}", yaxis_title="ns / cell / step")
        st.plotly_chart(fig, width="stretch")

        manifest_row = data.decks_manifest[data.decks_manifest["tier"] == tier] if not data.decks_manifest.empty else pd.DataFrame()
        expected = manifest_row["expect_path"].iloc[0] if not manifest_row.empty else None

        # Auto's path is inferred purely from which forced timing its own timing sits
        # closer to. When any of the three is noisy, or when the two forced paths are
        # closer together than the noise, that inference cannot support a conclusion --
        # so don't report a "mismatch" that is really just a wide IQR.
        any_noisy = bool(g["noisy"].any())
        separation_pct = None
        if not direct_row.empty and not spectral_row.empty:
            d = direct_row["ns_per_cell_step_median"].iloc[0]
            s = spectral_row["ns_per_cell_step_median"].iloc[0]
            if min(d, s) > 0:
                separation_pct = 100.0 * abs(d - s) / min(d, s)
        rel_spreads = (g["ns_per_cell_step_q3"] - g["ns_per_cell_step_q1"]) / g["ns_per_cell_step_median"]
        worst_spread = float(rel_spreads.max() * 100.0) if rel_spreads.notna().any() else None

        inconclusive = any_noisy or (
            separation_pct is not None and worst_spread is not None and separation_pct < worst_spread
        )

        if inconclusive:
            detail = f"the two forced paths differ by only {separation_pct:.1f}%" if separation_pct is not None else "the forced paths were not both measured"
            spread_txt = f", while the widest IQR here is {worst_spread:.1f}%" if worst_spread is not None else ""
            crossover_lines.append(
                f"<code>{tier}</code>: <b>inconclusive</b> — {detail}{spread_txt}, so which path Auto took "
                f"cannot be read off these timings (it appears to be {observed}, expected {expected}). "
                f"Re-run <code>--paths</code> under scripts/bench with the full step count before treating "
                f"this as a dispatch finding."
            )
        elif expected and observed != "unknown" and observed != expected:
            crossover_lines.append(
                f"<b>Mismatch on <code>{tier}</code></b>: Auto dispatched to {observed} but {expected} was expected — "
                f"either the dispatch rule or tests/validation/data/2d_spectral/README.md is now wrong. "
                f"Investigate before trusting this run."
            )
        elif not direct_row.empty and not spectral_row.empty:
            d = direct_row["ns_per_cell_step_median"].iloc[0]
            s = spectral_row["ns_per_cell_step_median"].iloc[0]
            faster_path = "direct-2d" if d < s else "spectral-2d"
            gain_pct = 100.0 * abs(d - s) / max(d, s)
            confirmed = f"<code>{tier}</code>: Auto chose <b>{observed}</b>, as expected."
            if faster_path == observed:
                crossover_lines.append(f"{confirmed} It is also the faster of the two forced paths, by {gain_pct:.1f}%.")
            else:
                # Auto followed the tap-count rule, but the other path measured faster --
                # that is a statement about where kFFTTapThreshold sits, not a dispatch bug.
                crossover_lines.append(
                    f"{confirmed} Note though that <b>{faster_path} measured {gain_pct:.1f}% faster</b> here, "
                    f"so the crossover for this deck's tap count may sit below "
                    f"<code>kFFTTapThreshold</code>. Worth a look before treating the threshold as tuned."
                )
    _interpretation(crossover_lines)


# ── 4. Elements ───────────────────────────────────────────────────────────────


def elements(data: bench_data.DiscoveredData):
    st.markdown('<div class="dnfc-section-title">Elements</div>', unsafe_allow_html=True)

    if data.profiler_decks.empty and data.legacy_profile_types.empty:
        _empty_state("build/release/tests/dnf_composer_profiler.exe --decks tests/benchmark/decks.json")
        return

    element_lines = []
    if not data.profiler_decks.empty:
        latest_ts = data.profiler_decks["timestamp"].max()
        latest = data.profiler_decks[data.profiler_decks["timestamp"] == latest_ts]

        st.markdown('<div class="dnfc-section-title">Per-element share (latest run)</div>', unsafe_allow_html=True)
        fig = px.treemap(
            latest, path=["tier", "element_name"], values="mean_us", color="share_pct",
            color_continuous_scale=[bench_theme.OK_COLOR, bench_theme.NOISY_COLOR, bench_theme.REGRESSED_COLOR],
        )
        fig.update_layout(height=420)
        st.plotly_chart(fig, width="stretch")

        timestamps = sorted(data.profiler_decks["timestamp"].unique())
        if len(timestamps) >= 2:
            prev = data.profiler_decks[data.profiler_decks["timestamp"] == timestamps[-2]]
            merged = latest.merge(prev, on=["tier", "element_name"], suffixes=("_now", "_prev"))
            merged["delta_us"] = merged["mean_us_now"] - merged["mean_us_prev"]
            merged = merged.sort_values("delta_us", ascending=False)
            st.markdown('<div class="dnfc-section-title">What changed since the previous run</div>', unsafe_allow_html=True)
            fig2 = px.bar(
                merged, x="delta_us", y="element_name", color="tier", orientation="h",
                color_discrete_sequence=list(bench_theme.DECK_COLORS.values()),
                labels={"delta_us": "Δ mean us"},
            )
            fig2.update_layout(height=max(240, 24 * len(merged)))
            st.plotly_chart(fig2, width="stretch")

            # The merge is on (tier, element_name), so it comes back empty whenever the two
            # runs share no deck -- e.g. one profiled a single --deck and the other a full
            # manifest. .iloc[0] would raise there.
            if not merged.empty:
                biggest = merged.reindex(merged["delta_us"].abs().sort_values(ascending=False).index).iloc[0]
                direction = "grew" if biggest["delta_us"] > 0 else "shrank"
                element_lines.append(
                    f"Biggest change since the previous profiler run: <code>{biggest['element_name']}</code> in "
                    f"<code>{biggest['tier']}</code> {direction} by {abs(biggest['delta_us']):.2f} us."
                )

        for tier, g in latest.groupby("tier"):
            top = g.sort_values("share_pct", ascending=False).iloc[0]
            element_lines.append(
                f"<code>{tier}</code>: <code>{top['element_name']}</code> dominates at {top['share_pct']:.1f}% "
                f"of step time ({top['mean_us']:.2f} us)."
            )

    _interpretation(element_lines)

    if not data.legacy_profile_types.empty:
        st.markdown('<div class="dnfc-section-title">Legacy per-element-type trend (profile.md)</div>', unsafe_allow_html=True)
        elements_available = sorted(data.legacy_profile_types["element"].unique())
        default = [e for e in ("NeuralField2D", "GaussKernel2D", "MexicanHatKernel2D") if e in elements_available] or elements_available[:3]
        chosen = st.multiselect("Elements", elements_available, default=default)
        if chosen:
            g = data.legacy_profile_types[data.legacy_profile_types["element"].isin(chosen)]
            fig3 = px.line(g, x="timestamp", y="mean_us", color="element", markers=True)
            fig3.update_layout(height=380, yaxis_title="mean us / step")
            st.plotly_chart(fig3, width="stretch")

    if not data.legacy_profile_sims.empty:
        st.markdown('<div class="dnfc-section-title">Legacy representative-sim share (profile.md)</div>', unsafe_allow_html=True)
        dim = st.radio("Dimension", sorted(data.legacy_profile_sims["dim"].unique()), horizontal=True)
        g = data.legacy_profile_sims[data.legacy_profile_sims["dim"] == dim]
        fig4 = px.area(g, x="timestamp", y="share_pct", color="element", groupnorm=None)
        fig4.update_layout(height=380, yaxis_title="% of step")
        st.plotly_chart(fig4, width="stretch")


# ── 5. Kernels ────────────────────────────────────────────────────────────────


def kernels(data: bench_data.DiscoveredData):
    st.markdown('<div class="dnfc-section-title">Kernels</div>', unsafe_allow_html=True)
    st.caption(
        "dnf_composer_kernelbench measures hot kernels in isolation — hot caches, no surrounding "
        "simulation. Useful for A/B-ing a kernel rewrite; not a substitute for the deckbench gate."
    )

    default_candidates = sorted(bench_data.RESULTS_DIR.glob("kernelbench*.json")) if bench_data.RESULTS_DIR.is_dir() else []
    path_str = st.text_input(
        "kernelbench JSON path",
        value=str(default_candidates[-1]) if default_candidates else "",
        placeholder="tests/benchmark/results/kernelbench.json",
        help="dnf_composer_kernelbench --benchmark_format=json --benchmark_out=<path> --benchmark_repetitions=5",
    )
    uploaded = st.file_uploader("...or drop a kernelbench JSON file here", type="json")

    df = None
    if uploaded is not None:
        try:
            df = bench_data.parse_kernelbench(json.loads(uploaded.getvalue()))
        except (json.JSONDecodeError, UnicodeDecodeError) as exc:
            st.error(f"That file is not readable as Google Benchmark JSON: {exc}", icon="🚫")
            return
    elif path_str and Path(path_str).is_file():
        try:
            df = bench_data.load_kernelbench(Path(path_str))
        except (json.JSONDecodeError, UnicodeDecodeError, OSError) as exc:
            st.error(f"Could not read {path_str}: {exc}", icon="🚫")
            return

    if df is None or df.empty:
        _empty_state(
            "build/release/tests/dnf_composer_kernelbench.exe --benchmark_repetitions=5 "
            "--benchmark_format=json --benchmark_out=tests/benchmark/results/kernelbench.json"
        )
        return

    st.markdown('<div class="dnfc-section-title">ns per call, by kernel and size</div>', unsafe_allow_html=True)
    fig = px.bar(
        df.sort_values(["family", "args"]), x="args", y="real_time", color="family", barmode="group",
        color_discrete_sequence=list(bench_theme.DECK_COLORS.values()) + bench_theme.DECK_COLOR_FALLBACKS,
        labels={"real_time": f"time ({df['time_unit'].iloc[0]})", "args": "args (size/taps)"},
    )
    fig.update_layout(height=420)
    st.plotly_chart(fig, width="stretch")

    direct = df[df["family"] == "BM_Conv2dSeparable"]
    spectral = df[df["family"] == "BM_Conv2dSpectral"]
    if not direct.empty and not spectral.empty:
        st.markdown('<div class="dnfc-section-title">Direct vs spectral, matching args</div>', unsafe_allow_html=True)
        merged = direct.merge(spectral, on="args", suffixes=("_direct", "_spectral"))
        if not merged.empty:
            fig2 = go.Figure()
            fig2.add_trace(go.Bar(x=merged["args"], y=merged["real_time_direct"], name="direct", marker_color=bench_theme.color_for_deck("large-a")))
            fig2.add_trace(go.Bar(x=merged["args"], y=merged["real_time_spectral"], name="spectral", marker_color=bench_theme.color_for_deck("large-b")))
            fig2.update_layout(height=340, barmode="group", yaxis_title=f"time ({df['time_unit'].iloc[0]})")
            st.plotly_chart(fig2, width="stretch")

    st.markdown('<div class="dnfc-section-title">Raw results</div>', unsafe_allow_html=True)
    st.dataframe(df, width="stretch", hide_index=True)

    kernel_lines = []
    med_by_family = df.groupby("family")["real_time"].median().sort_values()
    if len(med_by_family) >= 2:
        unit = df["time_unit"].iloc[0]
        fastest, slowest = med_by_family.index[0], med_by_family.index[-1]
        kernel_lines.append(
            f"<code>{fastest}</code> is the fastest kernel measured (median {med_by_family.iloc[0]:.1f} {unit}); "
            f"<code>{slowest}</code> is the slowest (median {med_by_family.iloc[-1]:.1f} {unit})."
        )
    if not direct.empty and not spectral.empty:
        merged = direct.merge(spectral, on="args", suffixes=("_direct", "_spectral"))
        if not merged.empty:
            wins_direct = int((merged["real_time_direct"] < merged["real_time_spectral"]).sum())
            wins_spectral = len(merged) - wins_direct
            if wins_direct and wins_spectral:
                kernel_lines.append(
                    f"Direct wins at {wins_direct} of {len(merged)} matching size(s), spectral at {wins_spectral} — "
                    f"consistent with a crossover in this size range, same as kFFTTapThreshold's role in the deckbench dispatch."
                )
            elif wins_direct:
                kernel_lines.append("Direct is faster at every matching size tested here.")
            else:
                kernel_lines.append("Spectral is faster at every matching size tested here.")
    _interpretation(kernel_lines)


# ── 6. Runs ───────────────────────────────────────────────────────────────────


def runs(data: bench_data.DiscoveredData):
    st.markdown('<div class="dnfc-section-title">Runs</div>', unsafe_allow_html=True)

    if data.envelopes.empty:
        _empty_state("build/release/tests/dnf_composer_deckbench.exe")
        return

    summary = (
        data.envelopes.groupby(["source_file", "source_kind", "timestamp", "fingerprint", "git", "git_dirty", "wrapped"])
        .agg(decks=("name", "count"), noisy=("noisy", "sum"))
        .reset_index()
        .sort_values("timestamp", ascending=False)
    )

    dirty_count = int(summary["git_dirty"].fillna(False).sum())
    unwrapped_count = int((~summary["wrapped"]).sum())
    noisy_count = int((summary["noisy"] > 0).sum())

    c1, c2, c3 = st.columns(3)
    with c1:
        (st.error if dirty_count else st.success)(f"{dirty_count} run(s) from a dirty tree", icon="🧹")
    with c2:
        (st.warning if unwrapped_count else st.success)(f"{unwrapped_count} unwrapped run(s)", icon="🧷")
    with c3:
        (st.warning if noisy_count else st.success)(f"{noisy_count} run(s) with a noisy deck", icon="🌫️")

    st.dataframe(
        summary.rename(columns={"source_file": "file", "source_kind": "kind", "git_dirty": "dirty"}),
        width="stretch", hide_index=True,
    )

    fps = summary["fingerprint"].dropna().unique()
    if len(fps) > 1:
        st.info(
            f"{len(fps)} distinct environment fingerprints present. The dashboard never compares runs "
            f"across fingerprints — Trends and Overview filter to one at a time.",
            icon="🖥️",
        )

    run_lines = [f"{len(summary)} run(s) discovered across {len(fps)} fingerprint(s)."]
    if dirty_count:
        run_lines.append(f"{dirty_count} run(s) came from a dirty tree — their numbers may not correspond to any commit.")
    if unwrapped_count:
        run_lines.append(f"{unwrapped_count} run(s) were not taken under scripts/bench — expect a wider noise floor on those.")
    if noisy_count:
        run_lines.append(f"{noisy_count} run(s) include at least one noisy deck.")
    if not (dirty_count or unwrapped_count or noisy_count):
        run_lines.append("Every run is clean, wrapped, and within the noise floor — nothing to flag.")
    _interpretation(run_lines)
