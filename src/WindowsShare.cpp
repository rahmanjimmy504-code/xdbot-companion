#include "SharePlatform.hpp"

// Intentionally fail closed. Building a Win64 binary does not mean the mod
// provides Windows sharing. Windows Share Sheet integration must be developed
// and tested with IDataTransferManagerInterop and real storage-file transfers.
// A file picker or Explorer shortcut must not be misrepresented as sharing.
bool shareMacroFilesPlatform(std::vector<std::filesystem::path> const&, std::string& error) {
    error = "Windows sharing is not implemented in this development preview. "
            "Do not release this build as a functioning Windows port.";
    return false;
}
