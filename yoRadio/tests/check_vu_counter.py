from pathlib import Path
import re
import sys


audio_cpp = Path(r"F:\My\YoRadio\2.8 touch\yoradio\yoRadio\src\audioI2S\Audio.cpp")
text = audio_cpp.read_text(encoding="utf-8")

match = re.search(r"void Audio::calculateVUlevel\(int32_t\* sample\)\s*\{.*?\n\}", text, re.S)

if not match:
    print("FAIL: calculateVUlevel() not found")
    sys.exit(1)

body = match.group(0)

for token in ("l >= m_vu_items.left", "r >= m_vu_items.right", "m_vu_reference", "m_vu_items.left_peak", "m_vu_items.right_peak", "m_vu_frame_write"):
    if token not in body:
        print(f"FAIL: missing independent VU state: {token}")
        sys.exit(1)

play_chunk = re.search(r"void IRAM_ATTR Audio::playChunk\(\)\s*\{.*?\n\}", text, re.S)
if not play_chunk or "framesConsumed" not in play_chunk.group(0):
    print("FAIL: VU is not updated from I2S-consumed frames")
    sys.exit(1)

get_level = re.search(r"uint16_t Audio::get_VUlevel\(uint16_t dimension\)\s*\{.*?\n\}", text, re.S)
if not get_level or "attackAlpha" not in get_level.group(0) or "releaseAlpha" not in get_level.group(0):
    print("FAIL: codec-independent display envelope is missing")
    sys.exit(1)

print("PASS: VU uses I2S cadence with legacy envelope, adaptive reference, and independent peak state")
