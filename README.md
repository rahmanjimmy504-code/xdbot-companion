# xdBot Companion — Windows/iOS port preview source

> **DEVELOPMENT BRANCH ONLY. Do not publish or distribute the CI artifacts as a finished cross-platform mod.** The separate v1.2.5 Android64 release remains available; the beta.2 four-platform package compiled but has not passed device transfer tests. This preview branch has a refactored Android32/64 backend, an iOS UIKit prototype that **compiled in GitHub Actions** but has not been tested on a device, and a Windows desktop Share UI candidate that **compiled in GitHub Actions** but has not been runtime-tested. The Geode Index previously rejected the AI-generated implementation as "vibecode"; this branch is not a fix for that rejection.

**In-scope targets:** Android32, Android64, Windows and iOS. No native Linux or macOS target. The internal ID remains `jimmy_1234ha.xdbot-companion`. This branch declares a development version `v1.3.0-beta.3` so that it cannot be confused with the v1.2.5 release.

## What changed in this branch

- Extracted common file-count, regular-file, symlink and 64MB checks to `src/ShareMacros.cpp` (host tests in `tests-share.cpp`).
- Retained the Android JNI backend in `src/AndroidShare.cpp` (Android64 cross-build passed; the OS share flow still needs device verification).
- Added `src/IOSShare.mm`, a UIKit `UIActivityViewController` prototype that compiled in GitHub Actions but still needs sandbox checks and an iOS tester.
- Replaced the Windows fail-closed placeholder with a **candidate** `IDataTransferManagerInterop` Share UI backend. It compiled in GitHub Actions at commit `33d052f` but has not passed a real transfer test. Do not call it working yet.
- Added a **manual/branch-push compile workflow** for Android32, Android64, Win64 and iOS. It uses the official Geode combine action and uploads a developer-only four-platform candidate named `xdbot-companion.geode`; it never creates a release. Android32 compiled in beta.2 CI but has not passed a device test.
- Added GitHub issue templates for volunteer/test reports.
- Added a case-insensitive, local-only filename search to the Share Macros popup (beta.3 candidate). It has not passed four-platform CI or UI tests yet. Hyperbolus sign-in/website uploading is **not implemented**: do not imply the mod can upload there.

## Test locally

```sh
g++ -std=c++20 -Wall -Wextra -Werror src/MacroScanner.cpp tests.cpp -o scanner-tests && ./scanner-tests
g++ -std=c++23 -Wall -Wextra -Werror src/ShareMacros.cpp tests-share.cpp -o share-tests && ./share-tests
g++ -std=c++23 -Wall -Wextra -Werror tests-search.cpp -o search-tests && ./search-tests
```

For Android64, use `scripts/build-android64.sh` with Geode SDK 5.10.1, compatible bindings, Geode CLI 3.9.0 and Android NDK r28c. This source built successfully for Android64 in the sandbox; it has **not** been runtime-tested. Build other platforms through the draft workflow or platform toolchains, and treat compile success as **not** equivalent to feature support.

See `UPLOAD-INSTRUCTIONS.md` for the exact branch upload, CI and tester steps. The original v1.2.5 release is unchanged; do not overwrite it.
