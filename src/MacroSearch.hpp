#pragma once

#include <string>
#include <string_view>

// Match macro filenames only, without changing files or selections. ASCII case
// folding leaves UTF-8 byte sequences unchanged; names with non-ASCII letters
// can still be found by entering the same characters exactly.
namespace macro_search {
inline std::string lowercaseAscii(std::string_view text) {
    std::string result;
    result.reserve(text.size());
    for (unsigned char c : text) {
        result.push_back(static_cast<char>(c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c));
    }
    return result;
}
inline std::string normalizeQuery(std::string_view text) {
    auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    auto last = text.find_last_not_of(" \t\r\n");
    return lowercaseAscii(text.substr(first, last - first + 1));
}
inline bool filenameMatches(std::string_view filename, std::string_view normalizedQuery) {
    return normalizedQuery.empty() ||
        lowercaseAscii(filename).find(normalizedQuery) != std::string::npos;
}
} // namespace macro_search
