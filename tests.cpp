#include "src/MacroScanner.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
namespace fs = std::filesystem;
int main() {
    auto dir = fs::temp_directory_path() / ("macro-share-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(dir);
    auto write = [](fs::path const& p, std::string const& s) { std::ofstream(p, std::ios::binary | std::ios::trunc) << s; };
    write(dir / "existing.cml", "abc");
    MacroScanner m;
    assert(!m.scan(dir)); // Existing macros are never offered retroactively.
    assert(!m.scan(dir));
    assert(m.latest() == dir / "existing.cml");
    write(dir / "notes.txt", "a");
    write(dir / "notamacro.json", "a");
    assert(!m.scan(dir));
    write(dir / "new.cml", "chunk1");
    assert(!m.scan(dir));
    write(dir / "new.cml", "chunk1 chunk2");
    assert(!m.scan(dir)); // Changed again: do not share partially written file.
    assert(m.scan(dir) == dir / "new.cml");
    assert(!m.scan(dir));
    write(dir / "new.cml", "overwritten later");
    assert(!m.scan(dir));
    assert(m.scan(dir) == dir / "new.cml");
    write(dir / "legacy.gdr2", "test");
    assert(!m.scan(dir));
    assert(m.scan(dir) == dir / "legacy.gdr2");
    assert(MacroScanner::isMacroName("test.GDR.JSON"));
    assert(!MacroScanner::isMacroName("foo.json"));
    auto dir2 = dir / "other"; fs::create_directory(dir2);
    write(dir2 / "old.cml", "test");
    assert(!m.scan(dir2)); // Changing configured folder resets baseline.
    assert(!m.scan(dir2));
    fs::remove_all(dir);
    std::cout << "MacroScanner tests passed\n";
}
