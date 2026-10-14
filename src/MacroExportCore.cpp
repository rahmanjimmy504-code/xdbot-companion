#include "MacroExport.hpp"
#include "MacroScanner.hpp"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

bool validateMacroForExport(fs::path const& source, std::string& error) {
    std::error_code ec;
    if (source.empty() || !MacroScanner::isMacroName(source.filename().string()) ||
        fs::is_symlink(source, ec) || ec || !fs::is_regular_file(source, ec) || ec ||
        fs::file_size(source, ec) == 0 || ec ||
        fs::file_size(source, ec) > 64ull * 1024 * 1024 || ec) {
        error = "The selected macro is missing, inaccessible, empty or too large. Reopen the list.";
        return false;
    }
    return true;
}

bool copyMacroForExport(fs::path const& source, fs::path const& destination, std::string& error) {
    if (!validateMacroForExport(source, error)) return false;
    if (destination.empty() || destination.filename().empty()) {
        error = "Choose a destination other than the original macro (not a shortcut).";
        return false;
    }
    std::error_code ec;
    auto status = fs::symlink_status(destination, ec);
    if (ec && ec != std::errc::no_such_file_or_directory) {
        error = "The chosen destination could not be checked.";
        return false;
    }
    if (status.type() == fs::file_type::symlink) {
        error = "Choose a destination other than a shortcut.";
        return false;
    }
    ec.clear();
    if (status.type() != fs::file_type::not_found) {
        if (fs::equivalent(source, destination, ec) || ec) {
            error = "Choose a destination other than the original macro.";
            return false;
        }
    }
    if (!fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec) || ec) {
        error = "The chosen destination could not be written. Choose another location.";
        return false;
    }
    return true;
}
