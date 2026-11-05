# Sound Imager 0.1.0 — Mallula Audio

An original stereo imaging processor inspired by the general workflow of stereo image tools. Not affiliated with Waves; not an emulation of S1 algorithms.

## Controls
- Width: 0–300%; 100% neutral, 0% mono. Mid/side gain preserves the mono sum at neutral rotation, asymmetry and balance.
- Rotation: -45 to +45 degrees; an energy-preserving L/R matrix rotation. This differs from S1's proprietary spatial processing.
- Asymmetry: -12 to +12 dB of relative L/R gain difference, before rotation.
- Balance: attenuates one output channel, centre unity.
- Mono bass: 0 off; 20–500 Hz applies a first-order side-channel high-pass. Below 20 Hz the control fades the filter in. This gently narrows the bass; it is not a brick-wall crossover.
- Output: -24 to +12 dB. Width and asymmetry can raise peaks. There is no limiter; use output trim and the peak display.
- Mono folds the side to zero before rotation; rotation/asymmetry can subsequently position that mono image.
- Swap L/R, bypass, correlation and stereo scope. Double-click a knob to restore its default; Reset restores all controls.

Stereo input/output, zero reported latency, 20 ms parameter smoothing, host automation and session state recall. No artificial stereo generation from mono sources. Peak readings are dBFS, not true peak.

## Install
Windows x64: run SoundImager-0.1.0-Windows-Setup.exe. Installs VST3 in Common Files/VST3, plus standalone app and uninstaller.
macOS 11+: run SoundImager-0.1.0-macOS.pkg. Universal Intel/Apple Silicon VST3 and AU are installed under /Library/Audio/Plug-Ins; standalone under /Applications/Mallula Audio.
In Logic use Audio Units > Mallula Audio > Sound Imager (stereo). Rescan in Plug-in Manager if needed. In Reaper rescan VST3.
Installers are unsigned; macOS package is not Apple notarized. Plugin bundles have ad-hoc signatures. macOS security approval may be required in System Settings > Privacy & Security after opening the package.
Updates use stable plugin IDs and installer identities, so future versions can replace the existing version.

## Build
Uses the same native GitHub Actions / JUCE / CMake / Inno Setup / pkgbuild method as Iron Squeeze. JUCE is pinned to commit 29396c22c93392d6738e021b83196283d6e4d850. Windows: build_windows.bat; Mac: bash build_macos.sh. Both scripts test before packaging. CI installs the Mac package and runs auval.
Portable tests: g++ -std=c++17 -O2 Tests/CoreTests.cpp -o core-tests && ./core-tests

## Licence
Original source: MIT; packaging adapted from Iron Squeeze (MIT). JUCE and bundled SDKs retain their own licences. Distribution of JUCE-based binaries must comply with JUCE's applicable commercial or AGPL terms; the MIT licence covers only this project's original source.
