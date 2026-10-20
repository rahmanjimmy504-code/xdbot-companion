# Changelog

## v1.3.0-beta.18 — explicit in-level AI start/stop/verify (source only)
- Beta.17 built in run 39308059206, but its setting silently auto-started on level entry, switched itself OFF on input/restart/quit, and exposed no button. Beta.18 creates a compact START AI / STOP AI / VERIFY AI control in the gameplay UI, separate from the removed XD TOOLS panel. Confirm before starting, reset the current level, then run the bounded local timing heuristic. Entering a level alone does nothing.
- The Geode preference defaults OFF; confirming START AI enables it. Manual gameplay or restart, STOP AI, quitting and completion cancel the current attempt without secretly toggling that preference. Turning the preference OFF explicitly cancels AI. Verification still requires a second matching reported completion and xdBot import/replay remains unverified. No automatic website upload. Native beta.18 CI and phone test pending.

## v1.3.0-beta.17 — AI hook compile-header correction (source only)
- Add the required generated Geode `PlayLayer` and `GJBaseGameLayer` modify headers to the Android AI gameplay file. The previous beta.16 GitHub Actions run 39306690163 failed at `ModifyDerive` static assertions because these headers were missing. Do not assume the full native build or runtime succeeds until a new CI run and device tests pass. AI behavior and removed XD TOOLS/monitor are otherwise unchanged.

## v1.3.0-beta.16 — experimental AI macro attempt source (NO NATIVE BUILD)
- Combine all beta.15 independent companion features with an OFF-by-default Android gameplay experiment. A bounded local timing search proposes player-1 jump presses after failures, then asks the user to re-enter the same level for an exact repeat run. Only after two matching reported completions is a **local `.gdr.json` compatibility candidate** written to xdBot's configured macro folder. No auto upload or cloud API.
- Reject practice, platformer and dual modes; manual input, manual restart, the setting toggle and an attempt limit can cancel. This is not a general vision/ML agent; it may never finish a level. Game hooks, replay timing and xdBot import are untested. See `AI-RECORD-ANDROID64-STATUS.md` and do **not** release as a working AI mod.
- Keep XD TOOLS pause panel and the FPS/deaths HUD removed; native Android64 CI, device safety tests and imported-macro replay validation are mandatory. No beta.16 `.geode` exists yet.

## v1.3.0-beta.15 — REA-informed safety and confirmation (source candidate)
- REA 6.5.0 inventoried the authorized xdBot 2.7.7 Geode package. The Android64 ELF symbol table confirms distinct LoadMacroLayer import/list/search actions and macro GDR2 handling; this is metadata/symbol evidence, **not** decompiled original source or a complete recreation. See `REA-INVESTIGATION.md` for evidence and limits.
- Share preflight now reuses local-export checks for every selected file: only recognized macro filenames, nonempty regular files, no symlinks and at most 64 MiB each. Selecting files still does nothing until SHARE is tapped; validation does not parse replay contents.
- When a user manually selects a HYPE replay with Android's document picker, show an additional **ATTACH/CANCEL** confirmation before returning that URI to the site. The existing preselected-macro ATTACH prompt remains, and `.cml`/`.gdr2` still cannot go to HYPE. The website may process a file at attachment; do not promise that its separate Submit is the only transfer point.
- Retain beta.14's complete removal of the companion XD TOOLS panel and FPS/deaths monitor and all 14 original generated icons. Java/D8 and host tests pass locally; native Android64 CI and device verification **have not** passed for beta.15. No installable beta.15 `.geode` exists yet.

## v1.3.0-beta.14 — remove XD TOOLS and monitor (source candidate)
- Remove the companion-owned **XD TOOLS** pause-menu button and its panel entirely. Remove its FPS/deaths/last-death HUD, PlayLayer/PauseLayer hooks, and `monitor-start` setting. Keep only the separate **SHARE MACROS** entry beside xdBot's SELECT ALL and the companion picker containing HYPE, GDB, SHARE and EXPORT.
- No xdBot binary or macros are changed. This is source-only until a new Android64 native CI build and device test confirm the SHARE MACROS entry still appears and the removed panel/HUD do not. The beta.15 investigation above uses the subsequently supplied REA URL.

## v1.3.0-beta.13 — individually generated arcade-game icon sprites (Android64 source only)
- Generate **14 distinct original image assets**, one for each HYPE, GDB, PREV, NEXT, ALL, CLEAR, EXPORT, SHARE, GDR, JSON, CML, GDR2, selected and unselected icon. The artwork has bold saturated colours, heavy black outlines and geometric highlights inspired by Geometry Dash menu styling, without copying an existing game sprite. Store full-size originals in `art/source-icons/`; crop their white generator backgrounds into separate transparent 96px RGBA PNG sprites in `resources/icons/`. No vector placeholders or image-edit-derived icons are substituted.
- Enlarge native action, website and macro-row sprites versus beta.12. Expand the popup from 360×260 to 360×288 game units, space row icons and labels farther apart, and show five rows per page to accommodate larger art without vertical collision. This modifies C++ and **requires a new native Android64 CI build**; beta.12 cannot be relabelled or repacked as beta.13.
- Keep the wide GDB WebView and explicit Hyperbolus upload/SAF save behavior. Java/D8 and local policy tests may pass, but native compilation, on-device icon layout, GDB navigation and Hyperbolus account upload/download remain unverified for beta.13. Do not release or claim Android32/Windows/iOS website support.

## v1.3.0-beta.12 — large, simple game-style icons (Android64 source only)
- Follow-up phone feedback: beta.11's framed generated pictures are too small and do not read as icons. Replace them with 14 bold, uncluttered pictograms on transparent PNGs—no common square picture tile. Keep high-contrast gold, blue, green and red with dark outlines; use a larger 64px source canvas.
- Increase the **native** action-icon scale and list selection/file-format icon scales. Space the selection and file images apart, shift row labels right, change row spacing to 24px and show six macros per page instead of seven. Move status away from the button row. Changes to native UI require a **new Android64 C++ CI build**: do not repack beta.9's old native library or claim a working installable beta.12 yet.
- Keep the proposed wider GDB WebView from beta.10 and all existing upload/download confirmation rules. Native compile, display geometry, website layout and Fire tablet tests of beta.12 are still pending.

## v1.3.0-beta.11 — logo-style generated images replace the UI icons
- The user rejected the beta.10 hand-drawn picture symbols: they wanted generated **images like the mod logo**. All 14 in-game icon PNGs are now raster picture-style assets with glossy cobalt, copper and gold game-item artwork. Ten were generated individually with the mod logo as a style reference; the other four are derivatives of those generated images for JSON, CML, GDR2 and selected state. `art/source-icons/` holds the ten original generated images and `scripts/create-icons.py` reproducibly derives all 14 separate 48px PNG assets. No xdBot-owned imagery is copied.
- Keep beta.10's proposed GDB landscape-dialog fix and Android Java bridge unchanged. Local testing repacks the inspected, byte-identical beta.9 CI native ELF with the beta.11 artwork, existing beta.10 DEX and new metadata. This is an Android64 device-test candidate, not a beta.11 CI-built release. Website behavior and final icon appearance require phone tests; Android32/Windows/iOS still await porting.

## v1.3.0-beta.10 — Android64 wider GDB window and larger illustrated icons (device test pending)
- The Fire tablet's beta.9 screenshot confirmed GD Browser opened, but its default AlertDialog left a roughly 670px-wide WebView on a 1568px-wide landscape screen; GDBrowser itself showed “Yikes! Your screen isn't wide enough.” Set the GDB dialog to 94% of the screen width after showing it, use the wide viewport and overview settings, and make its WebView height 58% of the screen. Keep Hyperbolus's existing dialog sizing and keyboard fix unchanged. This is a layout hypothesis, not yet phone-confirmed.
- Replace the 14 thin 32px outlined symbols with 14 original, fully drawn 48px PNG illustrations: browser window, magnifier, chunky arrows, check card, delete card, download tray, share plane, file cards and selection badges. Existing native sprite scaling makes them roughly 50% larger without changing the CI-built native library. They remain real packaged image assets rather than emoji or font glyphs.
- Android Java/D8 and website-policy tests pass. A local device-test repack uses the byte-identical **beta.9 CI-built Android64 native ELF** plus the new DEX, PNGs and beta.10 metadata. This specific repack was not built by GitHub CI; layout, icons and website browsing still need phone testing. Do not release it or claim Android32/Windows/iOS support.

## v1.3.0-beta.9 — Android64 icon and GDBrowser source candidate
- Add 14 original small icon PNG resources to the companion's action buttons, macro-format rows, selection state and status; preserve readable labels. Put a new GDB button beside HYPE with a narrower search field. No xdBot-owned art or child scan is used.
- Add a **separate in-game GD Browser WebView** at https://gdbrowser.com/ from the macro panel, plus HYPE/GDB switches in the browser controls. Each switch creates its own site WebView; no selected macro is handed to GDB automatically. HTTPS host checks distinguish the two sites. GD Browser's own forms and same-site navigation work; a site file request asks for explicit user confirmation and uses Android's document picker.
- Keep the beta.8 Fire OS keyboard fix, beta.6 URI-based local EXPORT, Hyperbolus's supported-format/confirmation rules and SAF downloads. Hyperbolus still depends on its server accepting a valid level ID (15031583 returned a server-side 400); this mod does not bypass that.
- Java/D8 and local policy checks pass. Android64 native CI compilation passed at run 38951809249, but verification failed because Geode CLI flattened icon PNGs declared as `resources.files`. Package the `icons` directory through `resources.folders` instead; a simulated package-layout check passes, but a new real CI run and phone tests are still required. The final other-platform ports are pending; do not release this candidate.

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
