#include "MacroScanner.hpp"
#include <algorithm>
#include <cctype>
#include <set>
#include <vector>

namespace fs = std::filesystem;

bool MacroScanner::isMacroName(std::string name) {
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
    auto ends = [&](char const* ext) {
        auto suffix = std::string(ext);
        return name.size() > suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    // The official v2.7.7 Android binary contains these macro extensions.
    // Ordinary .json files are not shared; only the explicit legacy .gdr.json.
    return ends(".cml") || ends(".gdr") || ends(".gdr2") || ends(".gdr.json");
}

void MacroScanner::reset() {
    m_folder.clear();
    m_known.clear();
    m_latest.reset();
    m_baselined = false;
}

std::optional<fs::path> MacroScanner::scan(fs::path const& directory) {
    if (directory.empty() || directory != m_folder) {
        reset();
        m_folder = directory;
    }
    std::error_code ec;
    if (directory.empty() || !fs::is_directory(directory, ec) || ec) {
        // If the directory is created later, the first scan will establish a baseline.
        reset();
        return {};
    }
    std::set<fs::path> seen;
    std::vector<std::pair<fs::file_time_type, fs::path>> ready;
    std::optional<std::pair<fs::file_time_type, fs::path>> newest;
    fs::directory_iterator iter(directory, fs::directory_options::skip_permission_denied, ec);
    if (ec) return {};
    std::size_t scanned = 0;
    for (; iter != fs::directory_iterator{}; iter.increment(ec)) {
        if (ec) { reset(); return {}; }
        auto const& entry = *iter;
        // Bound the work per poll. Do not partially baseline an oversized folder.
        if (++scanned > 2000) { reset(); return {}; }
        auto path = entry.path();
        if (!isMacroName(path.filename().string())) continue;
        ec.clear();
        if (entry.is_symlink(ec) || ec || !entry.is_regular_file(ec) || ec) continue;
        auto size = entry.file_size(ec);
        if (ec || size == 0 || size > 64ull * 1024 * 1024) continue;
        auto modified = entry.last_write_time(ec);
        if (ec) continue;
        seen.insert(path);
        if (!newest || modified > newest->first) newest = {modified, path};
        auto [it, inserted] = m_known.try_emplace(path, Stamp{modified, size, m_baselined});
        if (inserted) continue;
        auto& stamp = it->second;
        if (stamp.modified != modified || stamp.size != size) {
            stamp = {modified, size, m_baselined};
        } else if (stamp.pending && m_baselined) {
            ready.emplace_back(modified, path);
            stamp.pending = false;
        }
    }
    if (ec) { reset(); return {}; }
    for (auto it = m_known.begin(); it != m_known.end();) {
        if (!seen.contains(it->first)) it = m_known.erase(it);
        else ++it;
    }
    m_latest = newest ? std::optional<fs::path>(newest->second) : std::nullopt;
    m_baselined = true;
    if (ready.empty()) return {};
    return std::max_element(ready.begin(), ready.end())->second;
}
