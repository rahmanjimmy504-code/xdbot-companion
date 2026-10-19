#pragma once
#include "AiMacroSearch.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ai_macro {
struct ReplayMetadata {
    std::uint32_t levelId = 0;
    std::string levelName;
    std::uint32_t completedFrame = 0;
    std::uint32_t tickRate = 240;
};

// Independent legacy GDR JSON compatibility candidate. Never call it a
// verified xdBot macro until a real xdBot 2.7.7 import/replay succeeds.
std::optional<std::string> makeGdrJson(ReplayMetadata const& metadata,
    std::vector<InputEvent> const& verifiedEvents);
}
