# Ready-to-send request to Hyperbolus maintainers (not an authorization)

**Subject:** Supported in-game macro upload integration for xdBot Companion?

Hello Hyperbolus team. I maintain a separate Geode add-on for xdBot, with builds for Android32/64, Windows x64 and iOS arm64. Users would like to sign in to their Hyperbolus account within the game (using the site's own email/password page, never a password field owned by our mod), explicitly confirm one or more selected macro files, and upload supported files to your Macro/Replay archive without leaving Geometry Dash.

Before developing anything that sends data to your service, could you confirm whether such an integration is permitted and supported? We found that your public upload form currently requires an authenticated, verified account, a `level_id`, and one file; the file-extension validator accepts `.gdr` and `.json`, but not xdBot's `.cml` or `.gdr2`. We will **not** relabel unsupported files, scrape credentials/cookies, bypass CSRF, or claim success without server confirmation.

If you support third-party clients, please provide the official upload API/authentication flow or integration documentation, expected metadata per file (level ID, format, notes, coins), size/rate limits, supported macro formats, and how you want errors and moderation outcomes handled. Is an in-game platform WebView showing only your own email/password login and upload form permitted? Could you support `.cml` and `.gdr2` natively, or document a safe conversion? Does Hyperbolus offer Google/Microsoft/GitHub account login at all? Our mod must never handle the user's site password or upload automatically in the background.

Project source: https://github.com/rahmanjimmy504-code/xdbot-companion (development branch `port-preview`). Thank you.
