#include "MacroPicker.hpp"
#include "MacroScanner.hpp"
#include "MacroSearch.hpp"
#include "MacroExport.hpp"
#ifdef GEODE_IS_ANDROID
#include "HyperbolusAndroid.hpp"
#endif
#include "AndroidShare.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <algorithm>
#include <cctype>

using namespace geode::prelude;
namespace fs = std::filesystem;

namespace {
constexpr int kSelectionIconTag = 7431;
std::string iconPath(char const* name) {
    return (geode::Mod::get()->getResourcesDir() / "icons" / (std::string(name) + ".png")).string();
}
cocos2d::CCSprite* icon(char const* name, float scale = .42f) {
    auto path = iconPath(name);
    auto* image = cocos2d::CCSprite::create(path.c_str());
    if (image) image->setScale(scale);
    return image;
}
CCMenuItemSpriteExtra* button(char const* title, cocos2d::CCObject* owner,
    cocos2d::SEL_MenuHandler action, float scale = .44f, char const* symbol = nullptr,
    bool above = false) {
    auto* label = cocos2d::CCLabelBMFont::create(title, "goldFont.fnt");
    label->setScale(scale);
    if (!symbol) return CCMenuItemSpriteExtra::create(label, owner, action);
    auto* image = icon(symbol, above ? .36f : .30f); // 96px generated sprites, ~35/29 game units
    if (!image) return CCMenuItemSpriteExtra::create(label, owner, action);
    float lw = label->getContentSize().width * scale;
    float lh = label->getContentSize().height * scale;
    float iw = image->getContentSize().width * image->getScale();
    float ih = image->getContentSize().height * image->getScale();
    auto* body = cocos2d::CCNode::create();
    body->ignoreAnchorPointForPosition(false);
    if (above) {
        body->setContentSize({std::max(lw, iw), lh + ih + 1.f});
        label->setPosition({body->getContentSize().width / 2.f, lh / 2.f});
        image->setPosition({body->getContentSize().width / 2.f, lh + ih / 2.f + 1.f});
    } else {
        body->setContentSize({lw + iw + 3.f, std::max(lh, ih)});
        label->setPosition({iw + 3.f + lw / 2.f, body->getContentSize().height / 2.f});
        image->setPosition({iw / 2.f, body->getContentSize().height / 2.f});
    }
    body->addChild(label);
    body->addChild(image);
    return CCMenuItemSpriteExtra::create(body, owner, action);
}
void alert(char const* title, std::string text) {
    geode::createQuickPopup(title, std::move(text), "OK", nullptr, [](auto*, bool) {});
}
std::string displayName(fs::path const& p) {
    auto name = p.filename().string();
    std::replace_if(name.begin(), name.end(), [](char ch) { return ch == '<' || ch == '>'; }, '_');
    if (name.size() > 25) {
        std::size_t cut = 22;
        while (cut && (static_cast<unsigned char>(name[cut]) & 0xc0) == 0x80) --cut;
        name.resize(cut);
        name += "...";
    }
    return name;
}
char const* formatIcon(fs::path const& path) {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension == ".json") return "json";
    if (extension == ".gdr2") return "gdr2";
    if (extension == ".cml") return "cml";
    return "gdr";
}
CCMenuItemSpriteExtra* rowButton(fs::path const& path, bool selected,
    cocos2d::CCObject* owner, cocos2d::SEL_MenuHandler action) {
    auto* body = cocos2d::CCNode::create();
    body->ignoreAnchorPointForPosition(false);
    body->setContentSize({314.f, 35.f});
    auto* selectedIcon = icon(selected ? "selected" : "unselected", .35f);
    if (selectedIcon) {
        selectedIcon->setPosition({18.f, 17.5f});
        selectedIcon->setTag(kSelectionIconTag);
        body->addChild(selectedIcon);
    }
    auto* fileIcon = icon(formatIcon(path), .35f);
    if (fileIcon) { fileIcon->setPosition({55.f, 17.5f}); body->addChild(fileIcon); }
    auto* label = cocos2d::CCLabelBMFont::create(displayName(path).c_str(), "goldFont.fnt");
    label->setScale(.42f);
    label->setAnchorPoint({0.f, .5f});
    label->setPosition({78.f, 17.5f});
    body->addChild(label);
    return CCMenuItemSpriteExtra::create(body, owner, action);
}
} // namespace

bool MacroPicker::init(fs::path folder) {
    if (!Popup::init(360.f, 288.f, "GJ_square01.png")) return false;
    m_folder = std::move(folder);
    // A simple new panel echoing xdBot's documented default menu colour.
    auto bg = cocos2d::CCLayerColor::create(cocos2d::ccc4(51, 68, 153, 236), 348.f, 276.f);
    bg->setPosition({6.f, 6.f});
    m_mainLayer->addChild(bg, 1);
    this->setTitle("SHARE XDBOT MACROS", "goldFont.fnt", .55f, 17.f);
    m_status = cocos2d::CCLabelBMFont::create("", "chatFont.fnt");
    m_status->setScale(.45f);
    m_mainLayer->addChildAtPosition(m_status, Anchor::Bottom, {9.f, 57.f});
    m_status->setZOrder(5);
    if (auto* statusIcon = icon("gdr", .21f)) {
        m_mainLayer->addChildAtPosition(statusIcon, Anchor::Bottom, {-144.f, 57.f});
        statusIcon->setZOrder(5);
    }
#ifdef GEODE_IS_ANDROID
    auto* search = geode::TextInput::create(174.f, "Find macro...", "chatFont.fnt");
#else
    auto* search = geode::TextInput::create(232.f, "Find saved macro...", "chatFont.fnt");
#endif
    if (!search) return false;
    search->setMaxCharCount(80);
    search->setCallback([this](std::string const& query) {
        m_query = macro_search::normalizeQuery(query);
        m_page = 0;
        applyFilter();
    });
#ifdef GEODE_IS_ANDROID
    m_mainLayer->addChildAtPosition(search, Anchor::Top, {-61.f, -31.f});
    auto hype = button("HYPE", this, menu_selector(MacroPicker::onHyperbolus), .40f, "hype");
    auto gdb = button("GDB", this, menu_selector(MacroPicker::onGDBrowser), .40f, "gdb");
    m_buttonMenu->addChildAtPosition(hype, Anchor::Top, {81.f, -31.f});
    m_buttonMenu->addChildAtPosition(gdb, Anchor::Top, {145.f, -31.f});
#else
    m_mainLayer->addChildAtPosition(search, Anchor::Top, {0.f, -31.f});
#endif
    auto prev = button("PREV", this, menu_selector(MacroPicker::onPrev), .44f, "prev", true);
    auto next = button("NEXT", this, menu_selector(MacroPicker::onNext), .44f, "next", true);
    auto all = button("ALL", this, menu_selector(MacroPicker::onAll), .44f, "all", true);
    auto clear = button("CLEAR", this, menu_selector(MacroPicker::onClear), .44f, "clear", true);
    auto save = button("EXPORT", this, menu_selector(MacroPicker::onExport), .39f, "export", true);
    auto share = button("SHARE", this, menu_selector(MacroPicker::onShare), .44f, "share", true);
    m_buttonMenu->addChildAtPosition(prev, Anchor::Bottom, {-148.f, 24.f});
    m_buttonMenu->addChildAtPosition(next, Anchor::Bottom, {-92.f, 24.f});
    m_buttonMenu->addChildAtPosition(all, Anchor::Bottom, {-37.f, 24.f});
    m_buttonMenu->addChildAtPosition(clear, Anchor::Bottom, {18.f, 24.f});
    m_buttonMenu->addChildAtPosition(save, Anchor::Bottom, {82.f, 24.f});
    m_buttonMenu->addChildAtPosition(share, Anchor::Bottom, {145.f, 24.f});
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
        auto row = rowButton(m_files[index], m_selected.contains(m_files[index]),
            this, menu_selector(MacroPicker::onToggle));
        row->setTag(static_cast<int>(index));
        m_buttonMenu->addChildAtPosition(row, Anchor::TopLeft, {18.f, -66.f - 36.f * line}, {0.f, .5f});
        m_rows.push_back(row);
    }
    updateStatus();
}
void MacroPicker::onToggle(cocos2d::CCObject* sender) {
    auto index = static_cast<std::size_t>(static_cast<cocos2d::CCNode*>(sender)->getTag());
    if (index >= m_files.size()) return;
    auto const& file = m_files[index];
    if (!m_selected.erase(file)) m_selected.insert(file);
    // Update our own icon in place. Never remove the dispatching row or scan
    // xdBot-owned children (the old RTTI scan caused a phone crash).
    auto* item = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto* body = item->getNormalImage();
    auto* image = body ? static_cast<cocos2d::CCSprite*>(body->getChildByTag(kSelectionIconTag)) : nullptr;
    if (image) {
        auto path = iconPath(m_selected.contains(file) ? "selected" : "unselected");
        auto* texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(path.c_str(), false);
        if (texture) image->setTexture(texture);
    }
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
void MacroPicker::onHyperbolus(cocos2d::CCObject*) {
#ifdef GEODE_IS_ANDROID
    if (m_selected.size() > 1) {
        alert("One macro at a time", "Hyperbolus's website accepts one replay per upload. Choose one, or clear selection to browse and download.");
        return;
    }
    std::optional<fs::path> selected;
    if (!m_selected.empty()) {
        selected = *m_selected.begin();
        auto extension = selected->extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        if (extension != ".gdr" && extension != ".json") {
            alert("Unsupported website format", "Hyperbolus accepts .gdr or .json. .cml and .gdr2 cannot be uploaded there. No file was sent.");
            return;
        }
    }
    std::string error;
    if (!openHyperbolusAndroid(selected, error)) alert("Hyperbolus unavailable", error);
#endif
}
void MacroPicker::onGDBrowser(cocos2d::CCObject*) {
#ifdef GEODE_IS_ANDROID
    // Browsing never carries the companion's selected macros to GD Browser.
    std::string error;
    if (!openGDBrowserAndroid(error)) alert("GD Browser unavailable", error);
#endif
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
