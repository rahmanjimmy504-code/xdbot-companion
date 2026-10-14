#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
sdk="${ANDROID_HOME:-${ANDROID_SDK_ROOT:-/usr/local/lib/android/sdk}}"
platform="$(find "$sdk/platforms" -mindepth 1 -maxdepth 1 -type d -name 'android-*' | sort -V | tail -1)"
build_tools="$(find "$sdk/build-tools" -mindepth 1 -maxdepth 1 -type d | sort -V | tail -1)"
: "${platform:?Missing Android SDK platform}"
: "${build_tools:?Missing Android build tools}"
test -f "$platform/android.jar"
test -x "$build_tools/d8"
command -v javac >/dev/null
work="$root/.java-build"
mkdir -p "$work/classes" "$work/dex" "$root/resources"
javac -encoding UTF-8 --release 8 -classpath "$platform/android.jar" -d "$work/classes" \
  "$root/android/java/com/jimmy1234ha/xdbot/HyperbolusRules.java" \
  "$root/android/java/com/jimmy1234ha/xdbot/HyperbolusBridge.java"
mapfile -d '' classes < <(find "$work/classes" -name '*.class' -print0)
test "${#classes[@]}" -gt 0
"$build_tools/d8" --lib "$platform/android.jar" --min-api 23 --output "$work/dex" "${classes[@]}"
test -s "$work/dex/classes.dex"
cp "$work/dex/classes.dex" "$root/resources/hyperbolus-bridge.dex"
ls -lh "$root/resources/hyperbolus-bridge.dex"
