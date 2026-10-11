# xdBot Companion — Windows/iOS port preview source

> **DEVELOPMENT BRANCH ONLY. Do not publish or distribute the CI artifacts as a finished cross-platform mod.** The only existing tested binary release is the separate v1.2.5 Android64 package. This preview branch has a refactored Android backend, an iOS UIKit prototype that has **not** been compiled on macOS or tested on a device, and a Windows backend that **deliberately refuses to share** until a native implementation is reviewed and tested. The Geode Index previously rejected the AI-generated implementation as "vibecode"; this branch is not a fix for that rejection.

**In-scope targets:** Android64, Windows and iOS. No native Linux or macOS target. The internal ID remains `jimmy_1234ha.xdbot-companion`. This branch declares a development version `v1.3.0-beta.1` so that it cannot be confused with the v1.2.5 release.

## What changed in this branch

- Extracted common file-count, regular-file, symlink and 64MB checks to `src/ShareMacros.cpp` (host tests in `tests-share.cpp`).
- Retained the Android JNI backend in `src/AndroidShare.cpp` (Android64 cross-build passed; the OS share flow still needs device verification).
- Added `src/IOSShare.mm`, a UIKit `UIActivityViewController` prototype that needs a macOS/Xcode build, sandbox checks and an iOS tester.
- Added `src/WindowsShare.cpp` as a **fail-closed placeholder**, not a Windows port. A developer must implement file-based Windows Share Sheet support (e.g. the desktop `IDataTransferManagerInterop` flow), with one/two-file delivery tests.
- Added a **manual/branch-push compile workflow** for Win64 and iOS. It uploads developer-only Actions artifacts and never creates a release.
- Added GitHub issue templates for volunteer/test reports.

## Test locally

```sh
g++ -std=c++20 -Wall -Wextra -Werror src/MacroScanner.cpp tests.cpp -o scanner-tests && ./scanner-tests
g++ -std=c++23 -Wall -Wextra -Werror src/ShareMacros.cpp tests-share.cpp -o share-tests && ./share-tests
```

For Android64, use `scripts/build-android64.sh` with Geode SDK 5.10.1, compatible bindings, Geode CLI 3.9.0 and Android NDK r28c. This source built successfully for Android64 in the sandbox; it has **not** been runtime-tested. Build Win64/iOS only through the draft workflow or platform toolchains, and treat compile success as **not** equivalent to feature support.

See `UPLOAD-INSTRUCTIONS.md` for the exact branch upload, CI and tester steps. The original v1.2.5 release is unchanged; do not overwrite it.
