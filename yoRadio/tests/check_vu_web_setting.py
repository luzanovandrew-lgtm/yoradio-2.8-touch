#!/usr/bin/env python3
"""Static contract for the persistent web VU smoothness control."""

import gzip
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def require(text: str, needle: str, source: str) -> None:
    if needle not in text:
        raise AssertionError(f"{source}: missing {needle!r}")


config_h = (ROOT / "src/core/config.h").read_text(encoding="utf-8")
config_cpp = (ROOT / "src/core/config.cpp").read_text(encoding="utf-8")
player_cpp = (ROOT / "src/core/player.cpp").read_text(encoding="utf-8")
commands = (ROOT / "src/core/commandhandler.cpp").read_text(encoding="utf-8")
netserver = (ROOT / "src/core/netserver.cpp").read_text(encoding="utf-8")
audio_h = (ROOT / "src/audioI2S/Audio.h").read_text(encoding="utf-8")

with gzip.open(ROOT / "data/www/options.html.gz", "rt", encoding="utf-8") as asset:
    options = asset.read()

require(config_h, "#define CONFIG_VERSION  8", "config.h")
require(config_h, "uint16_t  vuGain;", "config.h")
require(config_cpp, "saveValue(&store.vuPeakReleaseMs, (uint16_t)1800);", "config.cpp")
require(player_cpp, "applyVUSettings();", "player.cpp")
require(audio_h, "setVUSettings", "Audio.h")

controls = {
    "vug": "vugain",
    "vuw": "vuwindow",
    "vua": "vuattack",
    "vur": "vurelease",
    "vuph": "vupeakhold",
    "vupr": "vupeakrel",
}
for element_id, command in controls.items():
    require(commands, f'strEquals(command, "{command}")', "commandhandler.cpp")
    require(netserver, f'\\"{element_id}\\":%u', "netserver.cpp")
    require(options, f'id="{element_id}"', "options.html.gz")
    require(options, f'data-command="{command}"', "options.html.gz")

print("VU web setting contract OK")
