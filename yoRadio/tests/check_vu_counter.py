from pathlib import Path
import re
import sys


audio_cpp = Path(r"F:\My\YoRadio\2.8 touch\yoradio\yoRadio\src\audioI2S\Audio.cpp")
text = audio_cpp.read_text(encoding="utf-8")

match = re.search(
    r"void Audio::_computeVUlevel\(int16_t sample\[2\]\)\s*\{.*?\n\}",
    text,
    re.S,
)

if not match:
    print("FAIL: _computeVUlevel() not found")
    sys.exit(1)

body = match.group(0)

if "cnt0 == 64" not in body:
    print("FAIL: expected cnt0 stage in _computeVUlevel()")
    sys.exit(1)

lines = [line.strip() for line in body.splitlines() if line.strip()]
last_statement = next((line for line in reversed(lines) if line not in {"}", "{"}), "")

if last_statement != "cnt0++;":
    print(f"FAIL: expected final sampling increment to be cnt0++; got: {last_statement}")
    sys.exit(1)

print("PASS: _computeVUlevel() uses cnt0++ as the final sampling increment")
