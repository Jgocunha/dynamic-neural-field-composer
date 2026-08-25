"""Palette, Plotly template and CSS for the benchmark dashboard.

Colors are derived from resources/style_light_green_accent.json (the app's own
ImGui theme, teal #1F8068) and resources/images/logo.png (the wave colors used
for per-deck series so a deck's color means the same thing across every chart
in the dashboard).
"""

from __future__ import annotations

import base64
from functools import lru_cache
from pathlib import Path

RESOURCES_DIR = Path(__file__).resolve().parent.parent
LOGO_PATH = RESOURCES_DIR / "images" / "logo.png"
ICON_PATH = RESOURCES_DIR / "icons" / "icon.png"

# ── Palette ──────────────────────────────────────────────────────────────────

ACCENT = "#1F8068"
ACCENT_DARK = "#166350"
BG = "#F5F5F5"
CARD_BG = "#FFFFFF"
BORDER = "#E3E6E5"
TEXT = "#1A1A1A"
TEXT_MUTED = "#6B7573"

OK_COLOR = "#1F8068"
REGRESSED_COLOR = "#D64550"
NOISY_COLOR = "#E8A23D"
UNKNOWN_COLOR = "#8B9290"

# Deck / series colors, sampled from the wave lines in resources/images/logo.png.
DECK_COLORS = {
    "small": "#2A7FBF",     # blue
    "medium": "#1F8068",    # teal (accent)
    "large-a": "#F5A623",   # orange
    "large-b": "#F638C8",   # magenta
}
DECK_COLOR_FALLBACKS = ["#35C8E8", "#2A7FBF", "#F5A623", "#F638C8", "#1F8068", "#8B9290"]


def color_for_deck(name: str, index: int = 0) -> str:
    if name in DECK_COLORS:
        return DECK_COLORS[name]
    return DECK_COLOR_FALLBACKS[index % len(DECK_COLOR_FALLBACKS)]


def hex_to_rgba(hex_color: str, alpha: float) -> str:
    hex_color = hex_color.lstrip("#")
    r, g, b = (int(hex_color[i : i + 2], 16) for i in (0, 2, 4))
    return f"rgba({r},{g},{b},{alpha})"


def verdict_color(verdict: str) -> str:
    return {
        "OK": OK_COLOR,
        "REGRESSED": REGRESSED_COLOR,
        "NOISY": NOISY_COLOR,
    }.get(verdict, UNKNOWN_COLOR)


# ── Font embedding ───────────────────────────────────────────────────────────

_FONT_FILES = {
    "Cera Pro": "Cera Pro Medium.ttf",
    "Cera Pro Bold": "Cera Pro Bold.ttf",
    "JetBrains Mono": "JetBrainsMono-Regular.ttf",
    "JetBrains Mono Medium": "JetBrainsMono-Medium.ttf",
}


@lru_cache(maxsize=1)
def font_face_css() -> str:
    faces = []
    for family, filename in _FONT_FILES.items():
        path = RESOURCES_DIR / "fonts" / filename
        if not path.exists():
            continue
        data = base64.b64encode(path.read_bytes()).decode("ascii")
        faces.append(
            f"""
            @font-face {{
                font-family: '{family}';
                src: url(data:font/ttf;base64,{data}) format('truetype');
                font-display: swap;
            }}"""
        )
    return "\n".join(faces)


@lru_cache(maxsize=1)
def logo_data_uri() -> str:
    path = RESOURCES_DIR / "images" / "logo.png"
    if not path.exists():
        return ""
    data = base64.b64encode(path.read_bytes()).decode("ascii")
    return f"data:image/png;base64,{data}"


def base_css() -> str:
    hero_wash = ", ".join(DECK_COLORS.values())
    return f"""
    <style>
    {font_face_css()}

    html, body, [class*="css"] {{
        font-family: 'Cera Pro', -apple-system, sans-serif;
    }}
    .stApp {{
        background-color: {BG};
    }}
    section[data-testid="stSidebar"] {{
        background-color: {CARD_BG};
        border-right: 1px solid {BORDER};
    }}
    .dnfc-mono {{
        font-family: 'JetBrains Mono', 'Consolas', monospace;
    }}
    .dnfc-card {{
        background: {CARD_BG};
        border: 1px solid {BORDER};
        border-radius: 8px;
        padding: 1rem 1.2rem;
        margin-bottom: 0.75rem;
    }}
    .dnfc-hero {{
        background: linear-gradient(120deg, {hero_wash});
        background-size: 400% 400%;
        opacity: 0.10;
        position: absolute;
        inset: 0;
        border-radius: 8px;
        z-index: 0;
    }}
    .dnfc-hero-content {{
        position: relative;
        z-index: 1;
        display: flex;
        align-items: center;
        gap: 1.2rem;
        padding: 1.4rem 1.6rem;
    }}
    .dnfc-pill {{
        display: inline-block;
        font-family: 'JetBrains Mono Medium', monospace;
        font-size: 0.72rem;
        font-weight: 600;
        letter-spacing: 0.03em;
        padding: 0.18rem 0.6rem;
        border-radius: 999px;
        color: white;
    }}
    .dnfc-chip {{
        display: inline-block;
        font-family: 'JetBrains Mono', monospace;
        font-size: 0.78rem;
        color: {TEXT_MUTED};
        background: {BG};
        border: 1px solid {BORDER};
        border-radius: 6px;
        padding: 0.15rem 0.55rem;
        margin: 0.1rem 0.25rem 0.1rem 0;
    }}
    .dnfc-metric-value {{
        font-family: 'JetBrains Mono Medium', monospace;
        font-size: 1.6rem;
        font-weight: 600;
        color: {TEXT};
    }}
    .dnfc-metric-label {{
        font-size: 0.78rem;
        color: {TEXT_MUTED};
        text-transform: uppercase;
        letter-spacing: 0.04em;
    }}
    .dnfc-section-title {{
        font-weight: 600;
        font-size: 1.05rem;
        margin: 0.4rem 0 0.6rem 0;
        color: {TEXT};
    }}
    </style>
    """


def plotly_template() -> dict:
    return {
        "layout": {
            "paper_bgcolor": CARD_BG,
            "plot_bgcolor": CARD_BG,
            "font": {"family": "JetBrains Mono, monospace", "color": TEXT, "size": 12},
            "colorway": list(DECK_COLORS.values()) + DECK_COLOR_FALLBACKS,
            "xaxis": {"gridcolor": BORDER, "zerolinecolor": BORDER, "linecolor": BORDER},
            "yaxis": {"gridcolor": BORDER, "zerolinecolor": BORDER, "linecolor": BORDER},
            "legend": {"bgcolor": "rgba(0,0,0,0)"},
            "margin": {"t": 40, "b": 40, "l": 50, "r": 20},
        }
    }
