#pragma once
#include <filesystem>
#include <string>
#include <vector>
// Launches Android ACTION_SEND or ACTION_SEND_MULTIPLE with read-granted
// content URIs. No network upload. The caller must be on the game's main thread.
bool shareMacroFiles(std::vector<std::filesystem::path> const& files, std::string& error);
inline bool shareMacroFile(std::filesystem::path const& file, std::string& error) {
    return shareMacroFiles({file}, error);
}
