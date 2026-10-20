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

## Beta.3 search field checks

Search for old and new macro filenames with differing capitalization; verify results update, no-results state is safe, paging matches the filtered list, page ALL selects only visible entries, and selections made in one query remain selected after changing or clearing the query. The status must show the total selected count even if some selections are hidden by a query. Confirm neither typing nor selecting uploads or shares files; the user must explicitly tap a share action. There is no Hyperbolus upload in beta.3.

## Beta.18 AI control regression — native and device tests pending

Beta.17 built in run 39308059206 but on device the user reported no AI button and the setting switched itself OFF. Beta.18 must receive a NEW Android64 CI build; its host-only source test does not prove a touch control works.

1. Confirm the beta.18 metadata, Android64 ELF, 14 icons and WebView DEX in the **new** artifact. CI checks that the native binary has START AI, STOP AI and VERIFY AI control IDs, the preference defaults OFF, and the old XD TOOLS/monitor strings are absent. Never relabel beta.17.
2. On a disposable classic cube level, check that START AI is visible and tappable without covering the game's pause or jump controls. Entering the level, ordinary taps and exiting must NOT silently change the preference or start AI. With the setting OFF, tapping START AI must show a consent dialog; Cancel must not start. Confirm must reset the current level to the beginning and show STOP AI. If it doesn't, report the exact step and screenshot; don't assume the AI is running.
3. Tap STOP AI and verify user control returns; a manual jump or restart also cancels only the active run. No auto upload. Try unsupported practice/dual/platformer: no AI input should begin. Recheck the Geode setting: it should stay ON after cancellation, and manually switching it OFF should cancel a running attempt.
4. Only after the control passes, test the bounded one-spike search. First completion must save nothing; re-enter and tap VERIFY AI; only a matching replay may write a candidate file. Test xdBot import/replay separately. No claim about arbitrary levels or a trained AI.

## Beta.17 experimental AI recorder gates — NOT device-verified

Beta.16 CI run 39306690163 failed on missing generated Geode modify headers; beta.17 adds them. This is a compile correction, not a verified Android AI player.

1. On a fresh Android64 CI run verify the native source compiles (new PlayLayer/GJBaseGameLayer hooks), metadata is `v1.3.0-beta.17`, 14 icons and WebView DEX remain packaged, and the native binary has `ai-record-experimental` but not `XD TOOLS` or `monitor-start`. A build failure blocks any installation; never relabel beta.13.
2. Use a disposable copied classic cube level, no valued progress or online account. First confirm the default AI setting is OFF, normal play unaffected, SHARE MACROS/HYPE/GDB and beta.14 XD TOOLS removal still work. Only then enable the setting in Geode's companion settings. Confirm manual input or manual restart stops AI and returns control.
3. Try a simple level with one jump hazard and a bounded number of retries. First reported completion must **not** create a file. Re-enter the same level; only a matching verified completion may create `companion-ai-*.gdr.json`. On a failed repeat, no candidate file should appear. Test interrupt, unsupported practice/platformer/dual, and max-attempt stop. Exact tick hooks and xdBot hook ordering are not yet validated.
4. Import the new file in xdBot and replay it on the same disposable level. Record whether it is accepted, exact frame timing, and any failures. Do not claim compatibility, success on arbitrary levels, or upload to HYPE until this test passes. No website or cloud service receives gameplay data automatically.

## Beta.15 Android64 REA-informed safety check (only after fresh native CI)

1. Run the beta.15 CI workflow. Do not install beta.13/beta.14 and call it beta.15. Inspect metadata for `v1.3.0-beta.15`, Android64 ELF with no companion XD TOOLS or `monitor-start` string, regenerated DEX with the manual HYPE confirmation and all 14 icon PNGs.
2. Open xdBot's Load Macro screen and use **SHARE MACROS**. With disposable `.gdr`, `.gdr.json`, `.cml` and `.gdr2` files, test multi-select SHARE, cancel, local EXPORT, and newly saved macro prompt. With an empty/removed/renamed file selected, SHARE should refuse to open an OS chooser. Never use private macros for testing.
3. On HYPE's replay upload page, manually choose a disposable `.gdr` or `.json` through Android documents. Check the *second* **Attach chosen replay?** confirmation. CANCEL must not return the file to the website; ATTACH may give it to the website immediately. A preselected macro must still ask before attachment. `.cml` and `.gdr2` remain local-only. No actual account upload is needed to validate the prompt.
4. Perform the beta.14 removal checks below on the beta.15 native build too. Record redacted screenshots and the actual `.geode` hash; native CI is not a phone test.

## Beta.14 Android64 removal check (only after fresh native CI)

1. Install a genuine beta.14 `.geode` alongside xdBot, replacing beta.13. Open the pause menu: **XD TOOLS must be absent**. Start and play a level: the companion FPS/deaths/last-death HUD must be absent. xdBot's own HUD, if enabled separately, is not this companion's responsibility.
2. Open xdBot's Load Macro UI. The separate companion-owned **SHARE MACROS** button should still open the picker. Check the five rows, large generated icon sprites, HYPE/GDB, SHARE, local EXPORT, selection and cancellation. Removing the pause fallback means the macro-screen entry is now the only in-game launcher for this picker; a missing entry is a blocking regression.
3. Test on Android64 first with expendable macros and a redacted screenshot. Android32/Windows/iOS are not port-certified by this run. This removal neither alters nor uninstalls xdBot itself.

## Beta.13 Android64 generated icon + larger panel check (after new native CI build only)

1. Inspect the actual beta.13 `.geode`: mod version beta.13, newly built Android64 ELF, the WebView DEX, and **14 distinct 96px transparent PNG resources** under `icons/`. Never install the source ZIP or relabel an older native library.
2. Replace beta.12 in Geode. Open the companion popup and confirm the five-row panel fits on the landscape tablet, has visibly larger **generated sprite** art on HYPE/GDB, action buttons, file-type rows and selection state, and has no clipped controls or overlapping labels. Try select/deselect, NEXT/PREV, ALL/CLEAR and filename search. If the enlarged popup crowds the screen, capture a redacted screenshot; CI cannot verify geometry on-device.
3. Open GDB with no selected macro; see if the wider WebView now shows the website instead of the blue **Yikes** warning. Test website switch to HYPE and back, keyboard input, and local EXPORT with a disposable macro. Do not submit private credentials/macros or falsely claim account download/upload works. Report precise errors and redacted screenshot.

## Beta.12 Android64 larger icon retest (only after a NEW native CI build)

1. Inspect the actual beta.12 package, confirm 14 transparent 64px icon resources and a newly built Android64 native library (not the reused beta.9 ELF). Replace older companion versions before installing. Do not treat this source ZIP as an installable package.
2. Open the macro panel: all button icons should be visibly bigger, simple pictograms with no square photo frame. Six file rows per page should show separate selection and file-type symbols without overlap, clipped text, or a crash; test selecting/deselecting, PREV/NEXT, ALL/CLEAR and status. Test search and local EXPORT with an expendable macro.
3. Tap GDB and check the site in the wider landscape view; test navigation to HYPE and back. If the website still shows its **Yikes** warning, capture the dialog screenshot. Do not use private macro files or send credentials/logs. This remains unverified until a Fire tablet test.

## Beta.11 Android64 generated image retest

1. Replace the earlier candidate with the beta.11 Android64 `.geode` file; verify HYPE/GDB, navigation, file-type and selection-state icons now look like glossy **raster image art**, in the style of the mod logo, not flat drawn glyphs. All 14 pictures should be visible and distinguishable; tap a row to confirm the selection graphic changes without crashing. A few are derived from the ten separately image-generated originals due to the per-turn image generation cap.
2. With no macro selected, open GDB. Confirm the wider landscape dialog removes the website's **Yikes!** screen. Test BACK, CLOSE and GDB → HYPE → GDB. Do not test with private files or share passwords/logs. The wider view has not yet passed a tablet retest.
3. If images appear too small or overlap, send a redacted screenshot with the panel unobscured. This is a repack of beta.9's CI-built Android64 native binary with beta.10's compiled DEX and beta.11's generated PNG art; no beta.11 native CI run has occurred.

## Beta.10 Android64 GDB width and illustrated icon retest (after beta.9 tablet screenshot)

1. Replace beta.9 with the labelled beta.10 Android64 device-test repack. Confirm the 14 icon+text button/row resources are visibly larger, colourful **picture-style images** rather than tiny thin outlines, and that buttons do not overlap or clip. Try selecting and deselecting a macro row.
2. Tap GDB with no macro selected. On the landscape Fire tablet, confirm the dialog uses most of the screen width and the website shows the real search/level page instead of its blue **Yikes! Your screen isn't wide enough** warning. Check that the CLOSE and BACK buttons remain reachable. This layout has not been phone-verified.
3. Switch GDB → HYPE → GDB, try public GDB search and keyboard entry, and confirm no macro or credentials were silently transferred. On a small phone, report if the site still refuses its viewport rather than pretending the fix is universal. Use no private macro in a website test.
4. Retest local EXPORT with a disposable macro to ensure the Java DEX change did not regress native SAF save. Record redacted screenshots and exact errors. This beta.10 package reuses the *byte-identical beta.9 CI native ELF*; beta.10 has no full native CI run yet.

## Beta.9 Android64 HYPE + GDB icon/navigation check (CI built, Fire tablet showed narrow GDB view and tiny icons)

1. Confirm icon+text buttons, 14 bundled resources, shortened search field, one HYPE and one GDB button fit without clipping on the macro panel. Row file-format icon and selection ring/check must match each file and toggle without a crash; no xdBot-owned UI scan.
2. With no selected macro, tap GDB. Confirm a separate GD Browser website window opens in-game, with no automatic macro attachment; search or view a public level and use BACK. External off-site links should be blocked rather than silently sent elsewhere. Do not provide any account credentials.
3. Tap HYPE in the GDB window and GDB in the HYPE window. Confirm each site gets a fresh WebView and a working keyboard. HYPE's supported-format checks, per-file attachment confirmation and own Submit still apply. Do not upload a private file.
4. GDB website file requests, if any, should ask for confirmation *before* Android's picker. Test CANCEL with no file transfer. A website may itself process a file after a user chooses it; do not test with a private macro.
5. Local EXPORT should still invoke Android's save picker and save the correct bytes. A successful native build alone is not a website or upload test. Record version, redacted screenshots, and any precise error. Do not use an unrelated Geometry Dash level ID to bypass Hyperbolus's `400 Level could not be fetched` response.
