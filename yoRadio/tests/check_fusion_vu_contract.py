from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
audio_h = (ROOT / "src" / "audioI2S" / "AudioEx.h").read_text(encoding="utf-8")
widgets_cpp = (ROOT / "src" / "displays" / "widgets" / "widgets.cpp").read_text(encoding="utf-8")

required_audio_tokens = [
    "uint16_t  vuLeft, vuRight;",
    "uint16_t  vuLeftPeak, vuRightPeak;",
    "uint8_t   vuLeftHold, vuRightHold;",
    "uint16_t get_VUlevel(uint16_t dimension);",
    "uint16_t get_VUpeak(uint16_t dimension);",
]

required_widget_tokens = [
    "player.get_VUlevel(dimension)",
    "player.get_VUpeak(dimension)",
    "dsp.setCursor(",
    "\"L\"",
    "\"R\"",
]

missing_audio = [token for token in required_audio_tokens if token not in audio_h]
missing_widget = [token for token in required_widget_tokens if token not in widgets_cpp]

if missing_audio or missing_widget:
    raise SystemExit(
        "Missing audio tokens: "
        + ", ".join(missing_audio)
        + " | Missing widget tokens: "
        + ", ".join(missing_widget)
    )
