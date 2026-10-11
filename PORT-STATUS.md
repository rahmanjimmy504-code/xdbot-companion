# Platform status (development branch)

| Platform | Compiles here? | Shares one and two macros to OS? | Can advertise support? |
|---|---|---|---|
| Android32 | Awaiting GitHub CI on beta.2; no device test | **Unverified** | **No** |
| Android64 | Compiled in beta.1 CI; no complete chooser device test | JNI unchanged in principle; end-to-end chooser unconfirmed | v1.2.5 remains separate Android64 release |
| Windows | Share UI candidate compiled in GitHub Actions; no device test | **Unverified** | **No** |
| iOS | Compiled in GitHub Actions; no device test | UIKit prototype, **unverified** | **No** |

No Linux or macOS port is planned. An iOS build uses a Mac as a *tool*, not as a macOS release target.

**Blockers:** Android32 compilation and native one/two-file chooser test, four-platform same-commit combined build, Windows native one/two-file transfer testing, iOS device testing, cross-platform xdBot UI/macro-folder verification, end-to-end Android64 sharing test, and human review after the prior Index rejection. Compiling is only the first gate.
