#pragma once
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>

// Filesystem-only logic; no xdBot internals or gameplay hooks. A newly saved
// file must be present and unchanged in two consecutive scans before notifying.
class MacroScanner {
public:
    void reset();
    std::optional<std::filesystem::path> scan(std::filesystem::path const& directory);
    std::optional<std::filesystem::path> latest() const { return m_latest; }
    static bool isMacroName(std::string name);
private:
    struct Stamp {
        std::filesystem::file_time_type modified;
        std::uintmax_t size;
        bool pending;
    };
    std::filesystem::path m_folder;
    std::map<std::filesystem::path, Stamp> m_known;
    std::optional<std::filesystem::path> m_latest;
    bool m_baselined = false;
};
