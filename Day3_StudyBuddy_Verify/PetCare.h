#pragma once
#include <stdint.h>

#if __cplusplus >= 201402L
#define CARE_FN constexpr
#else
#define CARE_FN inline
#endif

// 糧食由板子發，重新整理也不會多領
struct PetCare {
    uint32_t goalMs, progressMs = 0, learnedMs = 0, lastMs = 0, revision = 0;
    uint32_t generation = 1, feeds = 0, lastFeedMs = 0;
    unsigned food = 0;
    bool initialized = false, previous = false, canFeed = false, fed = false;

    CARE_FN explicit PetCare(uint32_t goal) : goalMs(goal) {}
    CARE_FN bool tick(uint32_t now, bool learning, bool available, uint32_t gen) {
        const uint32_t dt = initialized ? now - lastMs : 0;
        initialized = true;
        lastMs = now;
        canFeed = available;
        if (generation != gen) {
            generation = gen; feeds = 0; progressMs = 0; previous = false;
            fed = false; ++revision;
        }
        bool reward = false;
        if (learning && previous && dt <= 1500) {
            if (learnedMs <= UINT32_MAX - dt) learnedMs += dt;
            progressMs += dt;
            if (progressMs >= goalMs) {
                progressMs -= goalMs;
                if (food < 99) { ++food; ++revision; reward = true; }
            }
        }
        previous = learning;
        return reward;
    }
    CARE_FN bool feed(uint32_t expected, uint32_t now) {
        if (!initialized || !canFeed || now - lastMs > 1500 || !food || expected != revision ||
            (fed && now - lastFeedMs < 4000)) return false;
        --food; ++feeds; ++revision; lastFeedMs = now; fed = true;
        return true;
    }
};
#undef CARE_FN
