# Apple signing and notarization setup

Status: workflow prepared; no Apple signing credentials are configured and no
notarized package has been produced. The published 0.2.1 installer remains
unsigned/unnotarized until a successful signed release job replaces it with a
separately named Notarized package.

Apple requires Developer ID certificates and authorized notary authentication.
The original unsigned package cannot acquire a valid Developer ID signature
without the certificate private keys. A signed release rebuild signs the
payload before creating and notarizing its installer.

## Account material required

An active Apple Developer Program membership with access to Developer ID
Application and Developer ID Installer certificates and their private keys.
For this workflow, export both identities into one password-protected .p12.
A certificate-only .cer does not contain a signing private key.

Configure these encrypted Actions secrets directly in GitHub:

| Secret | Value |
|---|---|
| APPLE_SIGNING_P12_BASE64 | Base64 of the .p12 containing both Developer ID identities/private keys |
| APPLE_SIGNING_P12_PASSWORD | The export password protecting that .p12 |
| APPLE_APPLICATION_IDENTITY | Full Developer ID Application certificate display name |
| APPLE_INSTALLER_IDENTITY | Full Developer ID Installer certificate display name |
| APPLE_ID | Apple account used for notarization |
| APPLE_TEAM_ID | Developer team identifier |
| APPLE_APP_SPECIFIC_PASSWORD | Apple app-specific password for notarytool; not the account's normal password |

Do not commit credentials, private keys, certificate exports or passwords to
this public repository, and do not paste them into chat. Exporting new signing
identities or storing them in Actions requires the account owner's explicit
approval. For a shared repository, restrict the signing environment to trusted
maintainers and release refs before adding signing secrets.

## Workflow and results

.github/workflows/notarize.yml is restricted to a push to release/notarize or
a manual dispatch. GitHub only exposes manual dispatch when the workflow is
also present on the default branch. Until then, an authorized release-branch
push can trigger it. No pull-request or fork event runs the signing workflow.

The job checks required secrets, imports identities into a temporary keychain,
authenticates notarytool, builds universal binaries and runs the processor/DSP
tests. Packaging signs Mach-O code and bundles from the inside out with secure
timestamps and hardened runtime. The standalone app receives only the audio
input entitlement; it does not get JIT, debugging or library-validation bypasses.

It signs the .pkg with Developer ID Installer, requires Apple's JSON status
Accepted, staples and validates the approval ticket, and asks spctl to assess
the installer. Only after these steps does it create:

    IronSqueeze-0.2.1-macOS-Notarized.pkg

The signed release is then installed on the macOS runner and checked by auval.
The job uploads only the accepted installer, its checksum and Apple's non-secret
submission result. Signing material is deleted in an always-run cleanup step;
the GitHub-hosted runner is also destroyed at the end of the job.

The preparation has passed shell syntax, YAML and entitlement-plist checks.
Actual signing, Apple's acceptance and Gatekeeper assessment remain untested
until the account owner supplies the required signing material.

Primary references:
- https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution
- https://developer.apple.com/documentation/security/resolving-common-notarization-issues
- https://docs.github.com/en/actions/how-tos/deploy/deploy-to-third-party-platforms/sign-xcode-applications
