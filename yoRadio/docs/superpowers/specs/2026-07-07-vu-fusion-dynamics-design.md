# Fusion VU Dynamics Design

## Goal

Adopt the more expressive VU meter dynamics from the local `fusion-audio` worktree while keeping the current ESP32-S3 2.8" layout and theme stable.

## Chosen approach

Use a safe hybrid approach:

- move VU dynamics behavior from `fusion` into the current audio/VU pipeline
- keep the current display theme colors from `mytheme.h`
- keep the current overall screen layout
- add compact `L` and `R` labels centered between the two channel bars
- keep the bars visually placed to the left and right of those labels

## What will change

### Audio/VU behavior

The current branch uses a simpler averaged VU calculation in `src/audioI2S/Audio.cpp`.

The target behavior is based on the `fusion-audio` branch:

- fast attack when level rises
- gradual release when level falls
- peak-hold behavior per channel
- peak markers returning smoothly toward the active bars

Implementation intent:

- extend the current VU state with separate live level and peak level per channel
- keep the current branch API shape as stable as possible
- avoid dragging unrelated `fusion` audio features into this branch

### Display behavior

The current `VuWidget` will continue to own drawing.

We will:

- preserve the current palette
- add centered `L` and `R` labels between the channel bars
- use audio-provided live/peak values for drawing
- keep the present geometry unless a tiny spacing adjustment is needed for the labels

## Boundaries

This change explicitly does not include:

- switching to the full `fusion` audio library branch
- changing the weather/theme layout
- changing station/title layout
- introducing a new display mode

## Risks

- The VU path runs frequently, so extra logic must stay lightweight.
- Peak-hold state must not introduce visible lag in unrelated UI.
- Mapping from current audio scale to display scale may need tuning after first flash.

## Verification plan

- compile for `esp32:esp32:esp32s3` with `PartitionScheme=huge_app`
- flash to the existing board
- verify:
  - playback remains stable
  - VU reacts faster on attack
  - peaks fall smoothly
  - `L/R` labels are readable and centered
  - no obvious UI slowdown appears during playback
