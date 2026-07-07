# Fusion VU Dynamics Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bring `fusion`-style VU dynamics into the current ESP32-S3 2.8" build, add smooth peak return, and render centered `L/R` labels without changing the current palette or broader screen layout.

**Architecture:** Keep the existing branch structure intact. Extend the current `Audio` VU state in `src/audioI2S/AudioEx.h` and `src/audioI2S/Audio.cpp` with live and peak values modeled after the local `fusion-audio` worktree, then consume those values from the current `VuWidget` in `src/displays/widgets/widgets.cpp`. Add one lightweight regression script in `tests/` plus a full compile verification.

**Tech Stack:** Arduino ESP32 (`esp32:esp32:esp32s3`), C++, existing yoRadio display widget system, Python regression scripts in `tests/`, `arduino-cli`.

---

## File map

- Modify: `src/audioI2S/AudioEx.h`
  Purpose: expose the additional VU state and compact accessors needed by the widget without switching to the full `fusion` audio branch.
- Modify: `src/audioI2S/Audio.cpp`
  Purpose: replace the simplified averaged VU logic with the chosen `fusion`-style attack/release/peak-hold behavior adapted to the current branch.
- Modify: `src/displays/widgets/widgets.h`
  Purpose: declare any helper state needed to draw centered `L/R` labels and peak markers cleanly.
- Modify: `src/displays/widgets/widgets.cpp`
  Purpose: draw left/right bars around centered `L/R` labels and render separate live level plus smooth peak markers using the audio-provided VU state.
- Create: `tests/check_fusion_vu_contract.py`
  Purpose: regression-check that the expected VU API and label drawing hooks remain present after refactors.

### Task 1: Lock the new VU contract with a failing regression script

**Files:**
- Create: `tests/check_fusion_vu_contract.py`
- Test: `tests/check_fusion_vu_contract.py`

- [ ] **Step 1: Write the failing test**

```python
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
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```powershell
python tests/check_fusion_vu_contract.py
```

Expected: FAIL because `vuLeftPeak`, `vuRightPeak`, `vuLeftHold`, `vuRightHold`, `get_VUpeak(...)`, and the `L/R` drawing code do not exist yet.

- [ ] **Step 3: Commit the failing test**

```bash
git add tests/check_fusion_vu_contract.py
git commit -m "test: add fusion vu contract regression"
```

### Task 2: Add peak-hold VU state to the current audio interface

**Files:**
- Modify: `src/audioI2S/AudioEx.h`
- Test: `tests/check_fusion_vu_contract.py`

- [ ] **Step 1: Extend the VU declarations in `AudioEx.h`**

Add the new public method declaration near the current VU methods:

```cpp
    void     setVUmeter() {};
    void     getVUlevel() {};
    uint16_t get_VUlevel(uint16_t dimension);
    uint16_t get_VUpeak(uint16_t dimension);
```

Add the new state fields next to the existing `vuLeft` / `vuRight` members:

```cpp
    uint16_t  vuLeft, vuRight;
    uint16_t  vuLeftPeak, vuRightPeak;
    uint8_t   vuLeftHold, vuRightHold;
```

- [ ] **Step 2: Run regression script to verify it still fails for widget-side tokens only**

Run:

```powershell
python tests/check_fusion_vu_contract.py
```

Expected: FAIL mentioning missing widget-side tokens such as `player.get_VUpeak(dimension)` and `L/R` rendering hooks, but no longer missing the new `AudioEx.h` contract tokens.

- [ ] **Step 3: Commit the interface change**

```bash
git add src/audioI2S/AudioEx.h tests/check_fusion_vu_contract.py
git commit -m "feat: add peak vu state contract"
```

### Task 3: Port the `fusion` VU dynamics into the current audio pipeline

**Files:**
- Modify: `src/audioI2S/Audio.cpp`
- Modify: `src/audioI2S/AudioEx.h`
- Test: `tests/check_fusion_vu_contract.py`

- [ ] **Step 1: Replace the simplified VU state update with `fusion`-style live + peak handling**

In `src/audioI2S/Audio.cpp`, initialize the new peak state when playback is not running:

```cpp
    if(!m_f_running) {
      vuLeft = 0; vuRight = 0;
      vuLeftPeak = 0; vuRightPeak = 0;
      vuLeftHold = 0; vuRightHold = 0;
      vTaskDelay(2);
      return;
    }
```

Then update `_computeVUlevel(...)` so the final `f_vu` block computes live level plus peak-hold:

```cpp
  if(f_vu) {
    f_vu = false;

    const uint16_t nextLeft = avg(sampleArray[LEFTCHANNEL][3]);
    const uint16_t nextRight = avg(sampleArray[RIGHTCHANNEL][3]);
    const uint8_t holdFrames = 6;
    const uint8_t peakRelease = 1;

    vuLeft = nextLeft;
    if(vuLeft > config.vuThreshold) config.vuThreshold = vuLeft;
    vuRight = nextRight;
    if(vuRight > config.vuThreshold) config.vuThreshold = vuRight;

    if(vuLeft >= vuLeftPeak) {
      vuLeftPeak = vuLeft;
      vuLeftHold = holdFrames;
    } else if(vuLeftHold > 0) {
      vuLeftHold--;
    } else if(vuLeftPeak > peakRelease) {
      vuLeftPeak -= peakRelease;
    } else {
      vuLeftPeak = vuLeft;
    }

    if(vuRight >= vuRightPeak) {
      vuRightPeak = vuRight;
      vuRightHold = holdFrames;
    } else if(vuRightHold > 0) {
      vuRightHold--;
    } else if(vuRightPeak > peakRelease) {
      vuRightPeak -= peakRelease;
    } else {
      vuRightPeak = vuRight;
    }
  }
```

- [ ] **Step 2: Add peak scaling helper next to `get_VUlevel(...)`**

Add:

```cpp
uint16_t Audio::get_VUpeak(uint16_t dimension){
  if(!config.store.vumeter || config.vuThreshold==0) return 0;
  uint8_t L = map(vuLeftPeak, config.vuThreshold, 0, 0, dimension);
  uint8_t R = map(vuRightPeak, config.vuThreshold, 0, 0, dimension);
  return (L << 8) | R;
}
```

- [ ] **Step 3: Run the regression script to verify only widget work remains**

Run:

```powershell
python tests/check_fusion_vu_contract.py
```

Expected: FAIL only on widget-side tokens such as `player.get_VUpeak(dimension)` and the `L/R` text drawing.

- [ ] **Step 4: Commit the audio dynamics port**

```bash
git add src/audioI2S/Audio.cpp src/audioI2S/AudioEx.h tests/check_fusion_vu_contract.py
git commit -m "feat: port fusion vu peak dynamics"
```

### Task 4: Draw centered `L/R` labels and peak markers in the current VU widget

**Files:**
- Modify: `src/displays/widgets/widgets.h`
- Modify: `src/displays/widgets/widgets.cpp`
- Test: `tests/check_fusion_vu_contract.py`

- [ ] **Step 1: Add minimal widget state for text placement if needed**

If the existing class needs explicit center label positions, add only compact state like:

```cpp
    uint16_t _labelColor;
```

If no member is needed, skip this addition and keep the class unchanged.

- [ ] **Step 2: Update `_draw()` to consume both live and peak levels**

In `src/displays/widgets/widgets.cpp`, keep the current bar rendering but fetch both live and peak values:

```cpp
  uint16_t live = player.get_VUlevel(dimension);
  uint16_t peak = player.get_VUpeak(dimension);

  uint8_t L = (live >> 8) & 0xFF;
  uint8_t R = live & 0xFF;
  uint8_t peakL = (peak >> 8) & 0xFF;
  uint8_t peakR = peak & 0xFF;
```

For the aligned layout branch, keep the current mirrored bars and add 1-pixel peak indicators that drift toward the bars:

```cpp
  _canvas->fillRect(0, 0, measL, _bands.height, _bgcolor);
  _canvas->fillRect(_bands.width * 2 + _bands.space - measR, 0, measR, _bands.height, _bgcolor);

  const uint16_t leftPeakX = min<uint16_t>(_bands.width - 1, peakL);
  const uint16_t rightPeakX = min<uint16_t>(_bands.width - 1, peakR);
  _canvas->drawFastVLine(leftPeakX, 0, _bands.height, _vuframecolor);
  _canvas->drawFastVLine(_bands.width + _bands.space + (_bands.width - 1 - rightPeakX), 0, _bands.height, _vuframecolor);
```

- [ ] **Step 3: Draw centered `L/R` labels after pushing the bar canvas**

After the existing `dsp.writePixels(...)` block for the aligned layout, add text rendering directly on the display:

```cpp
  const uint16_t labelY = _config.top + _bands.height - 1;
  const uint16_t centerX = _config.left + _bands.width;

  dsp.setTextColor(_vuframecolor, _bgcolor);
  dsp.setTextSize(1);
  dsp.setCursor(centerX - 10, labelY - 7);
  dsp.print("L");
  dsp.setCursor(centerX + 4, labelY - 7);
  dsp.print("R");
```

Adjust `centerX - 10` / `centerX + 4` only if the letters visually collide with the bar gap on the real panel.

- [ ] **Step 4: Run the regression script and verify it passes**

Run:

```powershell
python tests/check_fusion_vu_contract.py
```

Expected: PASS with no output.

- [ ] **Step 5: Commit the widget rendering update**

```bash
git add src/displays/widgets/widgets.cpp src/displays/widgets/widgets.h tests/check_fusion_vu_contract.py
git commit -m "feat: render fusion vu peaks and lr labels"
```

### Task 5: Full build and device verification

**Files:**
- Modify: none
- Test: `tests/check_fusion_vu_contract.py`

- [ ] **Step 1: Run the regression script**

```powershell
python tests/check_fusion_vu_contract.py
```

Expected: PASS.

- [ ] **Step 2: Run the full compile**

```powershell
arduino-cli compile -b esp32:esp32:esp32s3:PartitionScheme=huge_app --build-path "F:\tmp\yoradio-fusion-vu" "F:\My\YoRadio\2.8 touch\yoradio\yoRadio"
```

Expected: successful build with no compile errors.

- [ ] **Step 3: Flash the board**

```powershell
arduino-cli upload -b esp32:esp32:esp32s3:PartitionScheme=huge_app -p COM5 --input-dir "F:\tmp\yoradio-fusion-vu" "F:\My\YoRadio\2.8 touch\yoradio\yoRadio"
```

Expected: upload completes successfully to `COM5`.

- [ ] **Step 4: Manual verification on device**

Check on the real board:

- playback starts normally
- left and right bars still use the current theme colors
- `L` and `R` appear centered in the gap
- peaks linger briefly and return smoothly
- UI does not visibly stutter compared with the current stable baseline

- [ ] **Step 5: Commit the verified implementation state**

```bash
git add src/audioI2S/Audio.cpp src/audioI2S/AudioEx.h src/displays/widgets/widgets.cpp src/displays/widgets/widgets.h tests/check_fusion_vu_contract.py
git commit -m "feat: add fusion-style vu dynamics and centered lr labels"
```

## Self-review

- Spec coverage: the plan covers `fusion`-style dynamics, peak-hold, smooth peak return, centered `L/R`, preserved theme colors, compile, and real-device validation.
- Placeholder scan: no `TODO` / `TBD` placeholders remain.
- Type consistency: `get_VUpeak(uint16_t dimension)` is introduced once and then reused consistently; peak state names are consistent across the tasks.
