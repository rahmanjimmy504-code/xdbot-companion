#include "src/MacroExport.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
namespace fs = std::filesystem;
int main() {
    auto root = fs::temp_directory_path() / "xdbot-companion-export-test";
    fs::remove_all(root);
    fs::create_directories(root);
    std::string err;
    auto src = root / "my macro.gdr";
    auto dst = root / "copy.gdr";
    { std::ofstream o(src); o << "macro bytes"; }
    assert(validateMacroForExport(src, err));
    assert(copyMacroForExport(src, dst, err));
    std::string content; { std::ifstream i(dst); std::getline(i, content); }
    assert(content == "macro bytes");
    assert(!copyMacroForExport(src, src, err));
    assert(!copyMacroForExport(root / "absent.gdr", dst, err));
    { std::ofstream o(root / "empty.gdr"); }
    assert(!validateMacroForExport(root / "empty.gdr", err));
    { std::ofstream o(root / "old.cml"); o << "old local macro"; }
    assert(validateMacroForExport(root / "old.cml", err));
    assert(copyMacroForExport(root / "old.cml", root / "saved.cml", err));
    { std::ofstream o(root / "new.gdr2"); o << "new local macro"; }
    assert(copyMacroForExport(root / "new.gdr2", root / "saved.gdr2", err));
    { std::ofstream o(root / "legacy.gdr.json"); o << "legacy macro"; }
    assert(copyMacroForExport(root / "legacy.gdr.json", root / "saved.gdr.json", err));
    fs::create_symlink(src, root / "shortcut.gdr");
    assert(!validateMacroForExport(root / "shortcut.gdr", err));
    assert(!copyMacroForExport(src, root / "shortcut.gdr", err));
    fs::remove_all(root);
}
