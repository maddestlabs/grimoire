#include "grimoire/core/story_state.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace grimoire::core {

StoryState::StoryState() {
    // 3 Active Plot Devices matching screenshot
    plotDevices_ = {
        {
            "dagger",
            "Dagger of Peril",
            "dagger",
            "Conflict & Climax: A hidden assassin strikes, or an insurmountable dilemma forces a sacrifice.",
            false
        },
        {
            "quill",
            "Quill of Revelation",
            "quill",
            "Exposition & Lore: Unearth a forgotten inscription in ancient tongue detailing the realm's doom.",
            false
        },
        {
            "key",
            "Skeleton Key",
            "key",
            "Discovery & Secrets: What locked door or arcane puzzle yielded to your protagonist?",
            false
        }
    };

    // Story Upgrade Shop matching screenshot
    shopItems_ = {
        {
            "candle_amber",
            "Scented Amber Wax",
            "Enhances dynamic candle flicker smoothness and warms the parchment tone.",
            15,
            false
        },
        {
            "rain_loop",
            "Sanctuary Lantern",
            "Summons a calming thunderstorm against the window panes.",
            20,
            false
        },
        {
            "quill_gold",
            "Golden Starburst Nib",
            "Gilds the book binding and unlocks enchanted ink flourishes.",
            30,
            false
        }
    };

    // Achievements matching screenshot (Dagger, Lantern, Goblet)
    achievements_ = {
        {
            "first_ink",
            "First Drop of Ink",
            "Inscribed the opening lines of your chronicle.",
            false
        },
        {
            "candlelit_vigil",
            "Candlelit Vigil",
            "Wrote 250 words while the candles burned low.",
            false
        },
        {
            "master_scribe",
            "Master of the Grimoire",
            "Reached the grand 1000-word milestone.",
            false
        }
    };

    // Character Traits matching screenshot (Sword, Crest, Starburst)
    characterTraits_ = {
        {
            "iron_will",
            "The Iron Will",
            "Undaunted resolve when writing thrilling climaxes.",
            true
        },
        {
            "ancient_blood",
            "Blood of the Keepers",
            "Attunement with forgotten arcane mysteries.",
            true
        },
        {
            "starlight_weaver",
            "Starlight Weaver",
            "Poetic imagery flows effortless through dialogue.",
            true
        }
    };
}

float StoryState::getProgressRatio() const noexcept {
    if (goalWords_ == 0) return 1.0f;
    return std::clamp(static_cast<float>(currentWords_) / static_cast<float>(goalWords_), 0.0f, 1.0f);
}

void StoryState::addEmbers(int amount) noexcept {
    if (amount > 0) {
        embers_ += amount;
        if (onEmbersChanged) {
            onEmbersChanged(embers_);
        }
    }
}

bool StoryState::spendEmbers(int amount) noexcept {
    if (amount <= 0 || embers_ < amount) {
        return false;
    }
    embers_ -= amount;
    if (onEmbersChanged) {
        onEmbersChanged(embers_);
    }
    return true;
}

void StoryState::updateWordCount(size_t words) {
    currentWords_ = words;

    // Grant 1 ember for every 25 words typed
    if (currentWords_ >= lastWordCheckpoint_ + 25) {
        size_t earned = (currentWords_ - lastWordCheckpoint_) / 25;
        lastWordCheckpoint_ += earned * 25;
        addEmbers(static_cast<int>(earned * 2));
    }

    checkMilestones();
}

void StoryState::checkMilestones() {
    if (currentWords_ >= 10 && !achievements_[0].unlocked) {
        achievements_[0].unlocked = true;
        addEmbers(5);
        if (onAchievementUnlocked) onAchievementUnlocked(achievements_[0].title);
    }

    if (currentWords_ >= 250 && !achievements_[1].unlocked) {
        achievements_[1].unlocked = true;
        addEmbers(15);
        if (onAchievementUnlocked) onAchievementUnlocked(achievements_[1].title);
    }

    if (currentWords_ >= goalWords_ && !achievements_[2].unlocked) {
        achievements_[2].unlocked = true;
        addEmbers(30);
        if (onAchievementUnlocked) onAchievementUnlocked(achievements_[2].title);
        if (onMilestoneReached) onMilestoneReached();
    }
}

void StoryState::togglePlotDevice(size_t index) {
    if (index >= plotDevices_.size()) return;
    
    // Toggle active state
    bool wasActive = plotDevices_[index].active;
    for (auto& pd : plotDevices_) pd.active = false;

    if (!wasActive) {
        plotDevices_[index].active = true;
        activePrompt_ = plotDevices_[index].prompt;
    } else {
        activePrompt_.clear();
    }
}

bool StoryState::buyShopItem(size_t index) {
    if (index >= shopItems_.size()) return false;
    auto& item = shopItems_[index];
    if (item.purchased) return false;

    if (spendEmbers(item.cost)) {
        item.purchased = true;
        return true;
    }
    return false;
}

std::string StoryState::exportToMarkdown(const std::string& manuscriptText) const {
    std::ostringstream ss;
    ss << "---\n";
    ss << "title: Grimoire Manuscript\n";
    ss << "words: " << currentWords_ << "\n";
    ss << "goal: " << goalWords_ << "\n";
    ss << "embers: " << embers_ << "\n";
    ss << "candle_upgrade: " << (shopItems_[0].purchased ? "true" : "false") << "\n";
    ss << "rain_upgrade: " << (shopItems_[1].purchased ? "true" : "false") << "\n";
    ss << "nib_upgrade: " << (shopItems_[2].purchased ? "true" : "false") << "\n";
    ss << "---\n\n";
    ss << manuscriptText << "\n";
    return ss.str();
}

bool StoryState::loadFromMarkdown(const std::string& markdownContent, std::string& outManuscriptText) {
    if (markdownContent.rfind("---", 0) != 0) {
        outManuscriptText = markdownContent;
        return false;
    }

    size_t endHeader = markdownContent.find("\n---\n", 3);
    if (endHeader == std::string::npos) {
        outManuscriptText = markdownContent;
        return false;
    }

    outManuscriptText = markdownContent.substr(endHeader + 5);
    return true;
}

} // namespace grimoire::core
