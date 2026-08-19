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


audio_h = strip_comments(read_source("src", "audioI2S", "Audio.h"))
audio_cpp = strip_comments(read_source("src", "audioI2S", "Audio.cpp"))
widgets_cpp = strip_comments(read_source("src", "displays", "widgets", "widgets.cpp"))
display_cpp = strip_comments(read_source("src", "core", "display.cpp"))
player_cpp = strip_comments(read_source("src", "core", "player.cpp"))
vu_draw_body = extract_function_body(
    widgets_cpp,
    r"void\s+VuWidget::_draw\s*\(\s*\)\s*\{",
    "VuWidget::_draw()",
)

required_audio_patterns = [
    ("uint16_t get_VUlevel(uint16_t)", r"uint16_t\s+get_VUlevel\s*\(\s*uint16_t(?:\s+\w+)?\s*\)\s*;"),
    ("uint16_t get_VUpeak(uint16_t)", r"uint16_t\s+get_VUpeak\s*\(\s*uint16_t(?:\s+\w+)?\s*\)\s*;"),
    ("queued live VU source", r"get_VUlevel[\s\S]*?m_vu_display_frame\.left"),
    ("display peak VU source", r"get_VUpeak[\s\S]*?m_vu_display_peak_left"),
]

required_widget_patterns = [
    ("player.get_VUlevel(...)", r"player\.get_VUlevel\s*\(\s*[^)]+\s*\)"),
    ("player.get_VUpeak(...)", r"player\.get_VUpeak\s*\(\s*[^)]+\s*\)"),
    ('setCursor(...) then print("L") nearby', r'dsp\.setCursor\s*\([^;]*\)\s*;[\s\S]{0,200}?dsp\.print\s*\(\s*"L"\s*\)'),
    ('setCursor(...) then print("R") nearby', r'dsp\.setCursor\s*\([^;]*\)\s*;[\s\S]{0,200}?dsp\.print\s*\(\s*"R"\s*\)'),
    ("peak clamped to rendered bar edge", r"renderedBarEnd\s*\(\s*active[LR]\s*\)"),
]

missing_audio = missing_patterns(audio_h + "\n" + audio_cpp, required_audio_patterns)
missing_widget = missing_patterns(vu_draw_body, required_widget_patterns)

legacy_widget_dynamics = [
    label
    for label, pattern in [
        ("local VU smoothing state", r"static\s+uint16_t\s+meas[LR]"),
        ("legacy fade-speed dynamics", r"_bands\.fadespeed"),
        ("legacy 9/8 VU gain", r"raw[LR]\s*\*\s*9"),
    ]
    if re.search(pattern, vu_draw_body)
]

task_core_errors = []
if re.search(r"setAudioTaskCore\s*\(\s*0\s*\)", player_cpp) is None:
    task_core_errors.append("audio task is not pinned to core 0")
if re.search(r"#define\s+DSP_TASK_CORE_ID\s+1", display_cpp) is None:
    task_core_errors.append("display task is not pinned to core 1")
pager_init = display_cpp.find("_pager = new Pager()")
display_task_start = display_cpp.find("_createDspTask()", pager_init)
if pager_init == -1 or display_task_start == -1 or pager_init > display_task_start:
    task_core_errors.append("display task starts before Pager initialization")

if missing_audio or missing_widget or legacy_widget_dynamics or task_core_errors:
    raise SystemExit(
        "Missing audio contract: "
        + ", ".join(missing_audio)
        + " | Missing widget contract: "
        + ", ".join(missing_widget)
        + " | Legacy widget dynamics: "
        + ", ".join(legacy_widget_dynamics)
        + " | Task core errors: "
        + ", ".join(task_core_errors)
    )
