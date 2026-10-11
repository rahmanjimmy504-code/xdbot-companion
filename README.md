# xdBot Companion v1.2.5 — Android macro sharing

Independent Geode Android64 add-on targeting **Geometry Dash 2.2081 / Geode 5.10.1**. Install it *alongside* the official xdBot v2.7.7 release. This release uses the new ID `jimmy_1234ha.xdbot-companion` and credits **Jimmy_1234ha**. First remove the former `arena.xdbot-companion` add-on; otherwise both independent copies may show duplicate buttons/prompts. The ID change resets this add-on's settings to defaults, but does not touch your xdBot macros. It does not include, modify, repackage, or redistribute xdBot's binaries, code, images or sounds. Compatibility uses xdBot's `macros_folder` setting and the `LoadMacroLayer` runtime type name in the official Android64 release.

## Credits and scope

This is an independent companion for [xdBot](https://github.com/CamelliaOffical/XDBot-2.7) (`zilko.xdbot`) by Zilko; it is not an official xdBot release. It does not include xdBot code, binaries or artwork. Its folder-and-share-arrow icon was created for this add-on.

## Icon and changelog

The original folder-and-share-arrow icon is included as `logo.png`; `changelog.md` documents the release history. Neither includes xdBot artwork.

## Select and share saved macros

1. Open xdBot's **Load Macro** UI. The companion attempts to attach a smaller, light-outlined brown **SHARE MACROS** button beside **SELECT ALL** in the bottom row. Tap it to open the companion's blue macro picker. If that button does not appear, pause a level and use **XD TOOLS > MACROS** instead.
2. Tap saved macros to select them. **ALL** selects the current page; **CLEAR** deselects everything. **PREV/NEXT** change pages; selection persists. You can select previously saved files, not just the newest one.
3. Tap **SHARE** to open Android's share sheet with the selected file(s). Up to 30 attachments at a time. Select a receiving app and complete its flow to actually send. Nothing uploads without your choice.

The existing **Later / Share now** prompt for a newly saved macro remains optional; disable it with the companion's **Offer to share saved macros** setting if you prefer manual selection. The FPS/status/death panel and optional tiny HUD are unchanged.

## Share-path repair and troubleshooting

v1.2.0 opened the picker inside xdBot's UI, but a real-device screenshot showed `stage: send intent` on sharing. The cause was an incorrect JNI method descriptor: Android's `Intent.setClipData(ClipData)` returns **void**, not `Intent`. v1.2.1 uses the verified `(Landroid/content/ClipData;)V` descriptor and checks additional stages for future errors. v1.2.0 also added FileProvider/UserDirectoryProvider fallback. **The corrected share sheet still requires a phone retest**. If an error remains, please share the stage text, a screenshot, Android version and Geode launcher version (no private macro needed).

The picker displays direct files in xdBot's **Macro Save Location**: `.cml`, `.gdr`, `.gdr2`, and `.gdr.json`. It does not scan subfolders or autosaves; ordinary `.json` files are omitted to avoid sharing unrelated files. It lists the newest 1000 matching files among at most 5000 direct directory entries and refuses symlinks, files larger than 64 MB, and selections larger than 30 files. Custom macro paths outside the launcher's storage cannot be shared by its provider. File detection is passive (a folder watcher), not an xdBot save hook; the post-save prompt may appear if another app creates a matching file there, but still requires approval.

**UI integration is best-effort:** it identifies `LoadMacroLayer` by its C++ RTTI name (the method that worked in v1.2.1), then places the smaller separate framed button in the bottom row beside **SELECT ALL**, using an Android v1.2.3 screenshot for placement without scanning or casting xdBot's child nodes. No xdBot asset is copied into the add-on. If xdBot's class hierarchy, RTTI, or layout changes, the pause-menu fallback remains; this is not a change to the xdBot binary. The v1.2.2 crash report identified an unsafe `dynamic_cast` in the attempted search-control scan; v1.2.3 removes that scan entirely. The Android share chooser and corrected UI layout still need testing on a real device. Do not claim a device-verified fix yet.

## Status and review

**Not yet device-verified:** the corrected Android share chooser and the final bottom-row **SHARE MACROS** button placement still need testing on a real phone. Do not treat sharing as fully confirmed until that is done. The source was developed with AI assistance and needs human code review before any Geode Index submission.

## Build and test

Source is original companion code only. Build with Geode SDK v5.10.1 and GD 2.2081 bindings, Geode CLI 3.9.0, CMake/Ninja, Android NDK r28c; set `GEODE_SDK`, `GEODE_BINDINGS_REPO_PATH`, `ANDROID_NDK_ROOT`, then run `bash scripts/build-android64.sh`. The Arc/NDK workaround edits only a generated dependency file under the build directory. Run host scanner tests with:

```sh
g++ -std=c++20 -Wall -Wextra -Werror src/MacroScanner.cpp tests.cpp -o scanner-tests && ./scanner-tests
```
