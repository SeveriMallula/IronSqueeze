# Iron Squeeze 0.2.1 native build validation

Build source commit: f8ec4efc0f756be96a1a7ccac02f5eb24a04e8c1
Run: https://github.com/SeveriMallula/IronSqueeze/actions/runs/41026240511
Date: 30 October 2026.

## Windows x64

- Native Visual Studio 2022 build of VST3 and standalone succeeded.
- DSP and processor tests passed.
- Inno Setup installer compilation succeeded.
- SHA256 of downloaded installer matches the build manifest.
- The installer has not been interactively run on a physical Windows computer.

## macOS universal

- Native Xcode build of AU, VST3 and standalone succeeded, targeting macOS 11.
- x86_64 and arm64 slices confirmed with lipo.
- DSP and processor tests passed on the Apple Silicon runner.
- pkgbuild produced the installer; system installation with installer -target /
  succeeded.
- Apple auval for aufx / IrSq / Mllu reported AU VALIDATION SUCCEEDED after
  installing the package and refreshing AudioComponentRegistrar.
- The installed component's plist and complete package payload were checked.
- SHA256 of downloaded package matches the build manifest.
- Intel runtime execution and macOS 11 runtime execution were not tested.

## GUI and modeling

The architecture/transfer/history panel is removed. The 1060x440 editor was
rendered and inspected in the local Linux build. Local Linux DSP, processor,
and actual VST3 module-loading checks also passed. DSP sanitizer checks from
0.2.0 remain applicable; the audio core is unchanged in 0.2.1.

This is a behavioral model with estimated thresholds, curves, filters and
harmonic stages. No component-level or measured hardware match is claimed.
No Reaper/Logic listening session, exhaustive CPU/aliasing benchmark, or full
Steinberg VST3 validator run has been completed.

Both installers are unsigned; the macOS package is not notarized.

## Checksums

Windows: 823b6a5fc2f4ac167208d21d7ee9f5d428bf4be4d7c70439518f5b1cd7d21371
macOS: 0076815e8f775ec51ec6ec48b3cc159847f9d6e8494b991781dcf18b9c6d4c25
