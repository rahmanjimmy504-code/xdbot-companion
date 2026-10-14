# xdBot Companion — Android/Windows/iOS port preview source

> **DEVELOPMENT BRANCH ONLY. Do not publish or distribute CI artifacts as a finished cross-platform mod.** The separate v1.2.5 Android64 release remains available. Beta.3 compiled for all four targets, but has not passed device transfer tests. The new beta.4 export code has passed host tests only and has **not** passed four-platform CI or device tests. The Geode Index previously rejected the AI-generated implementation as "vibecode"; this branch is not a fix for that rejection.

**In-scope targets:** Android32, Android64, Windows x64 and iOS arm64. No native Linux or macOS target. The internal ID remains `jimmy_1234ha.xdbot-companion`. This branch declares a development version `v1.3.0-beta.4` so that it cannot be confused with the v1.2.5 release.

## What changed in this branch

- Extracted common file-count, regular-file, symlink and 64MB checks to `src/ShareMacros.cpp` (host tests in `tests-share.cpp`).
- Retained the Android JNI backend in `src/AndroidShare.cpp` (Android64 cross-build passed; the OS share flow still needs device verification).
- Added `src/IOSShare.mm`, a UIKit `UIActivityViewController` prototype that compiled in GitHub Actions but still needs sandbox checks and an iOS tester.
- Replaced the Windows fail-closed placeholder with a **candidate** `IDataTransferManagerInterop` Share UI backend. It compiled in GitHub Actions at commit `33d052f` but has not passed a real transfer test. Do not call it working yet.
- Added a **manual/branch-push compile workflow** for Android32, Android64, Win64 and iOS. It uses the official Geode combine action and uploads a developer-only four-platform candidate named `xdbot-companion.geode`; it never creates a release. Android32 compiled in beta.2 CI but has not passed a device test.
- Added GitHub issue templates for volunteer/test reports.
- Added a case-insensitive, local-only filename search to the Share Macros popup (beta.3 compiled on all four CI targets; UI still untested).
- Added an EXPORT button to save **one** selected local macro through a native destination chooser on Windows and iOS (beta.4 source candidate). iOS uses UIKit document export directly; Windows uses Geode SaveFile. Android's stock Geode save picker loses the SAF content URI and may guess the wrong path, so Android EXPORT **does not save**: it points users to SHARE and a Files app instead. Android native SaveFile requires a new safe URI-based implementation. This is not a Hyperbolus account download.
- Hyperbolus in-game email/password sign-in, manual website uploads, and account downloads are **not implemented**. Do not imply these are included. A site-approved account endpoint and reviewed in-game browser support are outstanding. No automatic upload is performed.

## Test locally

```sh
g++ -std=c++20 -Wall -Wextra -Werror src/MacroScanner.cpp tests.cpp -o scanner-tests && ./scanner-tests
g++ -std=c++23 -Wall -Wextra -Werror src/ShareMacros.cpp tests-share.cpp -o share-tests && ./share-tests
g++ -std=c++23 -Wall -Wextra -Werror tests-search.cpp -o search-tests && ./search-tests
g++ -std=c++23 -Wall -Wextra -Werror tests-export.cpp src/MacroExportCore.cpp src/MacroScanner.cpp -o export-tests && ./export-tests
```

For Android64, use `scripts/build-android64.sh` with Geode SDK 5.10.1, compatible bindings, Geode CLI 3.9.0 and Android NDK r28c. This source built successfully for Android64 in the sandbox; it has **not** been runtime-tested. Build other platforms through the draft workflow or platform toolchains, and treat compile success as **not** equivalent to feature support.

See `UPLOAD-INSTRUCTIONS.md` for the exact branch upload, CI and tester steps. The original v1.2.5 release is unchanged; do not overwrite it.
