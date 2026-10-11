# Platform status (development branch)

| Platform | Compiles here? | Shares one and two macros to OS? | Can advertise support? |
|---|---|---|---|
| Android64 | Yes (source build only) | JNI unchanged in principle; end-to-end chooser unconfirmed | v1.2.5 remains Android-only testing release |
| Windows | Not built/tested; GitHub Actions workflow prepared | **No. Explicit not-implemented backend** | **No** |
| iOS | Not built/tested; macOS runner workflow prepared | UIKit prototype, **unverified** | **No** |

No Linux or macOS port is planned. An iOS build uses a Mac as a *tool*, not as a macOS release target.

**Blockers:** Windows native Share Sheet implementation, iOS compile and device testing, cross-platform xdBot UI/macro-folder verification, end-to-end Android sharing test, and human review after the prior Index rejection. Compiling is only the first gate.
