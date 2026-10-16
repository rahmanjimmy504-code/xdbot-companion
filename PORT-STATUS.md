# Hyperbolus status — Android-first development branch

| Platform | Hyperbolus browser | User-tested? |
|---|---|---|
| Android64 | Beta.5 CI passed but failed phone picker test. Beta.6 URI-based picker/export source and dex built locally; beta.6 CI pending | Beta.5 failure; beta.6 not tested |
| Android32 | Same Android source expected to compile later; no separate build | No |
| Windows x64 | Embedded WebView2 backend not implemented | No |
| iOS arm64 | Embedded WKWebView backend not implemented | No |

No platform has verified website login, manual file upload or account download yet. The combined four-platform beta.4 and older beta.3 do not contain Hyperbolus. Android website form and SAF stream path are actual code in this branch, not a claim of device functionality. Only Android64 is in the current test workflow. No native Linux/macOS target.
