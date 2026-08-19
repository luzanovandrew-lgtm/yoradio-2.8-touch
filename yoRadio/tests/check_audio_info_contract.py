#!/usr/bin/env python3
"""Ensure codec and bitrate do not depend only on legacy text callbacks."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
player = (ROOT / "src/core/player.cpp").read_text(encoding="utf-8")
handlers = (ROOT / "src/core/audiohandlers.h").read_text(encoding="utf-8")
display = (ROOT / "src/core/display.cpp").read_text(encoding="utf-8")

required = (
    "Audio::getCodecname()",
    "Audio::getBitRate()",
    "config.setBitrateFormat(format)",
    "config.station.bitrate = bitrateKbps",
    "display.putRequest(DBITRATE)",
    "netserver.requestOnChange(BITRATE, 0)",
)

missing = [item for item in required if item not in player]
if missing:
    raise SystemExit("Missing direct audio info path: " + ", ".join(missing))

if "if(end == info || bitrateBps <= 0) return;" not in handlers:
    raise SystemExit("Invalid callback bitrate can still erase the displayed value")
dbitrate = display[display.index("case DBITRATE:"):display.index("case AUDIOINFO:")]
if "_drawBitrate();" not in dbitrate or "_drawWeatherIcon();" in dbitrate:
    raise SystemExit("DBITRATE must redraw only the bitrate block")
if "now - _bitrateUpdateTicks >= 5000" not in player:
    raise SystemExit("VBR display updates are not throttled")

print("Direct codec/bitrate display path OK")
