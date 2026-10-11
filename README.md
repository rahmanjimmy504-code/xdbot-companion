# xdBot Companion — Windows/iOS port preview source

> **DEVELOPMENT BRANCH ONLY. Do not publish or distribute the CI artifacts as a finished cross-platform mod.** The only existing tested binary release is the separate v1.2.5 Android64 package. This preview branch has a refactored Android32/64 backend, an iOS UIKit prototype that **compiled in GitHub Actions** but has not been tested on a device, and a Windows desktop Share UI candidate that **compiled in GitHub Actions** but has not been runtime-tested. The Geode Index previously rejected the AI-generated implementation as "vibecode"; this branch is not a fix for that rejection.

**In-scope targets:** Android32, Android64, Windows and iOS. No native Linux or macOS target. The internal ID remains `jimmy_1234ha.xdbot-companion`. This branch declares a development version `v1.3.0-beta.2` so that it cannot be confused with the v1.2.5 release.

## What changed in this branch

- Extracted common file-count, regular-file, symlink and 64MB checks to `src/ShareMacros.cpp` (host tests in `tests-share.cpp`).
- Retained the Android JNI backend in `src/AndroidShare.cpp` (Android64 cross-build passed; the OS share flow still needs device verification).
- Added `src/IOSShare.mm`, a UIKit `UIActivityViewController` prototype that compiled in GitHub Actions but still needs sandbox checks and an iOS tester.
- Replaced the Windows fail-closed placeholder with a **candidate** `IDataTransferManagerInterop` Share UI backend. It compiled in GitHub Actions at commit `33d052f` but has not passed a real transfer test. Do not call it working yet.
- Added a **manual/branch-push compile workflow** for Android32, Android64, Win64 and iOS. It uses the official Geode combine action and uploads a developer-only four-platform candidate named `xdbot-companion.geode`; it never creates a release. Android32 has not yet compiled or passed a device test.
- Added GitHub issue templates for volunteer/test reports.

## Test locally

```sh
g++ -std=c++20 -Wall -Wextra -Werror src/MacroScanner.cpp tests.cpp -o scanner-tests && ./scanner-tests
g++ -std=c++23 -Wall -Wextra -Werror src/ShareMacros.cpp tests-share.cpp -o share-tests && ./share-tests
```

For Android64, use `scripts/build-android64.sh` with Geode SDK 5.10.1, compatible bindings, Geode CLI 3.9.0 and Android NDK r28c. This source built successfully for Android64 in the sandbox; it has **not** been runtime-tested. Build Win64/iOS only through the draft workflow or platform toolchains, and treat compile success as **not** equivalent to feature support.

See `UPLOAD-INSTRUCTIONS.md` for the exact branch upload, CI and tester steps. The original v1.2.5 release is unchanged; do not overwrite it.
