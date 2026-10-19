#include <Geode/Geode.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/ui/Popup.hpp>
#include "MacroScanner.hpp"
#include "AndroidShare.hpp"
#include "MacroPicker.hpp"
#include <typeinfo>
#include <algorithm>
#include <string>

using namespace geode::prelude;

namespace {
MacroScanner g_scanner;
bool g_promptOpen = false;

std::filesystem::path macroFolder() {
    auto* xdbot = Loader::get()->getInstalledMod("zilko.xdbot");
    if (!xdbot || !xdbot->isLoaded()) return {};
    auto folder = xdbot->getSettingValue<std::filesystem::path>("macros_folder");
    if (folder.empty()) folder = geode::dirs::getGameDir() / "macros";
    if (folder.is_relative()) folder = geode::dirs::getGameDir() / folder;
    return folder;
}
void showSharingError(std::string const& error) {
    geode::createQuickPopup("Sharing unavailable", error, "OK", nullptr, [](auto*, bool) {});
}
void shareSelectedMacro(std::filesystem::path const& path) {
    std::string error;
    if (!shareMacroFile(path, error)) showSharingError(error);
}
void openMacroPicker() {
    auto folder = macroFolder();
    std::error_code ec;
    if (folder.empty() || !std::filesystem::is_directory(folder, ec) || ec) {
        showSharingError("No xdBot macro folder is available. Check xdBot's Macro Save Location.");
        return;
    }
    if (auto* popup = MacroPicker::create(folder)) popup->show();
}

// A small brown, light-outlined plaque inspired by the profile frame. Drawn
// locally; no xdBot image files are embedded or redistributed.
cocos2d::CCNode* makeSharePlaque() {
    constexpr float w = 78.f, h = 22.f;
    auto* art = cocos2d::CCDrawNode::create();
    if (!art) return nullptr;
    art->setContentSize({w, h});
    auto rounded = [art](float x, float y, float width, float height, float r,
                         cocos2d::ccColor4F color) {
        cocos2d::CCPoint corners[] = {
            {x+r, y}, {x+width-r, y}, {x+width, y+r}, {x+width, y+height-r},
            {x+width-r, y+height}, {x+r, y+height}, {x, y+height-r}, {x, y+r}
        };
        art->drawPolygon(corners, 8, color, 0.f, color);
    };
    rounded(1.f, 0.f, w-1.f, h-2.f, 3.f, {0.12f, 0.06f, 0.04f, .9f});
    rounded(0.f, 1.5f, w-2.f, h-2.f, 3.f, {.95f, .92f, .83f, 1.f});
    rounded(2.2f, 3.7f, w-6.4f, h-6.3f, 2.f, {.43f, .23f, .13f, 1.f});
    rounded(3.7f, 5.2f, w-9.4f, h-9.3f, 1.5f, {.64f, .35f, .19f, 1.f});
    auto* label = cocos2d::CCLabelBMFont::create("SHARE MACROS", "goldFont.fnt");
    if (label) {
        label->setScale(.29f);
        label->setPosition({(w-2.f)/2.f, (h+1.f)/2.f});
        art->addChild(label, 1);
    }
    return art;
}

class ShareWatcher : public cocos2d::CCObject {
    void onOpenPicker(cocos2d::CCObject*) { openMacroPicker(); }
    void attachButton(cocos2d::CCNode* node, int depth) {
        if (!node || depth > 5) return;
        // Attach to m_mainLayer as in v1.2.1. Avoid casting/scanning
        // xdBot's children: Android's mixed RTTI crashed in v1.2.2.
        auto typeName = std::string_view(typeid(*node).name());
        if (typeName == "14LoadMacroLayer"
#ifdef GEODE_IS_WINDOWS
            || typeName.find("LoadMacroLayer") != std::string_view::npos
#endif
        ) {
            auto* main = static_cast<FLAlertLayer*>(node)->m_mainLayer;
            if (main && !main->getChildByID("jimmy_1234ha.xdbot-companion/share-entry")) {
                auto* menu = cocos2d::CCMenu::create();
                if (!menu) return;
                menu->setID("jimmy_1234ha.xdbot-companion/share-entry");
                menu->setPosition({0.f, 0.f});
                auto* plaque = makeSharePlaque();
                if (!plaque) return;
                auto* button = CCMenuItemSpriteExtra::create(plaque, this, menu_selector(ShareWatcher::onOpenPicker));
                if (!button) return;
                // xdBot 2.7.7: the bottom controls have SELECT ALL on the
                // left and the trash/folder/add icons on the right. In the
                // actual phone screenshot, (234,39) is the open space between
                // SELECT ALL and trash. The compact plaque is 78 x 22.
                button->setPosition({234.f, 39.f});
                menu->addChild(button);
                main->addChild(menu, 999);
            }
            return;
        }
        if (auto* children = node->getChildren()) {
            for (auto* child : CCArrayExt<cocos2d::CCNode*>(children)) attachButton(child, depth + 1);
        }
    }
public:
    void poll(float) {
        auto folder = macroFolder();
        if (folder.empty()) { g_scanner.reset(); return; }
        if (auto* scene = cocos2d::CCDirector::get()->getRunningScene()) attachButton(scene, 0);
        auto saved = g_scanner.scan(folder);
        if (!saved || g_promptOpen || !Mod::get()->getSettingValue<bool>("share-after-save")) return;
        if (!cocos2d::CCDirector::get()->getRunningScene()) return;
        g_promptOpen = true;
        auto name = saved->filename().string();
        if (name.size() > 38) name = name.substr(0, 35) + "...";
        std::replace_if(name.begin(), name.end(), [](char c) { return c == '<' || c == '>'; }, '_');
        geode::createQuickPopup(
            "Macro saved", "Share <cy>" + name + "</c> with another app? No file is sent until you choose a target.",
            "Later", "Share now", [saved = *saved](auto*, bool share) {
                g_promptOpen = false;
                if (share) shareSelectedMacro(saved);
            }
        );
    }
};
} // namespace

$on_mod(Loaded) {
    // No gameplay or pause-layer hooks; only the macro sharing entry remains.
    // Schedule on the Cocos main thread, independent of PlayLayer pause state.
    geode::queueInMainThread([] {
        auto* watcher = new ShareWatcher; // lives for the mod's process lifetime
        cocos2d::CCDirector::get()->getScheduler()->scheduleSelector(
            schedule_selector(ShareWatcher::poll), watcher, .85f, false);
    });
}
