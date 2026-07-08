import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read_source(*parts: str) -> str:
    return (ROOT.joinpath(*parts)).read_text(encoding="utf-8")


def strip_comments(source: str) -> str:
    source = re.sub(r"/\*.*?\*/", "", source, flags=re.DOTALL)
    source = re.sub(r"//.*?$", "", source, flags=re.MULTILINE)
    return source


def extract_function_body(source: str, signature_pattern: str, label: str) -> str:
    match = re.search(signature_pattern, source, flags=re.MULTILINE)
    if match is None:
        raise SystemExit(f"Unable to locate {label} in widgets.cpp")

    open_brace = source.find("{", match.end() - 1)
    if open_brace == -1:
        raise SystemExit(f"Unable to locate opening brace for {label} in widgets.cpp")

    depth = 0
    for index in range(open_brace, len(source)):
        char = source[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[open_brace + 1:index]

    raise SystemExit(f"Unable to locate closing brace for {label} in widgets.cpp")


def missing_patterns(source: str, patterns: list[tuple[str, str]]) -> list[str]:
    return [
        label
        for label, pattern in patterns
        if re.search(pattern, source, flags=re.MULTILINE | re.DOTALL) is None
    ]


audio_h = strip_comments(read_source("src", "audioI2S", "AudioEx.h"))
widgets_cpp = strip_comments(read_source("src", "displays", "widgets", "widgets.cpp"))
vu_draw_body = extract_function_body(
    widgets_cpp,
    r"void\s+VuWidget::_draw\s*\(\s*\)\s*\{",
    "VuWidget::_draw()",
)

required_audio_patterns = [
    ("uint16_t vuLeft", r"uint16_t\b[\s\w,*]*\bvuLeft\b"),
    ("uint16_t vuRight", r"uint16_t\b[\s\w,*]*\bvuRight\b"),
    ("uint16_t vuLeftPeak", r"uint16_t\b[\s\w,*]*\bvuLeftPeak\b"),
    ("uint16_t vuRightPeak", r"uint16_t\b[\s\w,*]*\bvuRightPeak\b"),
    ("uint8_t vuLeftHold", r"uint8_t\b[\s\w,*]*\bvuLeftHold\b"),
    ("uint8_t vuRightHold", r"uint8_t\b[\s\w,*]*\bvuRightHold\b"),
    ("uint16_t get_VUlevel(uint16_t)", r"uint16_t\s+get_VUlevel\s*\(\s*uint16_t(?:\s+\w+)?\s*\)\s*;"),
    ("uint16_t get_VUpeak(uint16_t)", r"uint16_t\s+get_VUpeak\s*\(\s*uint16_t(?:\s+\w+)?\s*\)\s*;"),
]

required_widget_patterns = [
    ("player.get_VUlevel(dimension)", r"player\.get_VUlevel\s*\(\s*dimension\s*\)"),
    ("player.get_VUpeak(dimension)", r"player\.get_VUpeak\s*\(\s*dimension\s*\)"),
    ('setCursor(...) then print("L") nearby', r'dsp\.setCursor\s*\([^;]*\)\s*;[\s\S]{0,400}?dsp\.print\s*\(\s*"L"\s*\)'),
    ('setCursor(...) then print("R") nearby', r'dsp\.setCursor\s*\([^;]*\)\s*;[\s\S]{0,400}?dsp\.print\s*\(\s*"R"\s*\)'),
]

missing_audio = missing_patterns(audio_h, required_audio_patterns)
missing_widget = missing_patterns(vu_draw_body, required_widget_patterns)

if missing_audio or missing_widget:
    raise SystemExit(
        "Missing audio contract: "
        + ", ".join(missing_audio)
        + " | Missing widget contract: "
        + ", ".join(missing_widget)
    )
