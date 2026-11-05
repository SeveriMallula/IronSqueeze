# Sound Imager 0.1.0 validation — 2026-11-05

## macOS native build: PASS
Build commit: 5833b8dd586c2bef278eda18658f4d5a2e89c8b4
Run: https://github.com/SeveriMallula/IronSqueeze/actions/runs/42027598558

- Xcode Release universal AU, VST3 and standalone compilation passed.
- Native DSP and real-processor integration tests passed (2/2).
- DSP checks: unity transparency, mono fold, mono-sum preservation at 300% width, rotation energy, channel swap, bass narrowing, finite extreme automation at 44.1/48/96/192 kHz, bypass.
- Processor checks: stereo layout, default unity, session state recall, zero reported latency, editor construction.
- Package installed successfully with Installer on the macOS runner.
- Installed AU and VST3 present at the standard /Library/Audio/Plug-Ins paths; ad-hoc code signatures verified.
- Apple auval: AU VALIDATION SUCCEEDED for aufx SoIm Mllu.
- Installed AU and VST3 both contain arm64 and x86_64 slices.
- Downloaded package and artifact SHA256 values matched CI outputs.
- Package SHA256: 4b51d7e0ef8a20844ef76e79f10d938600668617f67263e23d977205aea40731

The initial installer relocation failure was corrected by disabling bundle relocation in the component specification. The final build also corrects stereo scope left/right orientation.

## Windows native build: PASS
Build commit: 5f3faa0e2d767b3cfe988bf02c5e8d4c2ff9f2a0
Run: https://github.com/SeveriMallula/IronSqueeze/actions/runs/42028480682

- Windows x64 VST3 and standalone compiled using Visual Studio 2022.
- DSP and processor integration tests passed.
- Inno Setup EXE installer built successfully.
- Actual JUCE editor rendered using a software image; visible-pixel check and PNG export passed.

The later Windows commit changes only the test preview renderer and workflow selection, not the plugin code shipped in the macOS build.

## Limits
- No manual listening session or Logic/Reaper UI session was performed.
- Intel binaries were compiled and inspected; native runtime tests ran on the Apple Silicon macOS runner.
- Windows installer execution was not tested; compilation and packaging passed.
- Installers are unsigned. macOS plugins are ad-hoc signed; the package is not notarized.
