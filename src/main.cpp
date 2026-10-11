#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/ui/Popup.hpp>
#include "MacroScanner.hpp"
#include "AndroidShare.hpp"
#include "MacroPicker.hpp"
#include <typeinfo>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>

using namespace geode::prelude;

namespace {
struct Session {
    bool show = false;
    int deaths = 0;
    int lastHazard = 0;
    float lastDeathPercent = -1.f;
    float renderFps = 0.f;
};
Session g_session;

std::string modStatus(char const* id) {
    auto* mod = Loader::get()->getInstalledMod(id);
    if (!mod) return "NOT INSTALLED";
    return mod->getVersion().toVString() + (mod->isLoaded() ? "  LOADED" : "  NOT LOADED");
}
std::string playMode(PlayerObject* p) {
    if (!p) return "--";
    if (p->m_isSwing) return "Swing";
    if (p->m_isSpider) return "Spider";
    if (p->m_isDart) return "Wave";
    if (p->m_isShip) return "Ship";
    if (p->m_isBird) return "UFO";
    if (p->m_isBall) return "Ball";
    if (p->m_isRobot) return "Robot";
    return "Cube";
}
} // namespace

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
        if (std::string_view(typeid(*node).name()) == "14LoadMacroLayer") {
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
            "Macro saved", "Share <cy>" + name + "</c> using Android? No upload happens until you choose an app.",
            "Later", "Share now", [saved = *saved](auto*, bool share) {
                g_promptOpen = false;
                if (share) shareSelectedMacro(saved);
            }
        );
    }
};
} // namespace

$on_mod(Loaded) {
    // Schedule on the Cocos main thread, independent of PlayLayer pause state.
    geode::queueInMainThread([] {
        auto* watcher = new ShareWatcher; // lives for the mod's process lifetime
        cocos2d::CCDirector::get()->getScheduler()->scheduleSelector(
            schedule_selector(ShareWatcher::poll), watcher, .85f, false);
    });
}

class $modify(CompanionPlay, PlayLayer) {
    struct Fields {
        cocos2d::CCLabelBMFont* hud = nullptr;
        std::chrono::steady_clock::time_point lastSample{};
        unsigned int lastFrames = 0;
        bool countedDeath = false;
    };
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        g_session = {};
        g_session.show = Mod::get()->getSettingValue<bool>("monitor-start");
        auto* hud = cocos2d::CCLabelBMFont::create("", "goldFont.fnt");
        if (hud) {
            hud->setAnchorPoint({0.f, 1.f});
            hud->setScale(.30f);
            hud->setPosition({8.f, cocos2d::CCDirector::get()->getWinSize().height - 9.f});
            hud->setColor(cocos2d::ccc3(173, 227, 255));
            hud->setVisible(g_session.show);
            this->addChild(hud, 3000);
            m_fields->hud = hud;
        }
        m_fields->lastSample = std::chrono::steady_clock::now();
        m_fields->lastFrames = cocos2d::CCDirector::get()->getTotalFrames();
        return true;
    }
    void resetLevel() {
        m_fields->countedDeath = false;
        PlayLayer::resetLevel();
    }
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (player == m_player1 && !m_fields->countedDeath) {
            m_fields->countedDeath = true;
            ++g_session.deaths;
            g_session.lastDeathPercent = std::clamp(this->getCurrentPercent(), 0.f, 100.f);
            g_session.lastHazard = object ? object->m_objectID : 0;
        }
        PlayLayer::destroyPlayer(player, object);
    }
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        auto* hud = m_fields->hud;
        if (!hud) return;
        hud->setVisible(g_session.show && !m_isPaused);
        // Director frames are rendered frames; postUpdate can run several times per render.
        // Sample even when the HUD is OFF, so the pause card has a useful FPS reading.
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<float>(now - m_fields->lastSample).count();
        if (elapsed < .55f) return;
        auto frames = cocos2d::CCDirector::get()->getTotalFrames();
        // Ignore a sample that spans a long OS suspension or pause instead of
        // presenting it as a severe gameplay slowdown.
        if (elapsed < 3.f) {
            g_session.renderFps = std::clamp((frames - m_fields->lastFrames) / elapsed, 0.f, 500.f);
        }
        m_fields->lastFrames = frames;
        m_fields->lastSample = now;
        if (g_session.show) {
            char buf[112];
            std::snprintf(buf, sizeof(buf), "FPS %.0f  |  DEATHS %d  |  LAST %.1f%%", g_session.renderFps,
                g_session.deaths, std::max(0.f, g_session.lastDeathPercent));
            hud->setString(buf);
        }
    }
};

class $modify(CompanionPause, PauseLayer) {
    struct Fields {
        cocos2d::CCNode* panel = nullptr;
        cocos2d::CCLabelBMFont* info = nullptr;
        cocos2d::CCLabelBMFont* monitor = nullptr;
    };
    void customSetup() {
        PauseLayer::customSetup();
        auto win = cocos2d::CCDirector::get()->getWinSize();
        auto* btnLabel = cocos2d::CCLabelBMFont::create("XD TOOLS", "bigFont.fnt");
        if (!btnLabel) return;
        btnLabel->setScale(.42f);
        btnLabel->setColor(cocos2d::ccc3(169, 215, 255));
        auto* menu = cocos2d::CCMenu::create();
        menu->setPosition({0.f, 0.f});
        auto* button = CCMenuItemSpriteExtra::create(btnLabel, this, menu_selector(CompanionPause::onPanel));
        button->setPosition({win.width - 50.f, 30.f});
        menu->addChild(button);
        addChild(menu, 9900);

        // Compact dark-blue card; colours echo xdBot's published default (#334499).
        auto* bg = cocos2d::CCLayerColor::create(cocos2d::ccc4(51, 68, 153, 245), 264.f, 130.f);
        if (!bg) return;
        bg->setPosition({(win.width - 264.f) / 2.f, (win.height - 130.f) / 2.f});
        bg->setVisible(false);
        addChild(bg, 10000);
        m_fields->panel = bg;
        auto* title = cocos2d::CCLabelBMFont::create("XDBOT COMPANION", "bigFont.fnt");
        title->setScale(.47f);
        title->setPosition({132.f, 114.f});
        bg->addChild(title);
        auto* info = cocos2d::CCLabelBMFont::create("", "chatFont.fnt");
        info->setScale(.44f);
        info->setAnchorPoint({0.f, 1.f});
        info->setPosition({13.f, 99.f});
        info->setColor(cocos2d::ccc3(225, 239, 255));
        bg->addChild(info);
        m_fields->info = info;
        auto* monitor = cocos2d::CCLabelBMFont::create("", "goldFont.fnt");
        monitor->setScale(.40f);
        m_fields->monitor = monitor;
        auto* cardMenu = cocos2d::CCMenu::create();
        cardMenu->setPosition({0.f, 0.f});
        auto* monitorBtn = CCMenuItemSpriteExtra::create(monitor, this, menu_selector(CompanionPause::onMonitor));
        monitorBtn->setPosition({48.f, 14.f});
        cardMenu->addChild(monitorBtn);
        auto* share = cocos2d::CCLabelBMFont::create("MACROS", "goldFont.fnt");
        share->setScale(.40f);
        auto* shareBtn = CCMenuItemSpriteExtra::create(share, this, menu_selector(CompanionPause::onBrowse));
        shareBtn->setPosition({132.f, 14.f});
        cardMenu->addChild(shareBtn);
        auto* close = cocos2d::CCLabelBMFont::create("CLOSE", "goldFont.fnt");
        close->setScale(.40f);
        auto* closeBtn = CCMenuItemSpriteExtra::create(close, this, menu_selector(CompanionPause::onPanel));
        closeBtn->setPosition({218.f, 14.f});
        cardMenu->addChild(closeBtn);
        bg->addChild(cardMenu);
        refresh();
    }
    void refresh() {
        if (m_fields->monitor) m_fields->monitor->setString(g_session.show ? "HUD: ON" : "HUD: OFF");
        if (!m_fields->info) return;
        auto* play = PlayLayer::get();
        auto* player = play ? play->m_player1 : nullptr;
        char details[400];
        std::string xb = modStatus("zilko.xdbot");
        std::string ff = modStatus("eclipse.ffmpeg-api");
        std::snprintf(details, sizeof(details),
            "xdBot: %s\nFFmpeg API: %s\nFPS: %.0f  Mode: %s\nDeaths: %d  Last: %.1f%%  Object: %d\nMacro sharing: only after you approve.",
            xb.c_str(), ff.c_str(), g_session.renderFps, playMode(player).c_str(),
            g_session.deaths, std::max(0.f, g_session.lastDeathPercent), g_session.lastHazard);
        m_fields->info->setString(details);
    }
    void onPanel(cocos2d::CCObject*) {
        if (!m_fields->panel) return;
        m_fields->panel->setVisible(!m_fields->panel->isVisible());
        if (m_fields->panel->isVisible()) {
            refresh();
            log::info("xdBot Companion: xdBot={}, ffmpeg={}, renderFPS={}, deaths={}, lastPct={}",
                modStatus("zilko.xdbot"), modStatus("eclipse.ffmpeg-api"),
                g_session.renderFps, g_session.deaths, g_session.lastDeathPercent);
        }
    }
    void onMonitor(cocos2d::CCObject*) {
        g_session.show = !g_session.show;
        refresh();
    }
    void onBrowse(cocos2d::CCObject*) {
        openMacroPicker();
    }
};
