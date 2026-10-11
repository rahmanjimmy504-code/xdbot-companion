#pragma once
#include <filesystem>
#include <string>
#include <vector>
// Internal platform-specific backend; validation happens in ShareMacros.cpp.
bool shareMacroFilesPlatform(std::vector<std::filesystem::path> const& files, std::string& error);
