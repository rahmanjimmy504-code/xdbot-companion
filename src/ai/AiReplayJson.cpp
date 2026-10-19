#include "AiReplayJson.hpp"
#include <cstdio>
#include <sstream>

namespace ai_macro {
namespace {
std::string escape(std::string const& input) {
    std::string result;
    for (unsigned char ch : input) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 32) {
                    char code[7];
                    std::snprintf(code, sizeof(code), "\\u%04x", ch);
                    result += code;
                } else result += static_cast<char>(ch);
        }
    }
    return result;
}
}

std::optional<std::string> makeGdrJson(ReplayMetadata const& m,
    std::vector<InputEvent> const& events) {
    if (m.levelName.size() > 256 || !m.tickRate || m.tickRate > 1000 ||
        !m.completedFrame || m.completedFrame > 240u * 3600u ||
        events.empty() || events.size() > 100000 || events.size() % 2) return std::nullopt;
    std::uint32_t lastFrame = 0;
    bool held = false;
    for (auto const& event : events) {
        if (event.button != 1 || event.frame < lastFrame || event.frame > m.completedFrame ||
            event.down == held) return std::nullopt;
        held = event.down;
        lastFrame = event.frame;
    }
    if (held) return std::nullopt;

    std::ostringstream out;
    out << "{\"author\":\"Jimmy_1234ha\",\"description\":\"Experimental AI-generated replay; verify in xdBot before sharing\","
        << "\"duration\":" << (static_cast<double>(m.completedFrame) / m.tickRate)
        << ",\"gameVersion\":22081,\"version\":1,\"framerate\":" << m.tickRate
        << ",\"seed\":0,\"coins\":0,\"ldm\":false,"
        << "\"bot\":{\"name\":\"xdBot Companion AI\",\"version\":\"0.1-experimental\"},"
        << "\"level\":{\"id\":" << m.levelId << ",\"name\":\"" << escape(m.levelName) << "\"},"
        << "\"inputs\":[";
    for (std::size_t i = 0; i < events.size(); ++i) {
        auto const& e = events[i];
        if (i) out << ',';
        out << "{\"frame\":" << e.frame << ",\"btn\":1,\"2p\":false,\"down\":"
            << (e.down ? "true" : "false") << '}';
    }
    out << "],\"frameFixes\":[]}\n";
    return out.str();
}
} // namespace ai_macro
