#include "AiMacroSearch.hpp"
#include "AiReplayJson.hpp"
#ifdef GEODE_IS_ANDROID
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/ui/Popup.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

using namespace geode::prelude;
namespace fs = std::filesystem;

namespace {
constexpr char const* kEnabled = "ai-record-experimental";
struct Session {
    PlayLayer* layer = nullptr; // invalidated on quit; never dereference after that
    std::unique_ptr<ai_macro::Search> search;
    std::vector<ai_macro::InputEvent> plan;
    std::vector<ai_macro::InputEvent> firstRun;
    std::vector<ai_macro::InputEvent> observed;
    std::size_t nextEvent = 0;
    unsigned lastTick = std::numeric_limits<unsigned>::max();
    unsigned firstCompletionFrame = 0;
    int levelID = 0;
    std::string levelName;
    bool injecting = false;
    bool resettingForStart = false;
    bool held = false;
    bool deathReported = false;
    bool verifying = false;

    bool active() const {
        return layer && search && (search->phase() == ai_macro::Phase::Attempting ||
            search->phase() == ai_macro::Phase::Verifying);
    }
    void release() {
        if (held && layer) {
            injecting = true;
            layer->handleButton(false, 1, true);
            injecting = false;
        }
        held = false;
    }
    void stop() {
        release();
        layer = nullptr;
        search.reset();
        plan.clear(); firstRun.clear(); observed.clear();
        nextEvent = 0; lastTick = std::numeric_limits<unsigned>::max();
        deathReported = false; verifying = false;
    }
    void startAttempt() {
        if (!search || !search->beginAttempt()) { stop(); return; }
        plan = search->plannedEvents();
        observed.clear(); nextEvent = 0;
        lastTick = std::numeric_limits<unsigned>::max();
        deathReported = false; verifying = false; held = false;
    }
    // Entering a level never starts AI. Beta.17 silently started here and then
    // treated an ordinary screen tap as a reason to turn the preference off.
    void enter(PlayLayer* play) {
        if (!play->m_level) { stop(); return; }
        int id = play->m_level->m_levelID.value();
        std::string name = play->m_level->m_levelName.c_str();
        if (!(search && search->phase() == ai_macro::Phase::VerifyReady &&
              id == levelID && name == levelName)) {
            stop();
            levelID = id;
            levelName = std::move(name);
        }
        layer = play;
    }
    // The on-screen button is the ONLY way to arm input injection. Starting
    // mid-level resets to frame zero before constructing any replay plan.
    bool startFor(PlayLayer* play) {
        if (layer != play || active() || !play->m_level ||
            play->m_isPracticeMode || play->m_isPlatformer ||
            play->m_gameState.m_isDualMode) return false;
        auto* xdbot = Loader::get()->getInstalledMod("zilko.xdbot");
        if (!xdbot || !xdbot->isLoaded()) return false;
        bool verify = search && search->phase() == ai_macro::Phase::VerifyReady &&
            play->m_level->m_levelID.value() == levelID &&
            std::string(play->m_level->m_levelName.c_str()) == levelName;
        // A deliberate reset is not a manual-restart kill switch. Keep the
        // search inactive until resetLevel and its input-release hooks finish.
        resettingForStart = true;
        play->resetLevel();
        resettingForStart = false;
        layer = play;
        if (verify) {
            if (!search->beginVerification()) { stop(); return false; }
            verifying = true;
            plan = search->plannedEvents();
            observed.clear(); nextEvent = 0;
            lastTick = std::numeric_limits<unsigned>::max();
            deathReported = false; held = false;
        } else {
            stop();
            layer = play;
            levelID = play->m_level->m_levelID.value();
            levelName = play->m_level->m_levelName.c_str();
            search = std::make_unique<ai_macro::Search>(80, 240 * 600);
            startAttempt();
        }
        return active();
    }
    void tick(GJBaseGameLayer* game) {
        if (game != layer || !active()) return;
        if (!Mod::get()->getSettingValue<bool>(kEnabled) || game->m_isPracticeMode ||
            game->m_isPlatformer || game->m_gameState.m_isDualMode) { stop(); return; }
        unsigned tick = game->m_gameState.m_currentProgress;
        if (lastTick != std::numeric_limits<unsigned>::max() && tick < lastTick) { stop(); return; }
        if (tick == lastTick) return;
        lastTick = tick;
        if (tick > 240u * 600u) { stop(); return; }
        while (nextEvent < plan.size() && plan[nextEvent].frame <= tick) {
            auto event = plan[nextEvent++];
            // Dropped steps change input timing. Do not turn a late input into
            // an apparently frame-accurate replay.
            if (event.frame != tick || observed.size() >= 100000) { stop(); return; }
            injecting = true;
            game->handleButton(event.down, event.button, true);
            injecting = false;
            held = event.down;
            observed.push_back(event);
        }
    }
    void died(PlayLayer* play) {
        if (play != layer || !active() || deathReported) return;
        deathReported = true;
        auto frame = play->m_gameState.m_currentProgress;
        release();
        if (verifying) { search->finishVerification({ai_macro::TrialResult::Kind::Death, frame}); stop(); return; }
        search->finishAttempt({ai_macro::TrialResult::Kind::Death, frame});
        if (search->phase() == ai_macro::Phase::Stopped) {
            stop();
        }
    }
    fs::path macroFolder() const {
        auto* xdbot = Loader::get()->getInstalledMod("zilko.xdbot");
        if (!xdbot || !xdbot->isLoaded()) return {};
        auto folder = xdbot->getSettingValue<fs::path>("macros_folder");
        if (folder.empty()) folder = geode::dirs::getGameDir() / "macros";
        if (folder.is_relative()) folder = geode::dirs::getGameDir() / folder;
        return folder;
    }
    std::string saveVerified(unsigned frame) const {
        if (firstRun.empty() || firstRun != observed || frame != firstCompletionFrame ||
            !search || !search->verifiedEvents()) return "Verification inputs or completion timing did not match; nothing was saved.";
        auto json = ai_macro::makeGdrJson({static_cast<unsigned>(std::max(levelID, 0)), levelName,
            frame, 240}, observed);
        if (!json || json->size() > 8 * 1024 * 1024) return "Replay encoding failed; nothing was saved.";
        auto dir = macroFolder();
        std::error_code ec;
        if (dir.empty() || !fs::is_directory(dir, ec) || ec) return "xdBot's macro folder is unavailable; nothing was saved.";
        for (int i = 1; i <= 100; ++i) {
            auto file = dir / ("companion-ai-" + std::to_string(levelID) + "-" + std::to_string(i) + ".gdr.json");
            if (fs::exists(file, ec) || ec) { ec.clear(); continue; }
            auto temp = fs::path(file.string() + ".part");
            if (fs::exists(temp, ec) || ec) { ec.clear(); continue; }
            {
                std::ofstream out(temp, std::ios::binary | std::ios::trunc);
                if (!out || !(out << *json)) { fs::remove(temp, ec); return "Could not write the candidate replay."; }
                out.close();
                if (!out) { fs::remove(temp, ec); return "Could not finish writing the candidate replay."; }
            }
            // Link into the final name without overwriting a macro that may
            // have appeared since the earlier exists check, then drop staging.
            fs::create_hard_link(temp, file, ec);
            if (ec) { fs::remove(temp, ec); return "Could not publish the candidate replay safely."; }
            fs::remove(temp, ec);
            return "Saved " + file.filename().string() + " locally. Try importing and replaying it in xdBot before sharing; compatibility is unverified.";
        }
        return "Too many AI replay filenames for this level; nothing was saved.";
    }
    void completed(PlayLayer* play) {
        if (play != layer || !active()) return;
        unsigned frame = play->m_gameState.m_currentProgress;
        release();
        if (!verifying) {
            if (nextEvent != plan.size() || observed != plan) { stop(); return; }
            search->finishAttempt({ai_macro::TrialResult::Kind::Completed, frame});
            if (search->phase() != ai_macro::Phase::VerifyReady) { stop(); return; }
            firstRun = observed; firstCompletionFrame = frame;
            layer = nullptr; // keep only the candidate across exit/re-entry
            geode::queueInMainThread([] {
                geode::createQuickPopup("AI candidate found",
                    "Re-enter this same level and tap VERIFY AI to replay the candidate. Nothing has been saved or uploaded yet.",
                    "OK", nullptr, [](auto*, bool) {});
            });
        } else {
            search->finishVerification({ai_macro::TrialResult::Kind::Completed, frame});
            auto message = saveVerified(frame);
            stop();
            geode::queueInMainThread([message = std::move(message)] {
                geode::createQuickPopup("AI replay result", message, "OK", nullptr, [](auto*, bool) {});
            });
        }
    }
};
Session g_ai;
constexpr char const* kMenuID = "jimmy_1234ha.xdbot-companion/ai-button";
constexpr char const* kStartID = "jimmy_1234ha.xdbot-companion/ai-start";
constexpr char const* kStopID = "jimmy_1234ha.xdbot-companion/ai-stop";
constexpr char const* kVerifyID = "jimmy_1234ha.xdbot-companion/ai-verify";
void syncButton(PlayLayer* play) {
    if (!play || !play->m_uiLayer) return;
    auto* menu = play->m_uiLayer->getChildByID(kMenuID);
    if (!menu) return;
    bool running = g_ai.active();
    bool verify = !running && g_ai.search &&
        g_ai.search->phase() == ai_macro::Phase::VerifyReady;
    if (auto* start = menu->getChildByID(kStartID)) start->setVisible(!running && !verify);
    if (auto* stop = menu->getChildByID(kStopID)) stop->setVisible(running);
    if (auto* next = menu->getChildByID(kVerifyID)) next->setVisible(verify);
}
} // namespace

class $modify(CompanionAIGame, PlayLayer) {
    void onAiControl(cocos2d::CCObject*) {
        if (g_ai.active()) {
            g_ai.stop();
            syncButton(this);
            geode::createQuickPopup("AI stopped", "The AI attempt was cancelled. Nothing was uploaded.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        if (!m_level || m_isPracticeMode || m_isPlatformer || m_gameState.m_isDualMode) {
            geode::createQuickPopup("AI unavailable", "Use a disposable normal-mode classic cube level. Practice, dual and platformer are not supported.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        auto* xdbot = Loader::get()->getInstalledMod("zilko.xdbot");
        if (!xdbot || !xdbot->isLoaded()) {
            geode::createQuickPopup("AI unavailable", "xdBot must be installed and loaded to save a candidate macro.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        bool verify = g_ai.search && g_ai.search->phase() == ai_macro::Phase::VerifyReady;
        std::string prompt = verify
            ? "Replay the candidate on this level to verify it? Nothing will be uploaded."
            : "Reset this level and let experimental AI try up to 80 times? It is a limited jump-timing heuristic, not an any-level solver. Nothing will be uploaded.";
        geode::createQuickPopup(verify ? "VERIFY AI" : "START AI", prompt,
            "Cancel", verify ? "Verify" : "Start", [this](auto*, bool confirmed) {
                if (!confirmed || PlayLayer::get() != this || !m_level) return;
                Mod::get()->setSettingValue<bool>(kEnabled, true); // preference; never auto-clear it
                if (!g_ai.startFor(this)) {
                    geode::createQuickPopup("AI could not start", "Check that xdBot is loaded and this is a supported normal-mode level.",
                        "OK", nullptr, [](auto*, bool) {});
                }
                syncButton(this);
            });
    }
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        g_ai.enter(this);
        if (m_uiLayer) {
            auto* menu = cocos2d::CCMenu::create();
            if (menu) {
                menu->setID(kMenuID);
                menu->setPosition({0.f, 0.f});
                auto makeButton = [this](char const* text, char const* id) {
                    auto* body = cocos2d::CCLayerColor::create({30, 18, 8, 230}, 106.f, 31.f);
                    auto* label = cocos2d::CCLabelBMFont::create(text, "goldFont.fnt");
                    if (!body || !label) return static_cast<CCMenuItemSpriteExtra*>(nullptr);
                    body->ignoreAnchorPointForPosition(false);
                    label->setScale(.37f);
                    label->setPosition({53.f, 15.5f});
                    body->addChild(label);
                    auto* item = CCMenuItemSpriteExtra::create(body, this,
                        menu_selector(CompanionAIGame::onAiControl));
                    if (item) item->setID(id);
                    return item;
                };
                auto* start = makeButton("START AI", kStartID);
                auto* stop = makeButton("STOP AI", kStopID);
                auto* verify = makeButton("VERIFY AI", kVerifyID);
                if (start && stop && verify) {
                    auto height = cocos2d::CCDirector::get()->getWinSize().height;
                    start->setPosition({67.f, height - 61.f});
                    stop->setPosition({67.f, height - 61.f});
                    verify->setPosition({67.f, height - 61.f});
                    menu->addChild(start);
                    menu->addChild(stop);
                    menu->addChild(verify);
                    m_uiLayer->addChild(menu, 1000);
                    syncButton(this);
                }
            }
        }
        return true;
    }
    void resetLevel() {
        bool died = g_ai.layer == this && g_ai.deathReported;
        if (!g_ai.resettingForStart && g_ai.layer == this && g_ai.active() &&
            m_gameState.m_currentProgress > 0 && !died) {
            g_ai.stop(); // manual restart cancels this run, not the preference
        }
        PlayLayer::resetLevel();
        if (g_ai.layer == this && died && g_ai.search &&
            g_ai.search->phase() == ai_macro::Phase::Ready) g_ai.startAttempt();
        syncButton(this);
    }
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (player == m_player1) g_ai.died(this);
        PlayLayer::destroyPlayer(player, object);
        syncButton(this);
    }
    void levelComplete() {
        g_ai.completed(this);
        PlayLayer::levelComplete();
        syncButton(this);
    }
    void onQuit() {
        if (g_ai.layer == this) {
            if (g_ai.search && g_ai.search->phase() == ai_macro::Phase::VerifyReady)
                g_ai.layer = nullptr;
            else g_ai.stop();
        }
        PlayLayer::onQuit();
    }
    void onExit() {
        if (g_ai.layer == this) {
            if (g_ai.search && g_ai.search->phase() == ai_macro::Phase::VerifyReady)
                g_ai.layer = nullptr;
            else g_ai.stop();
        }
        PlayLayer::onExit();
    }
};

class $modify(CompanionAITick, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        g_ai.tick(this);
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        if (auto* play = PlayLayer::get(); play &&
            static_cast<GJBaseGameLayer*>(play) ==
            static_cast<GJBaseGameLayer*>(this)) syncButton(play);
    }
};

class $modify(CompanionAIInput, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        // A real human input cancels the automated attempt, but must not
        // silently change the persistent preference or remove the AI button.
        if (!g_ai.resettingForStart && g_ai.layer &&
            static_cast<GJBaseGameLayer*>(g_ai.layer) ==
            static_cast<GJBaseGameLayer*>(this) && g_ai.active() && !g_ai.injecting) {
            g_ai.stop();
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};
#endif // GEODE_IS_ANDROID
