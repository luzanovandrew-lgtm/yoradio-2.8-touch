import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
audio_h = (ROOT / "src" / "audioI2S" / "AudioEx.h").read_text(encoding="utf-8")
widgets_cpp = (ROOT / "src" / "displays" / "widgets" / "widgets.cpp").read_text(encoding="utf-8")


def find_vu_draw_section(source: str) -> str:
    start = source.find("void VuWidget::_draw(){")
    end = source.find("\nvoid VuWidget::loop(){", start)
    if start == -1 or end == -1:
        raise SystemExit("Unable to locate VuWidget::_draw() contract section in widgets.cpp")
    return source[start:end]


def missing_patterns(source: str, patterns: list[tuple[str, str]]) -> list[str]:
    return [label for label, pattern in patterns if re.search(pattern, source, re.MULTILINE | re.DOTALL) is None]


vu_draw = find_vu_draw_section(widgets_cpp)

required_audio_patterns = [
    ("uint16_t vuLeft, vuRight;", r"uint16_t\s+vuLeft\s*,\s*vuRight\s*;"),
    ("uint16_t vuLeftPeak, vuRightPeak;", r"uint16_t\s+vuLeftPeak\s*,\s*vuRightPeak\s*;"),
    ("uint8_t vuLeftHold, vuRightHold;", r"uint8_t\s+vuLeftHold\s*,\s*vuRightHold\s*;"),
    ("uint16_t get_VUlevel(uint16_t dimension);", r"uint16_t\s+get_VUlevel\s*\(\s*uint16_t\s+dimension\s*\)\s*;"),
    ("uint16_t get_VUpeak(uint16_t dimension);", r"uint16_t\s+get_VUpeak\s*\(\s*uint16_t\s+dimension\s*\)\s*;"),
]

required_widget_patterns = [
    ("VuWidget::_draw uses player.get_VUlevel(dimension)", r"player\.get_VUlevel\s*\(\s*dimension\s*\)"),
    ("VuWidget::_draw uses player.get_VUpeak(dimension)", r"player\.get_VUpeak\s*\(\s*dimension\s*\)"),
    ('VuWidget::_draw renders "L" with dsp.setCursor(...) nearby', r'dsp\.setCursor\s*\([^;]*\)\s*;\s*dsp\.print\s*\(\s*"L"\s*\)'),
    ('VuWidget::_draw renders "R" with dsp.setCursor(...) nearby', r'dsp\.setCursor\s*\([^;]*\)\s*;\s*dsp\.print\s*\(\s*"R"\s*\)'),
]

missing_audio = missing_patterns(audio_h, required_audio_patterns)
missing_widget = missing_patterns(vu_draw, required_widget_patterns)

if missing_audio or missing_widget:
    raise SystemExit(
        "Missing audio contract: "
        + ", ".join(missing_audio)
        + " | Missing VuWidget::_draw contract: "
        + ", ".join(missing_widget)
    )
