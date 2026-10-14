#include "MacroExport.hpp"
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
    // The stock Geode Android SaveFile API discards the SAF content URI and
    // returns a guessed filesystem path. That path may be unwritable or even
    // point at a different Downloads file. Never call an unverified copy a
    // successful save. The existing SHARE action lets a Files app receive the
    // original bytes using a content URI; the user chooses the receiving app.
    notice("Android export unavailable",
        "This launcher's save picker cannot safely write the selected location. Use SHARE and choose a Files app to save your macro.");
    return;
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
