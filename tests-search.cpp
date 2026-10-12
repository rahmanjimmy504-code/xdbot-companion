#include "src/MacroSearch.hpp"
#include <cassert>
#include <iostream>

int main() {
    using macro_search::filenameMatches;
    using macro_search::normalizeQuery;
    assert(normalizeQuery("  ToN 618 \t") == "ton 618");
    assert(normalizeQuery(" \r\n\t ").empty());
    assert(filenameMatches("TON 618.GDR", normalizeQuery("ton 618")));
    assert(filenameMatches("old_macro.cml", normalizeQuery(".CML")));
    assert(!filenameMatches("old_macro.cml", normalizeQuery("new")));
    assert(filenameMatches("x.gdr2", normalizeQuery("   ")));
    assert(filenameMatches("Mañana.gdr", normalizeQuery("ñ")));
    assert(!filenameMatches("Mañana.gdr", normalizeQuery("zzz")));
    std::cout << "MacroSearch tests passed\n";
}
