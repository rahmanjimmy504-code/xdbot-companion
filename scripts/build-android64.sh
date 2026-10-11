#!/usr/bin/env bash
set -euo pipefail
: "${GEODE_SDK:?Set GEODE_SDK to the Geode 5.10.1 SDK}"
: "${ANDROID_NDK_ROOT:?Set ANDROID_NDK_ROOT to Android NDK r28c}"
command -v geode >/dev/null || { echo 'Geode CLI required on PATH' >&2; exit 1; }
root="$(cd "$(dirname "$0")/.." && pwd)"
cmake -S "$root" -B "$root/build" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 \
  -DCMAKE_BUILD_TYPE=Release -DGEODE_TARGET_PLATFORM=Android64 \
  -DGEODE_CLI="$(command -v geode)" -DGEODE_DONT_INSTALL_MODS=ON \
  -DGEODE_DISABLE_PRECOMPILED_HEADERS=ON \
  -DCMAKE_CXX_FLAGS='-include atomic -include type_traits'
# Arc 1.5.9 uses std::atomic_ref, unavailable in NDK r28c's libc++.
# Replace it with equivalent builtins in the downloaded build-tree copy only.
python3 - "$root/build/_deps/arc-src/src/runtime/Runtime.cpp" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]); s=p.read_text()
s=s.replace('auto curTask = std::atomic_ref<TaskBase*>(data.currentTask);\n        curTask.store(task, ::release);','__atomic_store_n(&data.currentTask, task, __ATOMIC_RELEASE);')
s=s.replace('curTask.store(nullptr, ::release);','__atomic_store_n(&data.currentTask, static_cast<TaskBase*>(nullptr), __ATOMIC_RELEASE);')
s=s.replace('std::atomic_ref{worker.currentTask}.load(::acquire)','__atomic_load_n(&worker.currentTask, __ATOMIC_ACQUIRE)')
p.write_text(s)
PY
cmake --build "$root/build" --parallel "${BUILD_JOBS:-3}"
echo "$root/build/jimmy_1234ha.xdbot-companion.geode"
