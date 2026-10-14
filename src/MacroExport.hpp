#pragma once
#include <filesystem>
#include <string>

// Called only after a user selects exactly one local xdBot macro and taps
// EXPORT. Presents the OS save location chooser. Never accesses Hyperbolus.
void exportMacroFile(std::filesystem::path const& source);

// Non-UI checks used before presenting a chooser and again before copying.
bool validateMacroForExport(std::filesystem::path const& source, std::string& error);
bool copyMacroForExport(std::filesystem::path const& source,
    std::filesystem::path const& destination, std::string& error);
