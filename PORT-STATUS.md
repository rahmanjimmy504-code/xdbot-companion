# Hyperbolus status — Android-first development branch

| Platform | Hyperbolus browser | User-tested? |
|---|---|---|
| Android64 | Native Android WebView Dialog source and dex built locally; native Geode CI pending | No |
| Android32 | Same Android source expected to compile later; no separate build | No |
| Windows x64 | Embedded WebView2 backend not implemented | No |
| iOS arm64 | Embedded WKWebView backend not implemented | No |

No platform has verified website login, manual file upload or account download yet. The combined four-platform beta.4 and older beta.3 do not contain Hyperbolus. Android website form and SAF stream path are actual code in this branch, not a claim of device functionality. Only Android64 is in the current test workflow. No native Linux/macOS target.
