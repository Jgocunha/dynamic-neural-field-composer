"""dnf-composer benchmark dashboard.

Read-only. Never launches a perf tool itself -- a benchmark measured next to a
running web server and browser is a worse measurement than one taken alone.
Run the tools yourself (ideally via scripts/bench.ps1 / bench.sh), then open
or refresh this page.

Run via run_dashboard.bat / run_dashboard.sh, or directly:
    streamlit run dashboard.py
"""

from __future__ import annotations

import plotly.io as pio
import streamlit as st

import bench_analysis
import bench_data
import bench_theme
import views

st.set_page_config(
    page_title="dnfc benchmarks",
    page_icon=str(bench_theme.ICON_PATH),
    layout="wide",
    initial_sidebar_state="expanded",
)
st.markdown(bench_theme.base_css(), unsafe_allow_html=True)

pio.templates["dnfc"] = bench_theme.plotly_template()
pio.templates.default = "dnfc"


# `signature` must NOT be underscore-prefixed: st.cache_data excludes underscored
# parameters from the cache key, which would mean the mtime signature never invalidated
# anything and the page served stale artifacts until someone hit Refresh.
@st.cache_data(show_spinner="Reading benchmark artifacts…")
def _load(signature: tuple) -> bench_data.DiscoveredData:
    return bench_data.load_all()


data = _load(bench_data.data_signature())

pages = [
    st.Page(lambda: views.overview(data), title="Overview", icon="🏠", url_path="overview", default=True),
    st.Page(lambda: views.trends(data), title="Trends", icon="📈", url_path="trends"),
    st.Page(lambda: views.decks(data), title="Decks", icon="🧩", url_path="decks"),
    st.Page(lambda: views.elements(data), title="Elements", icon="🔬", url_path="elements"),
    st.Page(lambda: views.kernels(data), title="Kernels", icon="⚙️", url_path="kernels"),
    st.Page(lambda: views.runs(data), title="Runs", icon="🗂️", url_path="runs"),
]

with st.sidebar:
    if bench_theme.LOGO_PATH.is_file():
        st.image(str(bench_theme.LOGO_PATH), width="stretch")
    st.divider()

    # position="sidebar" would work too, but Streamlit always renders that built-in
    # menu at the very top of the sidebar regardless of where st.navigation() is
    # called from -- there's no way to place it below the logo that way. Hiding it
    # and drawing our own st.page_link per page gets the same click-to-switch-page
    # behavior (including active-page highlighting) with the logo on top.
    nav = st.navigation(pages, position="hidden")
    for page in pages:
        st.page_link(page)

    st.divider()
    st.markdown("**Regression threshold**")
    st.session_state["threshold_pct"] = st.slider(
        "Threshold %", min_value=1.0, max_value=20.0,
        value=bench_analysis.DEFAULT_THRESHOLD_PCT, step=0.5,
        help="Mirrors dnf_composer_deckbench --check's --threshold. Default is the measured "
             "noise-floor-derived value (see .claude/(project)notes/perf-noise-floor.md).",
    )

    if not data.envelopes.empty:
        fingerprints = sorted(data.envelopes["fingerprint"].dropna().unique())
        if fingerprints:
            latest = bench_analysis.latest_deckbench_run(data.envelopes)
            default_fp = latest["fingerprint"].iloc[0] if not latest.empty else fingerprints[-1]
            st.session_state["selected_fingerprint"] = st.selectbox(
                "Fingerprint", fingerprints,
                index=fingerprints.index(default_fp) if default_fp in fingerprints else len(fingerprints) - 1,
                help="Runs are only ever compared within one fingerprint — a different machine, "
                     "compiler or flag string gets its own baseline, never averaged in.",
            )

    st.divider()
    st.caption("Read-only — this dashboard never launches a perf tool itself.")
    if st.button("↻ Refresh", width="stretch"):
        st.cache_data.clear()
        st.rerun()

nav.run()
