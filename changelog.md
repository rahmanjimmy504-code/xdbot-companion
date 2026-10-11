# Changelog

## v1.3.0-beta.1 — development source only
- Extracted shared file validation while retaining the Android JNI backend.
- Added an **untested iOS UIKit share-sheet prototype** and a Windows fail-closed placeholder. **No functional Windows sharing build exists yet.**
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
