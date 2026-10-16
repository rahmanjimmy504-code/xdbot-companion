#include "MacroExport.hpp"
#ifdef GEODE_IS_ANDROID
#include "HyperbolusAndroid.hpp"
#endif
#include <Geode/Geode.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>
#include <filesystem>
#include <optional>
#include <string>

namespace fs = std::filesystem;
namespace file = geode::utils::file;

namespace {
void notice(std::string const& title, std::string const& text) {
    geode::createQuickPopup(title.c_str(), text, "OK", nullptr, [](auto*, bool) {});
}
}

void exportMacroFile(fs::path const& source) {
    std::string error;
    if (!validateMacroForExport(source, error)) {
        notice("Export unavailable", error);
        return;
    }
#ifdef GEODE_IS_ANDROID
    // Use our own ACTION_CREATE_DOCUMENT framework Fragment result receiver.
    // The launcher drops the SAF URI; copying to its guessed path is unsafe.
    if (!exportMacroAndroid(source, error)) notice("Export unavailable", error);
#else
    file::FilePickOptions options;
    options.defaultPath = source.filename();
    geode::async::spawn(file::pick(file::PickMode::SaveFile, options),
        [source](geode::Result<std::optional<fs::path>> result) {
            if (result.isErr()) {
                notice("Export unavailable", "The save dialog could not be completed on this device.");
                return;
            }
            auto destination = std::move(result).unwrap();
            if (!destination) return; // cancellation is not a copy or upload
            std::string error;
            if (copyMacroForExport(source, *destination, error))
                notice("Macro exported", "A copy was saved to your chosen location.");
            else
                notice("Export unavailable", error);
        }
    );
#endif
}
