# XYGrains

This is a repository for an ongoing granular synthesizer + sequencer project. The intended use case is as a VST3 plugin for my own performances.

I have been consistently frustrated by unintuitive sequencers like those of Elektron whose slow workflow and overemphasis on "on-grid," 4/4-pattern musical logic necessarily leads to constraints on creativity. This sampler runs in the other direction. It can operate at any quantization, and can easily be used as a granular synthesizer, or 

## Features

XYGrains operates on a single sample to reduce complexity. The core engine spawns grains. Each grain reads from the main sample at different positions, and applies gain control, re-pitching, and an ADSR envelope. Currently, XYGrains uses a fixed bank of 32 voices. This may change in the future.

### Currently Implemented

- An ``XY-pad,'' where the X-axis represents time and the Y-axis is one of the following parameters:
    - Sample position
    - Gain
    - Attack (ADSR)
    - Decay (ADSR)
    - Sustain (ADSR)
    - Release (ADSR)
    - Note length
    - Playback speed (pitch)
- Variable BPM / beat count
- Time quantization
    - Note that, rather than quantizing to standard Western music theory-based time divisions (1/4 = quarter note, etc.) we just quantize to subdivisions of beats. 1/1 is one per beat, 1/2 2 per beat, and we go 1/n to n=32. This allows for easy implementation of polyrhythms.
    - There is no "sticky quantization"; notes are moved freely and we hit `q` to quantize selected notes.
- Patch saving via JSON

### To Implement

- 10 banks of patterns per patch; switch between banks with number keys
- Popup visual aids: wavefunction of sample when adjusting position, ADSR envelope when adjusting attack, etc.
- BPM estimator, cf. [Tzanetakis, et al.](https://soundlab.cs.princeton.edu/publications/2001_amta_aadwt.pdf)
    - Also: option for locked BPM -> sample playback speed relation (by multiplier, default 1.0)
- Stochastic behavior
    - Support for randomized distributions of trig times, with multiple distribution types supported:
        - Flat distributions
        - Gaussians
        - Any arbitrary 2D wavepacket function
- Pitch modulation
