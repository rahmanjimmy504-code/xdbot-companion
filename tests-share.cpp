#include "src/AndroidShare.hpp"
#include "src/SharePlatform.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;
static int nativeCalls = 0;
bool shareMacroFilesPlatform(std::vector<fs::path> const&, std::string&) {
    ++nativeCalls;
    return true; // Observe validation without invoking any OS sharing UI.
}
int main() {
    auto dir = fs::temp_directory_path() / "xdbot-companion-share-validation";
    fs::create_directories(dir);
    auto good = dir / "example.gdr";
    { std::ofstream out(good); out << "macro"; }
    std::string error;
    assert(!shareMacroFiles({}, error) && nativeCalls == 0);
    assert(shareMacroFiles({good}, error) && nativeCalls == 1);
    assert(!shareMacroFiles({dir}, error) && nativeCalls == 1);
    auto unrelated = dir / "private-notes.txt";
    { std::ofstream out(unrelated); out << "not a replay"; }
    assert(!shareMacroFiles({unrelated}, error) && nativeCalls == 1);
    auto empty = dir / "empty.gdr";
    { std::ofstream out(empty); }
    assert(!shareMacroFiles({empty}, error) && nativeCalls == 1);
    auto legacy = dir / "legacy.gdr.json";
    { std::ofstream out(legacy); out << "replay"; }
    assert(shareMacroFiles({good, legacy}, error) && nativeCalls == 2);
    std::vector<fs::path> tooMany(31, good);
    assert(!shareMacroFiles(tooMany, error) && nativeCalls == 2);
    auto alias = dir / "alias.gdr";
    std::error_code ec;
    fs::create_symlink(good, alias, ec);
    if (!ec) assert(!shareMacroFiles({alias}, error) && nativeCalls == 2);
    fs::resize_file(good, 65ull * 1024 * 1024, ec); // oversized, sparse where supported
    if (!ec) assert(!shareMacroFiles({good}, error) && nativeCalls == 2);
    fs::remove_all(dir, ec);
    std::cout << "Share validation tests passed\n";
}
