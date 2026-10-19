#include "AndroidShare.hpp"
#include "MacroExport.hpp"
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
    // Recheck every path immediately before the OS chooser: a file may have
    // changed or disappeared after the picker listed it. Apply the same name,
    // nonempty, regular-file, symlink and size rules as local export. This is
    // a metadata check, not a claim that the replay contents were parsed.
    for (auto const& path : files)
        if (!validateMacroForExport(path, error)) return false;
    return shareMacroFilesPlatform(files, error);
}
