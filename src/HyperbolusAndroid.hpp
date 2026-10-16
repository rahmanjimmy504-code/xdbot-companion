#pragma once
#include <filesystem>
#include <optional>
#include <string>

// Launches a native Android WebView Dialog over the game. A selected .gdr or
// .json is offered to the site's file input only after a user confirmation;
// submission happens only on Hyperbolus's own form. Empty = browse/login.
bool openHyperbolusAndroid(std::optional<std::filesystem::path> selected, std::string& error);
// A separate in-game WebView for gdbrowser.com, without a selected macro.
bool openGDBrowserAndroid(std::string& error);
// Uses ACTION_CREATE_DOCUMENT and ContentResolver for an exact chosen URI;
// unlike Geode's Android SaveFile path heuristic this never guesses Downloads.
bool exportMacroAndroid(std::filesystem::path const& source, std::string& error);
