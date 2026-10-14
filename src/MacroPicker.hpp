#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <filesystem>
#include <set>
#include <vector>

// Original artwork/UI are untouched: this is the companion's own popup,
// launched by a button attached to xdBot's LoadMacroLayer when detectable.
class MacroPicker : public geode::Popup {
    std::filesystem::path m_folder;
    std::vector<std::filesystem::path> m_allFiles;
    std::vector<std::filesystem::path> m_files; // visible matches of the current query
    std::set<std::filesystem::path> m_selected; // persists across search queries
    std::vector<CCMenuItemSpriteExtra*> m_rows;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    std::string m_query;
    std::size_t m_page = 0;
    static constexpr std::size_t perPage = 7;
    bool init(std::filesystem::path folder);
    void reload();
    void applyFilter();
    void updateStatus();
    void render();
    void onToggle(cocos2d::CCObject* sender);
    void onNext(cocos2d::CCObject*);
    void onPrev(cocos2d::CCObject*);
    void onAll(cocos2d::CCObject*);
    void onClear(cocos2d::CCObject*);
    void onExport(cocos2d::CCObject*);
    void onHyperbolus(cocos2d::CCObject*);
    void onShare(cocos2d::CCObject*);
public:
    static MacroPicker* create(std::filesystem::path folder);
};
