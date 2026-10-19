#include "AiMacroSearch.hpp"
#include <algorithm>

namespace ai_macro {
Search::Search(std::uint32_t maxAttempts, std::uint32_t maxFrames)
    : m_maxAttempts(std::min(maxAttempts, 1000u)),
      m_maxFrames(std::min(maxFrames, 240u * 3600u)) {
    if (!m_maxAttempts || m_maxFrames < 8) m_phase = Phase::Stopped;
}

bool Search::beginAttempt() {
    if (m_phase != Phase::Ready || m_attempts >= m_maxAttempts) return false;
    ++m_attempts;
    m_phase = Phase::Attempting;
    return true;
}

std::vector<InputEvent> Search::plannedEvents() const {
    if (m_phase == Phase::Stopped) return {};
    std::vector<InputEvent> events;
    events.reserve(m_pulses.size() * 2);
    for (auto const& pulse : m_pulses) {
        events.push_back({pulse.frame, 1, true});
        events.push_back({static_cast<std::uint32_t>(pulse.frame + pulse.hold), 1, false});
    }
    return events;
}

bool Search::adapt(std::uint32_t frame) {
    if (frame < 8 || frame >= m_maxFrames || m_pulses.size() >= 512) return false;
    bool same = m_previousDeath != 0 && frame <= m_previousDeath + 2 &&
        m_previousDeath <= frame + 2;
    if (same && !m_pulses.empty()) {
        ++m_sameDeath;
        // Move the last trial's input earlier in three-frame steps. A bounded
        // longer hold is considered after six misses; this is not level vision.
        std::uint32_t lead = 6 + 3 * (m_sameDeath % 7);
        auto next = frame > lead ? frame - lead : 1;
        auto& pulse = m_pulses.back();
        if (m_sameDeath > 6) pulse.hold = std::min<std::uint8_t>(8, pulse.hold + 2);
        if (m_pulses.size() > 1 && next <= m_pulses[m_pulses.size() - 2].frame +
            m_pulses[m_pulses.size() - 2].hold) return false;
        pulse.frame = next;
    } else {
        auto start = frame - 6;
        if (!m_pulses.empty() && start <= m_pulses.back().frame + m_pulses.back().hold)
            return false; // A new death cannot be safely isolated.
        m_pulses.push_back({start, 2});
        m_sameDeath = 0;
    }
    m_previousDeath = frame;
    return true;
}

void Search::finishAttempt(TrialResult result) {
    if (m_phase != Phase::Attempting) return;
    if (result.kind == TrialResult::Kind::Interrupted || result.frame > m_maxFrames) {
        stop(); return;
    }
    if (result.kind == TrialResult::Kind::Completed) {
        m_candidate = plannedEvents();
        // The successful attempt must have actually reached every planned
        // press/release before completion; never serialize a future input.
        m_phase = (m_candidate.empty() || m_candidate.back().frame > result.frame)
            ? Phase::Stopped : Phase::VerifyReady;
        return;
    }
    if (m_attempts >= m_maxAttempts || !adapt(result.frame)) { stop(); return; }
    m_phase = Phase::Ready;
}

bool Search::beginVerification() {
    if (m_phase != Phase::VerifyReady || m_candidate.empty()) return false;
    m_phase = Phase::Verifying;
    return true;
}

void Search::finishVerification(TrialResult result) {
    if (m_phase != Phase::Verifying) return;
    if (result.kind == TrialResult::Kind::Completed && result.frame <= m_maxFrames &&
        m_candidate == plannedEvents()) {
        m_verified = m_candidate;
        m_phase = Phase::Verified;
    } else stop();
}

std::optional<std::vector<InputEvent>> Search::verifiedEvents() const {
    if (m_phase != Phase::Verified) return std::nullopt;
    return m_verified;
}

void Search::stop() {
    m_phase = Phase::Stopped;
    m_candidate.clear();
    m_verified.clear();
}
} // namespace ai_macro
