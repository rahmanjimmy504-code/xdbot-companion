# What to upload and what happens next

This is a **source-only developer preview**, not a universal `.geode`. The v1.2.5 Android release is unchanged.

1. In `https://github.com/rahmanjimmy504-code/xdbot-companion`, create a **new branch named `port-preview`** from `main`. Do not replace `main` or the v1.2.5 release asset.
2. Upload the *contents* of this preview folder to `port-preview`, preserving `.github/workflows/port-compile.yml`, `.github/ISSUE_TEMPLATE/`, `src/`, and `mod.json`. Commit as “Start Windows/iOS port preview — NOT FOR RELEASE”. If using Git locally, `git switch -c port-preview`, copy the files, `git add .`, `git commit`, then `git push -u origin port-preview`.
3. GitHub Actions should run **Port compile checks** automatically on a push to `port-preview` (or manually if enabled). Report the actual Windows/iOS compiler errors; do not label an Actions artifact a tested mod. This action does not merge binaries or create a GitHub Release.
4. Post the included Windows and iOS tester issue templates to recruit volunteers. **Wait for a reviewed functional build** before giving any artifact to testers. The Windows Share UI candidate has not passed compile or real-file transfer tests.
5. Have a developer implement and review Windows native file sharing and inspect the iOS UIKit prototype. Require real tests of receiving one and two macro files, cancellation, UI fit and crash-free operation before advertising either platform.
6. Once both platforms truly pass, create a **new** version and (only then) a combined package named `xdbot-companion.geode`. Never overwrite the existing v1.2.5 release or claim that renaming the Android file adds Windows/iOS.

The existing Geode Index rejection for “vibecode” cannot be bypassed by packaging or platform labels. Human-led code ownership and review are required before any new Index submission.
