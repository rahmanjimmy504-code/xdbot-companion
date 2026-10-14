#pragma once
#include <filesystem>
#include <optional>
#include <string>

// Launches a native Android WebView Dialog over the game. A selected .gdr or
// .json is offered to the site's file input only after a user confirmation;
// submission happens only on Hyperbolus's own form. Empty = browse/login.
bool openHyperbolusAndroid(std::optional<std::filesystem::path> selected, std::string& error);
