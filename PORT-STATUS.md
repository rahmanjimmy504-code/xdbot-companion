# Platform status (development branch)

| Platform | Compiles here? | Shares one and two macros to OS? | Can advertise support? |
|---|---|---|---|
| Android32 | Compiled in beta.2 GitHub CI; no device test | **Unverified** | **No** |
| Android64 | Compiled in beta.1 CI; no complete chooser device test | JNI unchanged in principle; end-to-end chooser unconfirmed | v1.2.5 remains separate Android64 release |
| Windows | Share UI candidate compiled in GitHub Actions; no device test | **Unverified** | **No** |
| iOS | Compiled in GitHub Actions; no device test | UIKit prototype, **unverified** | **No** |

No Linux or macOS port is planned. An iOS build uses a Mac as a *tool*, not as a macOS release target.

**Blockers:** Android32 native one/two-file chooser test, beta.3 search device test, Windows native one/two-file transfer testing, iOS device testing, cross-platform xdBot UI/macro-folder verification, end-to-end Android64 sharing test, and human review after the prior Index rejection. Beta.4 one-file local EXPORT is source-only: all four host tests passed, but four-target compile and Windows/iOS device save checks are pending. Android SAF's write permission is tied to the chosen URI; Geode's existing SaveFile API loses this URI and guesses a path, so Android EXPORT intentionally does not save and instead advises using SHARE with a Files app. iOS uses the native document *export* picker rather than Geode's zero-byte SaveFile implementation. Compiling is only the first gate. Hyperbolus website sign-in/upload and account downloads are not implemented: the site does not offer a documented approved client account integration, and its form does not accept `.cml` / `.gdr2`.
