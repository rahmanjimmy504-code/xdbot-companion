# Publishing checklist

This repository is the independent **xdBot Companion** source for Android64.
The mod metadata identifies developer **Jimmy_1234ha** and ID
`jimmy_1234ha.xdbot-companion`.

## Before making the repository public

- Select and add your own software licence (for example MIT or GPL-3.0-or-later)
  and confirm the source and icon may be distributed under it. No licence is
  included here because that is the owner's decision.
- Have a developer review the code and Android JNI bridge. The source was
  developed with AI assistance, and Geode Index guidelines discourage
  mostly AI-generated code whose author cannot maintain it.
- Test v1.2.5 on a phone: open Load Macro, inspect button layout, share one
  old macro and two selected macros, and verify a receiving app receives them.
  The share chooser has not yet been confirmed on device.
- Add clear credit and, if Index moderators regard this independent companion
  as based on xdBot, discuss any required approval with its creator first.
  Do not add xdBot's binary, textures, or other unlicensed assets.
- Decide whether to list `zilko.xdbot` as an optional/recommended dependency
  in mod metadata; the companion is only useful alongside official xdBot.
- Check that old `arena.xdbot-companion` is uninstalled when testing; a changed
  ID means both editions can otherwise load at once.

## Create and fill a GitHub repository

1. Create a new **empty** repository named `xdbot-companion` under your GitHub
   account. Do not ask GitHub to initialise it with a README or licence if
   you plan to push this folder.
2. Copy these files into that repo. With Git installed on your computer:

       git init -b main
       git add .
       git commit -m "Initial xdBot Companion v1.2.5 source"
       git remote add origin https://github.com/YOUR_USERNAME/xdbot-companion.git
       git push -u origin main

   Replace `YOUR_USERNAME` with your GitHub account name; GitHub will require
   you to authenticate. Alternatively use GitHub's Upload files interface.
3. After phone testing, create a *new* v1.2.5 GitHub Release and attach
   `jimmy_1234ha.xdbot-companion-v1.2.5-android64.geode` from the separate
   release download, not the source directory. Do not overwrite a published
   Geode package, as its hash can be checked by the Index.
4. Submit the Release asset's direct download link on the Geode Index once
   you meet its current publishing requirements.

`src/`, `scripts/`, and `logo.png` contain independent companion material.
This repository does not contain the upstream xdBot package.
