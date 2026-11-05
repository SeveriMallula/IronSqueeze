#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."
artifacts="${1:-build/SoundImager_artefacts/Release}"
app_identity="${APP_SIGN_IDENTITY:--}"
installer_identity="${INSTALLER_SIGN_IDENTITY:-}"
require_notarization="${REQUIRE_NOTARIZATION:-0}"
notarize=0
if [[ -n "${NOTARY_PROFILE:-}" || "$require_notarization" == 1 ]]; then
  notarize=1
  [[ "$app_identity" == "Developer ID Application:"* ]] || { echo 'Developer ID Application identity required.' >&2; exit 1; }
  [[ "$installer_identity" == "Developer ID Installer:"* ]] || { echo 'Developer ID Installer identity required.' >&2; exit 1; }
  [[ -n "${NOTARY_PROFILE:-}" ]] || { echo 'Notary authentication profile required.' >&2; exit 1; }
fi
stage="$(mktemp -d)"
trap 'rm -rf "$stage"' EXIT
mkdir -p "$stage/Library/Audio/Plug-Ins/VST3" "$stage/Library/Audio/Plug-Ins/Components" "$stage/Applications/Mallula Audio" dist
cp -R "$artifacts/VST3/Sound Imager.vst3" "$stage/Library/Audio/Plug-Ins/VST3/"
cp -R "$artifacts/AU/Sound Imager.component" "$stage/Library/Audio/Plug-Ins/Components/"
cp -R "$artifacts/Standalone/Sound Imager.app" "$stage/Applications/Mallula Audio/"
sign_args=(--force --sign "$app_identity")
if [[ "$app_identity" != '-' ]]; then
  sign_args+=(--timestamp --options runtime)
fi
# Sign Mach-O code explicitly from the inside out. Do not use --deep to sign.
while IFS= read -r -d '' binary; do
  if file -b "$binary" | grep -q 'Mach-O'; then
    codesign "${sign_args[@]}" "$binary"
  fi
done < <(find "$stage" -type f -print0)
while IFS= read -r -d '' bundle; do
  if [[ "$bundle" == *.app && "$app_identity" != '-' ]]; then
    codesign "${sign_args[@]}" --entitlements packaging/macos/standalone.entitlements "$bundle"
  else
    codesign "${sign_args[@]}" "$bundle"
  fi
  codesign --verify --deep --strict --verbose=2 "$bundle"
done < <(find "$stage" -depth -type d \( -name '*.framework' -o -name '*.xpc' -o -name '*.vst3' -o -name '*.component' -o -name '*.app' \) -print0)
# Prevent Installer from relocating bundles to matching build-tree copies.
pkgbuild --analyze --root "$stage" "$stage/components.plist"
python3 - "$stage/components.plist" <<'PYPLIST'
import plistlib,sys
p=sys.argv[1]
with open(p,'rb') as f: components=plistlib.load(f)
for component in components:
    component['BundleIsRelocatable']=False
    component['BundleHasStrictIdentifier']=True
    component['BundleOverwriteAction']='upgrade'
with open(p,'wb') as f: plistlib.dump(components,f)
PYPLIST
# Keep the component specification outside the payload root.
mv "$stage/components.plist" "$stage/../soundimager-components-$$.plist"
component_plist="$stage/../soundimager-components-$$.plist"
trap 'rm -rf "$stage"; rm -f "$component_plist"' EXIT
args=(--root "$stage" --component-plist "$component_plist" --identifier audio.mallula.soundimager --version 0.1.0 --install-location /)
if [[ -n "$installer_identity" ]]; then args+=(--sign "$installer_identity"); fi
package=dist/SoundImager-0.1.0-macOS.pkg
pkgbuild "${args[@]}" "$package"
if [[ "$notarize" == 1 ]]; then
  pkgutil --check-signature "$package"
  notary_args=(--keychain-profile "$NOTARY_PROFILE")
  if [[ -n "${NOTARY_KEYCHAIN:-}" ]]; then notary_args+=(--keychain "$NOTARY_KEYCHAIN"); fi
  # Apple may return an Invalid status: explicitly require Accepted in the JSON.
  xcrun notarytool submit "$package" "${notary_args[@]}" --wait --timeout 30m --output-format json > dist/notarization-result.json
  python3 - <<'PY'
import json,sys
result=json.load(open('dist/notarization-result.json'))
if result.get('status') != 'Accepted':
    sys.exit('Apple notarization was not accepted. No notarized package will be published.')
print('Apple notarization accepted. Submission:',result.get('id',''))
PY
  xcrun stapler staple "$package"
  xcrun stapler validate "$package"
  spctl --assess --type install --verbose=2 "$package"
  mv "$package" dist/SoundImager-0.1.0-macOS-Notarized.pkg
fi
