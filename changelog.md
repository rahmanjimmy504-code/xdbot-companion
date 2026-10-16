# Changelog

## v1.3.0-beta.9 — Android64 icon and GDBrowser source candidate
- Add 14 original small icon PNG resources to the companion's action buttons, macro-format rows, selection state and status; preserve readable labels. Put a new GDB button beside HYPE with a narrower search field. No xdBot-owned art or child scan is used.
- Add a **separate in-game GD Browser WebView** at https://gdbrowser.com/ from the macro panel, plus HYPE/GDB switches in the browser controls. Each switch creates its own site WebView; no selected macro is handed to GDB automatically. HTTPS host checks distinguish the two sites. GD Browser's own forms and same-site navigation work; a site file request asks for explicit user confirmation and uses Android's document picker.
- Keep the beta.8 Fire OS keyboard fix, beta.6 URI-based local EXPORT, Hyperbolus's supported-format/confirmation rules and SAF downloads. Hyperbolus still depends on its server accepting a valid level ID (15031583 returned a server-side 400); this mod does not bypass that.
- Java/D8 and local policy checks pass. Android64 C++ CI, resource packaging, screen layout and both website flows in beta.9 are **not yet phone-tested**. The final other-platform ports are pending; do not release this candidate.

## v1.3.0-beta.8 — Android64 website keyboard test, unreviewed
- Phone test after enabling Amazon System WebView confirmed HYPE opens, but the website input keyboard does not appear. Make the WebView report that it hosts a text editor and keep its AlertDialog window connected to the Android IME; do not open a mod-owned password field or capture key events.
- Retain beta.7 failure diagnostics and the beta.6 CI-built native binary with its phone-confirmed local EXPORT. This Java DEX-only repack has passed compilation, not a beta.8 CI build or keyboard phone test. Do not treat login/upload/download as verified.

## v1.3.0-beta.7 — Android64 HYPE diagnostic, not a confirmed fix
- Keep the beta.6 Android64 native binary and Android EXPORT behavior. Replace only the embedded Java DEX to display the exact HYPE failure stage, exception name and message, and log a stack trace locally. This distinguishes a missing Android WebView provider from a window or page-load error.
- Beta.6 HYPE still fails on a phone with `AndroidRuntimeException`; do not enter a Hyperbolus password or call this a working integration. This diagnostic package is repacked from the inspected beta.6 CI artifact; beta.7 native CI and phone tests have not run.

## v1.3.0-beta.6 — Android document URI fix, development candidate
- Fix HYPE's pre-browser picker setup failure seen on a phone: remove reflection over launcher-specific contract names. A companion-owned Android framework Fragment starts the Android document UI and receives actual SAF content URIs for website file choice and account downloads; do not use Geode's guessed filesystem path. Unlike AndroidX classes in the released Geode launcher, Android SDK framework class names cannot be obfuscated by the launcher.
- Enable Android EXPORT for one selected local macro through ACTION_CREATE_DOCUMENT and ContentResolver streams. This handles .gdr, .gdr.json, .cml and .gdr2 locally; .cml/.gdr2 remain unsupported for Hyperbolus uploads.
- No new third-party Java dependencies. Clean Java builds discard stale class files. Java/D8 compilation passed; CI and device testing of beta.6 remain pending.

## v1.3.0-beta.5 — Android-only Hyperbolus native WebView source candidate, unverified
- Add an Android WebView dialog **inside Geometry Dash** for Hyperbolus's own email/password page and website navigation; no mod-owned password field or external Custom Tab.
- From the macro popup, HYPE with one supported local `.gdr`/`.gdr.json` offers that file to the site's upload form after a per-attachment confirmation. `.cml` and `.gdr2` are explicitly rejected for the website; nothing uploads until the user submits Hyperbolus's own form. No preselection permits one supported file via Android's document picker.
- Browser download links offer Android's SAF create-document picker, stream bytes into the chosen URI, and do not send account cookies to redirected storage. Browser/chooser/download behavior is **untested on a phone** and may fail; CI compile success would not certify it.
- Android64 is the first development target. Android32, Windows and iOS native Hyperbolus browser ports remain to be built; this is not a combined package or release.

## v1.3.0-beta.4 — local export source candidate, not a release
- Add an EXPORT action for **one selected local macro**. Windows uses Geode's native save picker; iOS uses UIKit's document export picker to avoid Geode 5.10.1's empty-file staging behavior. No export happens until the user taps EXPORT and selects a destination. Both picker flows need CI and device checks.
- On Android32/64, EXPORT shows an honest unsupported warning and points to the existing SHARE action, where the user may choose a Files app to save the file. Geode's Android SaveFile API drops the SAF content URI, guesses a filesystem path, and even ignores the Downloads directory hint; copying to that path could claim success for the **wrong** file. A safe native Android save integration is still pending.
- Local `.cml` and `.gdr2` files are eligible for **local export**; Hyperbolus website uploading of these formats remains unsupported. This version does **not** include Hyperbolus login, Hyperbolus account downloads, or in-game website browsing. No website credentials are collected.

## v1.3.0-beta.3 — search-bar development candidate
- Add a case-insensitive filename search field to the Share Macros popup. Selected files remain selected when the query changes, and page ALL selects only visible matches. Nothing is sent by searching or selecting.
- Hyperbolus website login/upload is **not** included: its current public upload form is authenticated, handles one file and requires a level ID; `.cml` and `.gdr2` are rejected. In-game website authentication/upload awaits a site-approved integration and platform review.

## v1.3.0-beta.2 — development source only
- Add an Android32 build target to the existing four-platform developer preview. The official Geode combine action will produce one `xdbot-companion.geode` containing Android32, Android64, Win64 and iOS binaries if all CI jobs pass. **Android32 has not passed CI or a real device test yet.**
- No platform is being announced as fully functional based on compilation.

## v1.3.0-beta.1 — development source only
- Extracted shared file validation while retaining the Android JNI backend.
- Added an **untested iOS UIKit share-sheet prototype**; Windows now has an untested desktop Share UI candidate. **No Windows sharing test has passed.**
- Added manual Win64/iOS compile checks and tester templates. Do not release CI artifacts as multiplatform support.


## v1.2.5 — 2026-10-11
- Added an original folder-and-share-arrow mod icon.
- Changed the displayed developer to **Jimmy_1234ha** and renamed the mod ID to `jimmy_1234ha.xdbot-companion`.
- Added this changelog. No sharing or UI-control logic changed from v1.2.4.
- **Migration:** uninstall the former `arena.xdbot-companion` before installing this version. Geode regards the new ID as a separate mod, and this add-on's settings reset to defaults. Your official xdBot installation and macro files are unaffected.

## v1.2.4 — 2026-10-11
- Made the brown bordered Share Macros button smaller and moved it beside Select All in xdBot's Load Macro bottom row. Placement awaits phone confirmation.

## v1.2.3 — 2026-10-11
- Removed the recursive `dynamic_cast` search-control scan that caused an Android crash in v1.2.2. Restored the safer popup attachment method; user confirmed it opened without that crash.

## v1.2.2 — 2026-10-11
- Introduced the framed button. The search-control scan caused an on-device crash and was removed in v1.2.3. **Do not install this version.**

## v1.2.1 — 2026-10-11
- Corrected Android JNI's `Intent.setClipData(ClipData)` method signature from an incorrect return type to `void`. The share chooser still needs confirmation on a phone.

## v1.2.0 — 2026-10-11
- Added a button in xdBot's Load Macro UI and a separate multi-selection macro picker. Its Android send-intent bug was repaired in v1.2.1.

## v1.1.0 — 2026-10-11
- Added an opt-in post-save share prompt and pause-menu access to the companion tools.
