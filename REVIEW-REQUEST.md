# Ready-to-post GitHub issue: independent platform review

**Title:** Review request: xdBot Companion Windows/iOS development sharing backends (not a release)

**Body:**

The `port-preview` branch of `rahmanjimmy504-code/xdbot-companion` contains candidate Windows and iOS sharing backends for the separate xdBot Companion add-on (`jimmy_1234ha.xdbot-companion`). Target setup: Geometry Dash 2.2081, Geode, xdBot v2.7.7. The intent is opt-in selection of one or multiple old/new saved macros from xdBot's Load Macro UI; no automatic upload or redistribution of xdBot assets. Native Linux/macOS are out of scope.

[GitHub Actions run 38154368616](https://github.com/rahmanjimmy504-code/xdbot-companion/actions/runs/38154368616) compiled both Win64 and iOS at commit `33d052f`. **Neither backend has been tested transferring files to a receiving app.** CI artifacts are developer-only and are not a release; v1.2.5's separately downloadable `.geode` file is Android64-only. The Index previously rejected an AI-generated submission as “vibecode”, so an independent human-led code review is needed before any tester installs these development artifacts or any Index resubmission.

Please review `src/WindowsShare.cpp`, `src/IOSShare.mm`, `src/ShareMacros.cpp`, `src/MacroPicker.cpp`, `src/main.cpp`, `CMakeLists.txt`, and `mod.json`. In particular:

- Windows desktop COM/WinRT apartment/threading, `IDataTransferManagerInterop` and `DataRequested` event lifetime/deferral, `StorageFile` access, multi-file transfer, cancellation, error reporting and Unicode paths.
- iOS active window/presentation context, main-thread UI, sandbox/file URL access, source popover on iPad, multi-file delivery, cancellation and error reporting.
- xdBot Load Macro UI integration across platforms, folder discovery, no unsafe RTTI casts into xdBot-owned children, no automatic file sharing, and validation of missing/symlinked/oversized files.
- Packaging/permissions/dependencies and risk of undesired network operations or xdBot asset redistribution.

Please comment with findings or a reviewed commit/PR; do **not** mark Windows/iOS support as working on compilation alone. Do not post private macros, full personal paths, credentials, or unredacted logs. The separate `TEST-PLAN.md` defines the subsequent on-device one/two-file checks.
