import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read_source(*parts: str) -> str:
    return (ROOT.joinpath(*parts)).read_text(encoding="utf-8")


def strip_comments(source: str) -> str:
    source = re.sub(r"/\*.*?\*/", "", source, flags=re.DOTALL)
    source = re.sub(r"//.*?$", "", source, flags=re.MULTILINE)
    return source


def extract_vu_widget_region(source: str) -> str:
    start_match = re.search(r"void\s+VuWidget::init\s*\(", source)
    if start_match is None:
        raise SystemExit("Unable to isolate VuWidget implementation region in widgets.cpp")
    region_source = source[start_match.start():]
    end_match = re.search(r"^\s*#else\b.*$", region_source, flags=re.MULTILINE)
    if end_match is None:
        raise SystemExit("Unable to isolate VuWidget implementation region in widgets.cpp")
    return region_source[:end_match.start()]


def missing_patterns(source: str, patterns: list[tuple[str, str]]) -> list[str]:
    return [
        label
        for label, pattern in patterns
        if re.search(pattern, source, flags=re.MULTILINE | re.DOTALL) is None
    ]


audio_h = strip_comments(read_source("src", "audioI2S", "AudioEx.h"))
widgets_cpp = strip_comments(read_source("src", "displays", "widgets", "widgets.cpp"))
vu_widget_region = extract_vu_widget_region(widgets_cpp)

required_audio_patterns = [
    ("uint16_t vuLeft, vuRight;", r"uint16_t\s+vuLeft\s*,\s*vuRight\s*;"),
    ("uint16_t vuLeftPeak, vuRightPeak;", r"uint16_t\s+vuLeftPeak\s*,\s*vuRightPeak\s*;"),
    ("uint8_t vuLeftHold, vuRightHold;", r"uint8_t\s+vuLeftHold\s*,\s*vuRightHold\s*;"),
    ("uint16_t get_VUlevel(uint16_t dimension);", r"uint16_t\s+get_VUlevel\s*\(\s*uint16_t\s+dimension\s*\)\s*;"),
    ("uint16_t get_VUpeak(uint16_t dimension);", r"uint16_t\s+get_VUpeak\s*\(\s*uint16_t\s+dimension\s*\)\s*;"),
]

required_widget_patterns = [
    ("player.get_VUlevel(dimension)", r"player\.get_VUlevel\s*\(\s*dimension\s*\)"),
    ("player.get_VUpeak(dimension)", r"player\.get_VUpeak\s*\(\s*dimension\s*\)"),
    ("dsp.setCursor(", r"dsp\.setCursor\s*\("),
    ('"L"', r'"L"'),
    ('"R"', r'"R"'),
]

missing_audio = missing_patterns(audio_h, required_audio_patterns)
missing_widget = missing_patterns(vu_widget_region, required_widget_patterns)

if missing_audio or missing_widget:
    raise SystemExit(
        "Missing audio contract: "
        + ", ".join(missing_audio)
        + " | Missing widget contract: "
        + ", ".join(missing_widget)
    )
