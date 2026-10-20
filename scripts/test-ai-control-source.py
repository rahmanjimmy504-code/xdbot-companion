#!/usr/bin/env python3
"""Static regression checks, NOT a native compile or touch/input test."""
from pathlib import Path
s = Path('src/ai/AiGameplayAndroid.cpp').read_text()
entry = s.split('    void enter(PlayLayer* play) {', 1)[1].split('    bool startFor(', 1)[0]
assert 'startAttempt(' not in entry and 'beginVerification(' not in entry, 'Level entry auto-starts AI'
assert 'setSettingValue<bool>(kEnabled, false)' not in s, 'Current-run cancellation toggles preference'
assert 'void onAiControl(' in s and 'geode::createQuickPopup(verify ? "VERIFY AI" : "START AI"' in s
assert 'if (!confirmed' in s and 'g_ai.startFor(this)' in s, 'Missing explicit consent gate'
assert all(k in s for k in ['ai-start', 'ai-stop', 'ai-verify']), 'Visible control missing'
assert 'resettingForStart = true;' in s and 'play->resetLevel();' in s, 'Current-level reset gate missing'
assert 'm_isPracticeMode' in s and 'm_isPlatformer' in s and 'm_isDualMode' in s
print('AI explicit button/consent source regression checks passed; native and phone test pending')
