#include "AndroidShare.hpp"
#include "SharePlatform.hpp"
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

bool shareMacroFiles(std::vector<fs::path> const& files, std::string& error) {
    error.clear();
    if (files.empty() || files.size() > 30) {
        error = "Select between 1 and 30 files.";
        return false;
    }
    for (auto const& path : files) {
        std::error_code ec;
        if (fs::is_symlink(path, ec) || ec || !fs::is_regular_file(path, ec) || ec ||
            fs::file_size(path, ec) > 64ull * 1024 * 1024 || ec) {
            error = "A selected macro is missing, inaccessible, or too large. Reopen the list.";
            return false;
        }
    }
    return shareMacroFilesPlatform(files, error);
}
