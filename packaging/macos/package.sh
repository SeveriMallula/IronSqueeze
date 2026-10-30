#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."
artifacts="${1:-build/IronSqueeze_artefacts/Release}"
stage="$(mktemp -d)"
trap 'rm -rf "$stage"' EXIT
mkdir -p "$stage/Library/Audio/Plug-Ins/VST3" "$stage/Library/Audio/Plug-Ins/Components" "$stage/Applications/Mallula Audio" dist
cp -R "$artifacts/VST3/Iron Squeeze.vst3" "$stage/Library/Audio/Plug-Ins/VST3/"
cp -R "$artifacts/AU/Iron Squeeze.component" "$stage/Library/Audio/Plug-Ins/Components/"
cp -R "$artifacts/Standalone/Iron Squeeze.app" "$stage/Applications/Mallula Audio/"
# Ad hoc signatures permit local testing; release distribution requires a Developer ID.
while IFS= read -r -d '' bundle; do
 codesign --force --deep --sign "${APP_SIGN_IDENTITY:--}" "$bundle"
done < <(find "$stage" -type d \( -name '*.vst3' -o -name '*.component' -o -name '*.app' \) -print0)
args=(--root "$stage" --identifier audio.mallula.ironsqueeze --version 0.2.1 --install-location /)
if [[ -n "${INSTALLER_SIGN_IDENTITY:-}" ]]; then args+=(--sign "$INSTALLER_SIGN_IDENTITY"); fi
pkgbuild "${args[@]}" dist/IronSqueeze-0.2.1-macOS.pkg
if [[ -n "${NOTARY_PROFILE:-}" ]]; then
 xcrun notarytool submit dist/IronSqueeze-0.2.1-macOS.pkg --keychain-profile "$NOTARY_PROFILE" --wait
 xcrun stapler staple dist/IronSqueeze-0.2.1-macOS.pkg
fi
