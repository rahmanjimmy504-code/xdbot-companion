#include "AiMacroSearch.hpp"
#include "AiReplayJson.hpp"
#ifdef GEODE_IS_ANDROID
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
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
    bool pendingStart = false;
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
        deathReported = false; verifying = false; pendingStart = false;
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
    // The PAUSE-menu button arms a run; Resume starts it. Rebind a current
    // PlayLayer even after a previous stop() cleared the session pointer.
    // Beta.18's layer != play guard rejected all subsequent starts after a
    // cancellation and returned the generic popup seen on the user's tablet.
    std::string startFor(PlayLayer* play) {
        if (!play || PlayLayer::get() != play || !play->m_level)
            return "The level changed before the AI could start. Nothing was saved.";
        if (active()) return "An AI attempt is already running.";
        if (play->m_isPracticeMode) return "Practice mode is unsupported. Switch to normal mode.";
        if (play->m_isPlatformer) return "Platformer mode is unsupported.";
        if (play->m_gameState.m_isDualMode) return "Dual mode is unsupported.";
        auto* xdbot = Loader::get()->getInstalledMod("zilko.xdbot");
        if (!xdbot || !xdbot->isLoaded())
            return "xdBot (zilko.xdbot) is not loaded. Check Geode's mod list.";
        bool verify = search && search->phase() == ai_macro::Phase::VerifyReady &&
            play->m_level->m_levelID.value() == levelID &&
            std::string(play->m_level->m_levelName.c_str()) == levelName;
        if (!verify) {
            stop();
            layer = play;
            levelID = play->m_level->m_levelID.value();
            levelName = play->m_level->m_levelName.c_str();
        } else {
            layer = play;
        }
        // Deliberate reset from the Pause-layer Resume path is not a manual
        // restart. Initialize the AI plan only after the base reset finishes.
        resettingForStart = true;
        play->resetLevel();
        resettingForStart = false;
        if (PlayLayer::get() != play || !play->m_level) {
            stop();
            return "The level ended during reset; no AI attempt was started.";
        }
        layer = play;
        if (verify) {
            if (!search || !search->beginVerification()) {
                stop();
                return "The saved in-memory candidate cannot be verified; start a new attempt.";
            }
            verifying = true;
            plan = search->plannedEvents();
            observed.clear(); nextEvent = 0;
            lastTick = std::numeric_limits<unsigned>::max();
            deathReported = false; held = false;
        } else {
            search = std::make_unique<ai_macro::Search>(80, 240 * 600);
            startAttempt();
        }
        if (!active()) return "The timing search could not begin; nothing was saved.";
        return {};
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
constexpr char const* kMenuID = "jimmy_1234ha.xdbot-companion/pause-ai-menu";
} // namespace

class $modify(CompanionAIGame, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        g_ai.enter(this);
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
    }
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (player == m_player1) g_ai.died(this);
        PlayLayer::destroyPlayer(player, object);
    }
    void levelComplete() {
        g_ai.completed(this);
        PlayLayer::levelComplete();
    }
    void onQuit() {
        g_ai.pendingStart = false;
        if (g_ai.layer == this) {
            if (g_ai.search && g_ai.search->phase() == ai_macro::Phase::VerifyReady)
                g_ai.layer = nullptr;
            else g_ai.stop();
        }
        PlayLayer::onQuit();
    }
    void onExit() {
        g_ai.pendingStart = false;
        if (g_ai.layer == this) {
            if (g_ai.search && g_ai.search->phase() == ai_macro::Phase::VerifyReady)
                g_ai.layer = nullptr;
            else g_ai.stop();
        }
        PlayLayer::onExit();
    }
};

// A standalone pause-menu action, not the removed XD TOOLS panel or a HUD.
// No AI gameplay is injected until the player explicitly arms and Resumes.
class $modify(CompanionAIPauseControl, PauseLayer) {
    void onAIButton(cocos2d::CCObject*) {
        auto* play = PlayLayer::get();
        if (!play || !play->m_level) {
            geode::createQuickPopup("AI unavailable", "There is no active level.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        if (g_ai.active()) {
            g_ai.stop();
            geode::createQuickPopup("AI stopped", "Attempt cancelled. Nothing was uploaded.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        if (g_ai.pendingStart) {
            g_ai.pendingStart = false;
            geode::createQuickPopup("AI disarmed", "The pending attempt will not start on Resume.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        if (play->m_isPracticeMode || play->m_isPlatformer || play->m_gameState.m_isDualMode) {
            geode::createQuickPopup("AI unavailable", "Use a disposable normal-mode classic cube level. Practice, dual and platformer are unsupported.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        auto* xdbot = Loader::get()->getInstalledMod("zilko.xdbot");
        if (!xdbot || !xdbot->isLoaded()) {
            geode::createQuickPopup("AI unavailable", "xdBot (zilko.xdbot) is not loaded in Geode.",
                "OK", nullptr, [](auto*, bool) {});
            return;
        }
        bool verify = g_ai.search && g_ai.search->phase() == ai_macro::Phase::VerifyReady &&
            play->m_level->m_levelID.value() == g_ai.levelID &&
            std::string(play->m_level->m_levelName.c_str()) == g_ai.levelName;
        std::string prompt = verify
            ? "Verify the candidate on this level? Confirm, then tap RESUME. It will reset from the beginning. Nothing is uploaded."
            : "Experimental timing heuristic: up to 80 jump attempts, not an any-level solver. Confirm, then tap RESUME to reset and start this level. Nothing is uploaded.";
        geode::createQuickPopup(verify ? "VERIFY AI" : "START AI", std::move(prompt),
            "Cancel", "Arm AI", [play](auto*, bool confirmed) {
                if (!confirmed || PlayLayer::get() != play || !play->m_level) return;
                Mod::get()->setSettingValue<bool>(kEnabled, true);
                g_ai.pendingStart = true;
            });
    }
    void customSetup() {
        PauseLayer::customSetup();
        if (!PlayLayer::get()) return;
        auto* menu = cocos2d::CCMenu::create();
        if (!menu) return;
        menu->setID(kMenuID);
        menu->setPosition({0.f, 0.f});
        char const* text = g_ai.active() ? "STOP AI" :
            g_ai.pendingStart ? "DISARM AI" :
            (g_ai.search && g_ai.search->phase() == ai_macro::Phase::VerifyReady)
                ? "VERIFY AI" : "START AI";
        auto* body = cocos2d::CCLayerColor::create({42, 25, 12, 235}, 120.f, 32.f);
        auto* label = cocos2d::CCLabelBMFont::create(text, "goldFont.fnt");
        if (!body || !label) return;
        body->ignoreAnchorPointForPosition(false);
        label->setScale(.4f);
        label->setPosition({60.f, 16.f});
        body->addChild(label);
        auto* button = CCMenuItemSpriteExtra::create(body, this,
            menu_selector(CompanionAIPauseControl::onAIButton));
        if (!button) return;
        auto size = cocos2d::CCDirector::get()->getWinSize();
        button->setPosition({78.f, size.height - 51.f});
        menu->addChild(button);
        addChild(menu, 2000);
    }
    void onResume(cocos2d::CCObject* sender) {
        bool armed = g_ai.pendingStart;
        g_ai.pendingStart = false;
        auto* play = PlayLayer::get();
        PauseLayer::onResume(sender);
        if (!armed) return;
        auto error = g_ai.startFor(play);
        if (!error.empty()) {
            geode::queueInMainThread([message = std::move(error)] {
                geode::createQuickPopup("AI could not start", message, "OK", nullptr,
                    [](auto*, bool) {});
            });
        }
    }
    void onRestart(cocos2d::CCObject* sender) {
        g_ai.pendingStart = false;
        PauseLayer::onRestart(sender);
    }
    void onRestartFull(cocos2d::CCObject* sender) {
        g_ai.pendingStart = false;
        PauseLayer::onRestartFull(sender);
    }
    void onQuit(cocos2d::CCObject* sender) {
        g_ai.pendingStart = false;
        PauseLayer::onQuit(sender);
    }
};

class $modify(CompanionAITick, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        g_ai.tick(this);
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
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
