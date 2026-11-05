# Sound Imager 0.1.0 validation status

Completed on 2026-11-05:
- Portable DSP test executable compiled using GNU C++ 13.3 and passed.
- Default unity transparency on 10,000 random stereo samples.
- Width-zero mono output.
- Mono-sum preservation with 300% width.
- Energy preservation at 45-degree L/R rotation.
- Channel swap and bypass.
- 30 Hz side attenuation with 200 Hz bass narrowing.
- Finite output during extreme smoothed control changes at 44.1, 48, 96 and 192 kHz.
- Source syntax checks passed against pinned JUCE 8.0.12 for processor, editor and integration tests.

Pending:
- Full native Windows and macOS compilation.
- JUCE-linked processor and state tests (included).
- Interface render from the compiled editor (CI step included).
- Native AU validation (CI step included).
- Installer payload, architecture and hash checks (CI steps included).
- Manual listening and DAW testing.

No Windows EXE or macOS PKG has been generated yet. Native builds will run on the isolated build/sound-imager branch in SeveriMallula/IronSqueeze.
