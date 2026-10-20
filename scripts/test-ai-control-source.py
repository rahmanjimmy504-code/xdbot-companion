#!/usr/bin/env python3
"""Static regressions only; CI native compile and device UI/input still needed."""
from pathlib import Path
s = Path('src/ai/AiGameplayAndroid.cpp').read_text()
entry = s.split('    void enter(PlayLayer* play) {', 1)[1].split('    std::string startFor(', 1)[0]
start = s.split('    std::string startFor(PlayLayer* play) {', 1)[1].split('    void tick(', 1)[0]
pause = s.split('class $modify(CompanionAIPauseControl, PauseLayer) {', 1)[1].split('class $modify(CompanionAITick,', 1)[0]
assert '#include <Geode/modify/PauseLayer.hpp>' in s
assert 'startAttempt(' not in entry and 'beginVerification(' not in entry, 'Entering level auto-starts AI'
assert 'layer != play' not in start, 'Stale session pointer blocks re-start after cancellation'
assert 'if (PlayLayer::get() != play' in start and 'return "Practice mode' in start
assert 'std::string startFor' in s and 'auto error = g_ai.startFor(play);' in pause
assert 'setSettingValue<bool>(kEnabled, false)' not in s, 'Current-run cancellation toggles preference'
assert 'void customSetup()' in pause and 'void onResume(' in pause
assert 'g_ai.pendingStart = true;' in pause and 'g_ai.pendingStart = false;' in pause
assert 'PauseLayer::onResume(sender);' in pause and 'onAIButton(' in pause
assert 'getChildByID(kMenuID)' not in s, 'Old in-level overlay control returned'
assert 'void onRestart(' in pause and 'void onQuit(' in pause
print('AI pause-menu/explicit consent/stale-session source checks passed; device test pending')
