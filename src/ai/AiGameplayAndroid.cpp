#include "AiMacroSearch.hpp"
#include "AiReplayJson.hpp"
#ifdef GEODE_IS_ANDROID
#include <Geode/Geode.hpp>
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
    void enter(PlayLayer* play) {
        if (!Mod::get()->getSettingValue<bool>(kEnabled)) { stop(); return; }
        auto* xdbot = Loader::get()->getInstalledMod("zilko.xdbot");
        if (!xdbot || !xdbot->isLoaded() || !play->m_level ||
            play->m_isPracticeMode || play->m_isPlatformer || play->m_gameState.m_isDualMode) {
            stop(); return;
        }
        int id = play->m_level->m_levelID.value();
        std::string name = play->m_level->m_levelName.c_str();
        if (search && search->phase() == ai_macro::Phase::VerifyReady &&
            id == levelID && name == levelName) {
            layer = play;
            if (!search->beginVerification()) { stop(); return; }
            verifying = true;
            plan = search->plannedEvents();
            observed.clear(); nextEvent = 0;
            lastTick = std::numeric_limits<unsigned>::max();
            deathReported = false; held = false;
            return;
        }
        stop();
        layer = play; levelID = id; levelName = std::move(name);
        search = std::make_unique<ai_macro::Search>(80, 240 * 600);
        startAttempt();
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
            Mod::get()->setSettingValue<bool>(kEnabled, false);
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
                    "Start this same level again to replay and verify the AI attempt. Nothing has been saved or uploaded yet.",
                    "OK", nullptr, [](auto*, bool) {});
            });
        } else {
            search->finishVerification({ai_macro::TrialResult::Kind::Completed, frame});
            auto message = saveVerified(frame);
            Mod::get()->setSettingValue<bool>(kEnabled, false);
            stop();
            geode::queueInMainThread([message = std::move(message)] {
                geode::createQuickPopup("AI replay result", message, "OK", nullptr, [](auto*, bool) {});
            });
        }
    }
};
Session g_ai;
} // namespace

class $modify(CompanionAIGame, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        g_ai.enter(this);
        return true;
    }
    void resetLevel() {
        bool died = g_ai.layer == this && g_ai.deathReported;
        if (g_ai.layer == this && g_ai.active() &&
            m_gameState.m_currentProgress > 0 && !died) {
            Mod::get()->setSettingValue<bool>(kEnabled, false);
            g_ai.stop(); // manual restart is a kill switch
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
        if (g_ai.layer == this) {
            Mod::get()->setSettingValue<bool>(kEnabled, false);
            g_ai.stop();
        }
        PlayLayer::onQuit();
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
        // A real human input cancels the automated attempt. Our own injected
        // calls are guarded, and the user's input still reaches the game.
        if (g_ai.layer && static_cast<GJBaseGameLayer*>(g_ai.layer) ==
            static_cast<GJBaseGameLayer*>(this) && g_ai.active() && !g_ai.injecting) {
            Mod::get()->setSettingValue<bool>(kEnabled, false);
            g_ai.stop();
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};
#endif // GEODE_IS_ANDROID
