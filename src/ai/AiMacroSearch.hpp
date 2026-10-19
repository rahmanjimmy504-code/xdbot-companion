#pragma once
#include <cstdint>
#include <optional>
#include <vector>

// Experimental, platform-independent search core. It does not inspect Geometry
// Dash, drive input, or write an xdBot macro. Its caller must provide trustworthy
// attempt outcomes; a real Geode/Android bridge has not been built or tested.
namespace ai_macro {
struct InputEvent {
    std::uint32_t frame;
    std::uint8_t button; // 1 = jump in the intended GD input adapter
    bool down;
    bool operator==(InputEvent const&) const = default;
};
struct TrialResult {
    enum class Kind { Death, Completed, Interrupted } kind;
    std::uint32_t frame;
};
enum class Phase { Ready, Attempting, VerifyReady, Verifying, Verified, Stopped };

class Search {
public:
    explicit Search(std::uint32_t maxAttempts = 80, std::uint32_t maxFrames = 240 * 600);
    Phase phase() const { return m_phase; }
    std::uint32_t attempts() const { return m_attempts; }
    bool beginAttempt();
    std::vector<InputEvent> plannedEvents() const;
    void finishAttempt(TrialResult result);
    bool beginVerification();
    void finishVerification(TrialResult result);
    std::optional<std::vector<InputEvent>> verifiedEvents() const;
    void stop();
private:
    struct Pulse { std::uint32_t frame; std::uint8_t hold; };
    Phase m_phase = Phase::Ready;
    std::uint32_t m_attempts = 0;
    std::uint32_t m_maxAttempts;
    std::uint32_t m_maxFrames;
    std::uint32_t m_previousDeath = 0;
    std::uint32_t m_sameDeath = 0;
    std::vector<Pulse> m_pulses;
    std::vector<InputEvent> m_candidate;
    std::vector<InputEvent> m_verified;
    bool adapt(std::uint32_t deathFrame);
};
} // namespace ai_macro
