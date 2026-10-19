#include "src/ai/AiMacroSearch.hpp"
#include <cassert>
#include <iostream>
#include <vector>
using namespace ai_macro;

static TrialResult toyLevel(std::vector<InputEvent> const& events) {
    // Toy model only: two hazards, each escaped by an exactly timed pulse.
    for (auto hazard : {24u, 60u}) {
        bool escaped = false;
        for (std::size_t i = 0; i + 1 < events.size(); i += 2)
            if (events[i] == InputEvent{hazard - 6, 1, true} &&
                events[i + 1] == InputEvent{hazard - 4, 1, false}) escaped = true;
        if (!escaped) return {TrialResult::Kind::Death, hazard};
    }
    return {TrialResult::Kind::Completed, 100};
}
int main() {
    Search search(10, 120);
    while (search.phase() == Phase::Ready) {
        assert(search.beginAttempt());
        search.finishAttempt(toyLevel(search.plannedEvents()));
    }
    assert(search.phase() == Phase::VerifyReady);
    assert(search.attempts() == 3);
    assert(!search.verifiedEvents());
    auto candidate = search.plannedEvents();
    assert(candidate.size() == 4);
    assert(search.beginVerification());
    search.finishVerification(toyLevel(candidate));
    assert(search.phase() == Phase::Verified);
    assert(search.verifiedEvents() == candidate);
    assert(!search.beginAttempt());

    Search unverified(3, 120);
    assert(unverified.beginAttempt());
    unverified.finishAttempt({TrialResult::Kind::Death, 24});
    assert(unverified.beginAttempt());
    unverified.finishAttempt({TrialResult::Kind::Completed, 100});
    assert(!unverified.verifiedEvents());
    assert(unverified.beginVerification());
    unverified.finishVerification({TrialResult::Kind::Death, 31});
    assert(unverified.phase() == Phase::Stopped && !unverified.verifiedEvents());

    Search noProgress(2, 120);
    assert(noProgress.beginAttempt());
    noProgress.finishAttempt({TrialResult::Kind::Death, 24});
    assert(noProgress.beginAttempt());
    noProgress.finishAttempt({TrialResult::Kind::Death, 24});
    assert(noProgress.phase() == Phase::Stopped && !noProgress.verifiedEvents());

    Search interrupted;
    assert(interrupted.beginAttempt());
    interrupted.finishAttempt({TrialResult::Kind::Interrupted, 2});
    assert(interrupted.phase() == Phase::Stopped && !interrupted.verifiedEvents());

    Search impossible(0, 120);
    assert(!impossible.beginAttempt());
    Search immediate;
    assert(immediate.beginAttempt());
    immediate.finishAttempt({TrialResult::Kind::Death, 3});
    assert(immediate.phase() == Phase::Stopped);
    std::cout << "Experimental AI search-core tests passed (simulated hazards only)\n";
}
