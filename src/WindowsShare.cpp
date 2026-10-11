#include "SharePlatform.hpp"

// Developer preview: Windows desktop Share UI. Windows 10/11 device testing,
// including transfer of two real files to a receiving application, is required.
// No private files are sent until the user taps Share and chooses a target.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shobjidl_core.h>
#include <winrt/base.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <filesystem>
#include <string>
#include <vector>

#pragma comment(lib, "windowsapp.lib")

namespace dt = winrt::Windows::ApplicationModel::DataTransfer;
namespace storage = winrt::Windows::Storage;
namespace collections = winrt::Windows::Foundation::Collections;

namespace {
// Keep the subscription alive until the next user-initiated share (and detach
// the old subscription before replacing it). The game calls this on its UI thread.
dt::DataTransferManager g_manager{nullptr};
winrt::event_token g_subscription{};

winrt::fire_and_forget fillShareRequest(
    dt::DataRequestedEventArgs args, std::vector<std::wstring> paths
) {
    auto request = args.Request();
    auto deferral = request.GetDeferral();
    try {
        std::vector<storage::IStorageItem> items;
        items.reserve(paths.size());
        for (auto const& path : paths) {
            storage::StorageFile file = co_await storage::StorageFile::GetFileFromPathAsync(path);
            items.emplace_back(file);
        }
        auto data = request.Data();
        data.Properties().Title(L"Share xdBot macros");
        data.RequestedOperation(dt::DataPackageOperation::Copy);
        data.SetStorageItems(winrt::single_threaded_vector<storage::IStorageItem>(std::move(items)));
    } catch (...) {
        // The OS may refuse a deleted/inaccessible file after validation.
        try { request.FailWithDisplayText(L"A selected macro could not be shared. Reopen the list."); }
        catch (...) {}
    }
    try { deferral.Complete(); }
    catch (...) {}
}

HWND gameWindow() {
    HWND window = GetActiveWindow();
    if (!window) window = GetForegroundWindow();
    if (!window) return nullptr;
    DWORD process = 0;
    GetWindowThreadProcessId(window, &process);
    return process == GetCurrentProcessId() ? window : nullptr;
}
} // namespace

bool shareMacroFilesPlatform(std::vector<std::filesystem::path> const& files, std::string& error) {
    auto window = gameWindow();
    if (!window) {
        error = "The Geometry Dash window is not active; cannot open Windows Share.";
        return false;
    }
    try {
        // Game code may already have initialized COM in a different apartment.
        static thread_local bool initialized = false;
        if (!initialized) {
            try { winrt::init_apartment(winrt::apartment_type::single_threaded); }
            catch (winrt::hresult_error const& ex) {
                if (ex.code() != RPC_E_CHANGED_MODE) throw;
            }
            initialized = true;
        }
        auto interop = winrt::get_activation_factory<dt::DataTransferManager, IDataTransferManagerInterop>();
        dt::DataTransferManager manager{nullptr};
        winrt::guid const iid{0xa5caee9b, 0x8708, 0x49d1, {0x8d, 0x36, 0x67, 0xd2, 0x5a, 0x8d, 0xa0, 0x0c}};
        winrt::check_hresult(interop->GetForWindow(window, iid, winrt::put_abi(manager)));
        if (g_manager) g_manager.DataRequested(g_subscription);
        g_manager = manager;
        std::vector<std::wstring> paths;
        paths.reserve(files.size());
        for (auto const& path : files) paths.push_back(path.wstring());
        g_subscription = manager.DataRequested([paths = std::move(paths)](
            dt::DataTransferManager const&, dt::DataRequestedEventArgs const& args
        ) { fillShareRequest(args, paths); });
        winrt::check_hresult(interop->ShowShareUIForWindow(window));
        return true; // Opened the OS UI, not proof that the recipient accepted.
    } catch (winrt::hresult_error const& ex) {
        error = "Windows Share could not be opened (HRESULT " +
            std::to_string(static_cast<unsigned long>(ex.code().value)) + ").";
        return false;
    } catch (...) {
        error = "Windows Share could not be opened. Please send a redacted log.";
        return false;
    }
}
