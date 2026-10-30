# Iron Squeeze 0.2.1

Distressor inspired compressor by Mallula Audio, implemented as an original
feedback DSP model. This is not an Empirical Labs product or a verified
component-by-component emulation. See docs/MODEL_AND_SOURCES.md for every estimate.

## Deliverable status

Native Windows and macOS installers were built successfully from commit
`f8ec4efc0f756be96a1a7ccac02f5eb24a04e8c1`. Download the platform artifacts from:
https://github.com/SeveriMallula/IronSqueeze/actions/runs/41026240511

Windows includes VST3 and standalone. The macOS package includes VST3, AU and
standalone for Intel and Apple Silicon, targeting macOS 11+. The macOS installer
was installed on the runner and Apple AU validation passed. Both native DSP and
processor test suites passed. See docs/TEST_REPORT.md for limits.

Installers are unsigned, and the macOS package is not notarized.
JUCE 8.0.12 is fetched at its pinned commit during configuration.

The production plugin format is VST3 (.vst3). Legacy VST2 (.vst) is not included.
The macOS target also includes Audio Unit (.component), for Logic Pro.

## Audio engine

- Eight compression curves: 1:1, 2:1, 3:1, 4:1, 6:1, optical-style 10:1,
  20:1, and 40:1 Crush.
- Feedback detector tapped after harmonic coloration and before the audio high pass.
- Ratio-specific thresholds and knees; input gain drives compression.
- Attack 0.05–30 ms, release 50–3500 ms. Optical memory lengthens recovery.
- Clean, asymmetric H2, symmetric H3; DC removal in the colour modes.
- Detector 90 Hz high pass and 6 kHz emphasis, plus separate audible 80 Hz HP.
- Linked stereo or independent dual mono, mono host support.
- 4x linear-phase FIR oversampling; integer latency reported to the host.
- Smoothed gain, filter engagement, colour blend, parallel mix and bypass.
- Five starting points, host automation, saved-state recall.

## Interface

Ivory rotary controls, LED gain-reduction meter and input/output peak levels
in a compact 1060x440 window. The DSP architecture diagram, transfer plot and
history panel have been removed from the GUI in 0.2.1.

## Use

Start with Neutral Start and drive Input until the meter shows 3–6 dB reduction.
Match the processed and bypassed loudness with Output. Slower attack preserves
transients; faster release brings forward sustain and room sound. SC LOW CUT
reduces bass sensitivity. AUDIO 80 Hz removes audible bass. Mix blends the
latency-aligned unprocessed input with the wet output.

Crush and British are compressor curves, not true-peak safety limiters. Output
can exceed 0 dBFS. No lookahead or external sidechain input is implemented.

## Build the Windows installer

Use Windows 10/11 x64, Visual Studio 2022 (Desktop development with C++),
CMake 3.22+ and Inno Setup 6. Run build_windows.bat from a developer terminal.
It builds VST3 and standalone, runs both test executables, then creates:

    dist/IronSqueeze-0.2.1-Windows-Setup.exe

The installer puts the VST3 in Common Files/VST3 and the standalone in
Program Files/Mallula Audio/Iron Squeeze. Its stable application identity
supports upgrades over installers produced by this packaging configuration.
Close audio hosts before updating. Windows compilation, tests and installer creation passed on the native runner.
JUCE 8.0.12 explicitly rejects MinGW; use the supported MSVC toolchain.

## Build the macOS installer

Use macOS with Xcode command-line tools, Xcode, and CMake 3.22+:

    ./build_macos.sh

It targets macOS 11+ with both Apple Silicon and Intel architectures, builds
VST3/AU/standalone, runs tests, and creates:

    dist/IronSqueeze-0.2.1-macOS.pkg

Installation paths: /Library/Audio/Plug-Ins/VST3,
/Library/Audio/Plug-Ins/Components, /Applications/Mallula Audio.
APP_SIGN_IDENTITY and INSTALLER_SIGN_IDENTITY select signing certificates.
NOTARY_PROFILE selects an existing notarytool keychain profile. Without these,
the plugins receive ad hoc signatures and the package remains unsigned and
unnotarized. Native macOS installation and AU validation passed; signing/notarization has not been performed.

Native builds run in this repository on the build/installers branch. Windows
VST3/standalone compilation, tests and installer creation have succeeded. The
macOS package passed installation and Apple auval on the runner. See the Actions page for current results. Generated installers
are unsigned; the macOS package is not notarized.

## Portable DSP tests

    cmake -S . -B core-build -DIRON_CORE_ONLY=ON
    cmake --build core-build
    ctest --test-dir core-build --output-on-failure

## Model accuracy and licensing

The topology is guided by the manufacturer's public manual. Thresholds, knees,
nonlinear equations, filter widths, and recovery laws are our estimates.
Hardware measurements and schematics are needed for a calibrated circuit model.
No measured match to a physical Distressor is claimed.

Original code is MIT licensed. Bundled JUCE is separately dual licensed under
AGPLv3 or its commercial licence; see third_party/JUCE/LICENSE.md (or
../third_party/JUCE/LICENSE.md in the complete archive). Keep those notices
and comply with the selected framework licence when distributing builds.
