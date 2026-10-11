# Windows/iOS development-build verification (after independent source review)

**Do not run this checklist yet unless a reviewer has approved the relevant platform build.** The Windows and iOS jobs compiled at [Actions run 38154368616](https://github.com/rahmanjimmy504-code/xdbot-companion/actions/runs/38154368616), commit `33d052f`, but neither has passed a device test. If code changes after that commit, test the *new* reviewed build, not these old artifacts. Neither CI artifact is a public release or a combined `xdbot-companion.geode`.

For a reviewer-approved test, the owner sends a volunteer the specific platform-labelled **DEV-DO-NOT-RELEASE-Windows-preview** or **DEV-DO-NOT-RELEASE-iOS-preview** Actions artifact and its SHA-256, plus the reviewed commit and build run. The volunteer must already have Geometry Dash 2.2081, Geode, and xdBot v2.7.7 on the platform. Confirm the artifact actually contains a `.geode` file for that platform; do not rename/install the Android64 release on Windows/iOS. Follow the platform's Geode installation instructions. Back up your own macros first; never provide private macros or credentials to the project.

Record: platform/OS version, game/Geode/xdBot versions, artifact filename, SHA-256, exact source commit/run, receiving app and redacted result. Use **two expendable test macros with different names and contents** (an old macro and a newly saved one), not private recordings.

1. Start the game and open xdBot's **Load Macro** screen. Confirm no crash, a visible bordered **SHARE MACROS** control that doesn't block SELECT ALL/other buttons, and a responsive companion picker.
2. Confirm both old and newly saved test macros appear. Select only the old one; tap **SHARE**. Confirm native OS sharing UI appears, select a receiving app that supports file attachments, and check the recipient obtained the **correct file content**, not just a filename/notification. Note recipient name and result.
3. Repeat with both distinct macros selected. Confirm the receiving app gets **two separate, correct files**; note any missing files or transformed names.
4. Open sharing again and **cancel** without selecting a recipient. Confirm no file was delivered, the game returns safely, and there is no crash or unintended upload.
5. Check the optional share-after-save prompt (if enabled): **Later** does nothing; **Share now** opens only the OS sharing UI and does not select a recipient by itself. Verify changing the xdBot macro location is respected if applicable.
6. If a test fails, capture the exact stage/error text, a redacted screenshot or crash log, and steps to reproduce. Never post macros, personal paths, account credentials, or unredacted logs.

**Pass gate:** independent review + successful one/two-file delivery + safe cancellation + stable xdBot UI on **each** platform. Compile success alone is not a pass. Android64's newer chooser fix and button layout also need phone verification before any combined release. No Linux/macOS test or release is planned.

## Android32 addition (beta.2 candidate)

Android32 is a **new, unverified target**. Do not infer support from Android64 or from the presence of an `.android32.so` in a package. Have a tester with a real **32-bit Android device and a working Android32 Geode + xdBot v2.7.7 installation** run the same old/new macro, one-file, two-file, cancellation and no-automatic-upload tests above. Record Android OS, CPU ABI as reported by the device, Geode/xdBot versions, artifact hash and redacted results. Keep the v1.2.5 Android64 release separate. No public four-platform release until all platform tests and code review pass.
