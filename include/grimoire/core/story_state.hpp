#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <functional>

namespace grimoire::core {

struct PlotDevice {
    std::string id;
    std::string name;
    std::string icon; // Icon type identifier
    std::string prompt;
    bool active{false};
};

struct ShopItem {
    std::string id;
    std::string name;
    std::string description;
    int cost{15};
    bool purchased{false};
};

struct Achievement {
    std::string id;
    std::string title;
    std::string description;
    bool unlocked{false};
};

struct CharacterTrait {
    std::string id;
    std::string title;
    std::string description;
    bool equipped{true};
};

class StoryState {
public:
    StoryState();

    void updateWordCount(size_t words);
    [[nodiscard]] size_t getWordCount() const noexcept { return currentWords_; }
    [[nodiscard]] size_t getGoalWords() const noexcept { return goalWords_; }
    void setGoalWords(size_t goal) noexcept { goalWords_ = goal > 0 ? goal : 1000; }
    [[nodiscard]] float getProgressRatio() const noexcept;

    [[nodiscard]] int getEmbers() const noexcept { return embers_; }
    void addEmbers(int amount) noexcept;
    bool spendEmbers(int amount) noexcept;

    [[nodiscard]] std::vector<PlotDevice>& getPlotDevices() noexcept { return plotDevices_; }
    [[nodiscard]] const std::vector<PlotDevice>& getPlotDevices() const noexcept { return plotDevices_; }
    void togglePlotDevice(size_t index);

    [[nodiscard]] std::vector<ShopItem>& getShopItems() noexcept { return shopItems_; }
    [[nodiscard]] const std::vector<ShopItem>& getShopItems() const noexcept { return shopItems_; }
    bool buyShopItem(size_t index);

    [[nodiscard]] std::vector<Achievement>& getAchievements() noexcept { return achievements_; }
    [[nodiscard]] const std::vector<Achievement>& getAchievements() const noexcept { return achievements_; }
    void checkMilestones();

    [[nodiscard]] std::vector<CharacterTrait>& getCharacterTraits() noexcept { return characterTraits_; }
    [[nodiscard]] const std::vector<CharacterTrait>& getCharacterTraits() const noexcept { return characterTraits_; }

    [[nodiscard]] const std::string& getActivePrompt() const noexcept { return activePrompt_; }
    void clearActivePrompt() noexcept { activePrompt_.clear(); }

    // Export / Persistence
    std::string exportToMarkdown(const std::string& manuscriptText) const;
    bool loadFromMarkdown(const std::string& markdownContent, std::string& outManuscriptText);

    // Callbacks
    std::function<void(const std::string& title)> onAchievementUnlocked;
    std::function<void(int newEmbers)> onEmbersChanged;
    std::function<void()> onMilestoneReached;

private:
    size_t currentWords_{0};
    size_t goalWords_{1000};
    int embers_{25}; // Starting embers
    size_t lastWordCheckpoint_{0};
    std::string activePrompt_;

    std::vector<PlotDevice> plotDevices_;
    std::vector<ShopItem> shopItems_;
    std::vector<Achievement> achievements_;
    std::vector<CharacterTrait> characterTraits_;
};

} // namespace grimoire::core
