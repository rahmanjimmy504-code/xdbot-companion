# HYPE + GD Browser status — Android-first development branch

| Platform | In-game website source | Phone/device result |
|---|---|---|
| Android64 | Beta.9 adds icons and a separate GD Browser website view beside the existing Hyperbolus view; native CI pending | Beta.8 Hyperbolus window, keyboard typing and local EXPORT worked on the user's Fire tablet. Beta.9 GDB and UI untested. |
| Android32 | Website backends not separately built | No test |
| Windows x64 | Embedded WebView2 backend not implemented | No test |
| iOS arm64 | Embedded WKWebView backend not implemented | No test |

Hyperbolus account login and account download have not been fully verified. The user encountered a 400 submitting a replay for level 15031583; the public Hyperbolus level page for that ID independently returned `Level could not be fetched`. This does not validate attachment or imply a companion upload failure. Do not use an unrelated level ID as a workaround. Beta.9 is not a public release. Linux/macOS are out of scope.
