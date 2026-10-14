#include "MacroPicker.hpp"
#include "MacroScanner.hpp"
#include "MacroSearch.hpp"
#include "MacroExport.hpp"
#include "AndroidShare.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <algorithm>

using namespace geode::prelude;
namespace fs = std::filesystem;

namespace {
CCMenuItemSpriteExtra* button(char const* title, cocos2d::CCObject* owner, cocos2d::SEL_MenuHandler action, float scale = .44f) {
    auto label = cocos2d::CCLabelBMFont::create(title, "goldFont.fnt");
    label->setScale(scale);
    return CCMenuItemSpriteExtra::create(label, owner, action);
}
void alert(char const* title, std::string text) {
    geode::createQuickPopup(title, std::move(text), "OK", nullptr, [](auto*, bool) {});
}
std::string displayName(fs::path const& p) {
    auto name = p.filename().string();
    std::replace_if(name.begin(), name.end(), [](char ch) { return ch == '<' || ch == '>'; }, '_');
    if (name.size() > 37) name.resize(34), name += "...";
    return name;
}
} // namespace

bool MacroPicker::init(fs::path folder) {
    if (!Popup::init(360.f, 260.f, "GJ_square01.png")) return false;
    m_folder = std::move(folder);
    // A simple new panel echoing xdBot's documented default menu colour.
    auto bg = cocos2d::CCLayerColor::create(cocos2d::ccc4(51, 68, 153, 236), 348.f, 248.f);
    bg->setPosition({6.f, 6.f});
    m_mainLayer->addChild(bg, 1);
    this->setTitle("SHARE XDBOT MACROS", "goldFont.fnt", .55f, 17.f);
    m_status = cocos2d::CCLabelBMFont::create("", "chatFont.fnt");
    m_status->setScale(.45f);
    m_mainLayer->addChildAtPosition(m_status, Anchor::Bottom, {0.f, 49.f});
    m_status->setZOrder(5);
    auto* search = geode::TextInput::create(232.f, "Find saved macro...", "chatFont.fnt");
    if (!search) return false;
    search->setMaxCharCount(80);
    search->setCallback([this](std::string const& query) {
        m_query = macro_search::normalizeQuery(query);
        m_page = 0;
        applyFilter();
    });
    m_mainLayer->addChildAtPosition(search, Anchor::Top, {0.f, -31.f});
    auto prev = button("PREV", this, menu_selector(MacroPicker::onPrev));
    auto next = button("NEXT", this, menu_selector(MacroPicker::onNext));
    auto all = button("ALL", this, menu_selector(MacroPicker::onAll));
    auto clear = button("CLEAR", this, menu_selector(MacroPicker::onClear));
    auto save = button("EXPORT", this, menu_selector(MacroPicker::onExport), .39f);
    auto share = button("SHARE", this, menu_selector(MacroPicker::onShare));
    m_buttonMenu->addChildAtPosition(prev, Anchor::Bottom, {-148.f, 21.f});
    m_buttonMenu->addChildAtPosition(next, Anchor::Bottom, {-92.f, 21.f});
    m_buttonMenu->addChildAtPosition(all, Anchor::Bottom, {-37.f, 21.f});
    m_buttonMenu->addChildAtPosition(clear, Anchor::Bottom, {18.f, 21.f});
    m_buttonMenu->addChildAtPosition(save, Anchor::Bottom, {82.f, 21.f});
    m_buttonMenu->addChildAtPosition(share, Anchor::Bottom, {145.f, 21.f});
    reload();
    return true;
}

void MacroPicker::reload() {
    m_allFiles.clear();
    std::vector<std::pair<fs::file_time_type, fs::path>> candidates;
    std::error_code ec;
    fs::directory_iterator it(m_folder, fs::directory_options::skip_permission_denied, ec);
    if (!ec) {
        std::size_t scanned = 0;
        for (; it != fs::directory_iterator{}; it.increment(ec)) {
            if (ec || ++scanned > 5000) break;
            auto const& entry = *it;
            if (!MacroScanner::isMacroName(entry.path().filename().string())) continue;
            ec.clear();
            if (entry.is_symlink(ec) || ec || !entry.is_regular_file(ec) || ec) continue;
            auto modified = entry.last_write_time(ec);
            if (ec) continue;
            candidates.emplace_back(modified, entry.path());
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](auto const& a, auto const& b) {
        return a.first != b.first ? a.first > b.first : a.second.string() < b.second.string();
    });
    if (candidates.size() > 1000) candidates.resize(1000);
    for (auto& candidate : candidates) m_allFiles.push_back(std::move(candidate.second));
    applyFilter();
}

void MacroPicker::applyFilter() {
    m_files.clear();
    for (auto const& path : m_allFiles) {
        if (macro_search::filenameMatches(path.filename().string(), m_query)) m_files.push_back(path);
    }
    if (m_page * perPage >= m_files.size()) m_page = 0;
    render();
}

void MacroPicker::updateStatus() {
    auto pages = std::max<std::size_t>(1, (m_files.size() + perPage - 1) / perPage);
    m_status->setString(fmt::format("{} of {} found | {} chosen | {}/{}",
        m_files.size(), m_allFiles.size(), m_selected.size(), m_page + 1, pages).c_str());
}

void MacroPicker::render() {
    for (auto* row : m_rows) row->removeFromParent();
    m_rows.clear();
    for (std::size_t line = 0; line < perPage; ++line) {
        auto index = m_page * perPage + line;
        if (index >= m_files.size()) break;
        auto text = std::string(m_selected.contains(m_files[index]) ? "[X] " : "[ ] ") + displayName(m_files[index]);
        auto row = button(text.c_str(), this, menu_selector(MacroPicker::onToggle), .47f);
        row->setTag(static_cast<int>(index));
        m_buttonMenu->addChildAtPosition(row, Anchor::TopLeft, {18.f, -62.f - 20.f * line}, {0.f, .5f});
        m_rows.push_back(row);
    }
    updateStatus();
}
void MacroPicker::onToggle(cocos2d::CCObject* sender) {
    auto index = static_cast<std::size_t>(static_cast<cocos2d::CCNode*>(sender)->getTag());
    if (index >= m_files.size()) return;
    auto const& file = m_files[index];
    if (!m_selected.erase(file)) m_selected.insert(file);
    // Do not remove the button that is currently dispatching this callback.
    auto* item = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto* label = static_cast<cocos2d::CCLabelBMFont*>(item->getNormalImage());
    auto text = std::string(m_selected.contains(file) ? "[X] " : "[ ] ") + displayName(file);
    label->setString(text.c_str());
    updateStatus();
}
void MacroPicker::onNext(cocos2d::CCObject*) {
    if ((m_page + 1) * perPage < m_files.size()) ++m_page;
    render();
}
void MacroPicker::onPrev(cocos2d::CCObject*) {
    if (m_page) --m_page;
    render();
}
void MacroPicker::onAll(cocos2d::CCObject*) {
    for (std::size_t i = m_page * perPage; i < std::min(m_files.size(), (m_page + 1) * perPage); ++i)
        m_selected.insert(m_files[i]);
    render();
}
void MacroPicker::onClear(cocos2d::CCObject*) {
    m_selected.clear(); render();
}
void MacroPicker::onExport(cocos2d::CCObject*) {
    if (m_selected.size() != 1) {
        alert("Export one macro", "Choose exactly one macro, then select its save location.");
        return;
    }
    exportMacroFile(*m_selected.begin());
}
void MacroPicker::onShare(cocos2d::CCObject*) {
    if (m_selected.empty()) { alert("Nothing selected", "Choose at least one macro to share."); return; }
    if (m_selected.size() > 30) { alert("Too many files", "Select up to 30 macros at once."); return; }
    std::vector<fs::path> paths(m_selected.begin(), m_selected.end());
    std::string error;
    if (!shareMacroFiles(paths, error)) alert("Sharing unavailable", error);
}
MacroPicker* MacroPicker::create(fs::path folder) {
    auto* ret = new MacroPicker;
    if (ret->init(std::move(folder))) { ret->autorelease(); return ret; }
    delete ret; return nullptr;
}
