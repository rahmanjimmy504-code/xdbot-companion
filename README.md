# xdBot Companion — Hyperbolus native Android WebView source candidate

**NOT A RELEASE OR VERIFIED INSTALLABLE MOD.** This separate `hyperbolus-android` development branch starts an actual Android in-game website implementation. It is intentionally Android64-only for the first test; Android32, Windows x64 and iOS arm64 are final targets but do not have this browser yet. Keep the existing four-platform beta.4 package separate. No Hyperbolus password belongs in chat or mod-owned native fields.

## Intended flow (requires CI and a phone test)

- From xdBot Companion's macro popup, tap **HYPE**. With no macro selected, a native Android WebView Dialog appears over Geometry Dash with LOGIN / REPLAYS / UPLOAD navigation. Hyperbolus's own website hosts the email/password form. The mod does not receive or store the password.
- With **one selected `.gdr` or `.gdr.json`**, tapping HYPE offers that macro to the website's file input only after another user confirmation. Hyperbolus's own upload page still requires the user to choose a level and press its Upload button. Selecting a file does **not** upload it automatically. `.cml` and `.gdr2` show an explicit unsupported error. With no preselection, Android's document picker can choose one `.gdr`/`.json` file at the site's file input.
- Hyperbolus replay downloads use Android's **ACTION_CREATE_DOCUMENT** SAF picker and write through `ContentResolver.openOutputStream(uri)` rather than guessing `/storage/emulated/0/Download`. The user chooses the location. The site currently redirects downloads to expiring external storage; the download code follows HTTPS redirects without forwarding Hyperbolus cookies to external hosts. This must be tested against the live site on a real device.
- A local xdBot macro can also be exported separately in later platform work; the beta.4 Android EXPORT button is intentionally disabled due to the unsafe stock Geode SaveFile path mapping. The existing SHARE-to-Files route remains.

## Why this is not marked working yet

Java and policy host compilation pass with Android SDK 35; the DEX resource has been generated with D8 and tested structurally. Geode native C++ compilation, DEX packaging, Android launcher compatibility, actual Hyperbolus login, file attachment, form submission and completed SAF download have **not** passed CI/device tests. The Android WebView is app-owned and has its own cookies; it does not reuse an external Chrome session. Do not enter credentials until a reviewed build and explicit phone testing plan are ready. Do not release or distribute CI artifacts.

## Build route

The `.github/workflows/hyperbolus-android.yml` workflow triggers only on pushes to `hyperbolus-android` (or manually). It compiles the Android Java sources into `resources/hyperbolus-bridge.dex`, builds the Android64 Geode package and checks that both the Android64 native binary and dex are packaged. It never makes a public release. `scripts/build-hyperbolus-bridge.sh` requires a standard Android SDK platform and build-tools with `javac`/`d8`.
