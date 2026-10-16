#pragma once
#include <stdint.h>

#if __cplusplus >= 201402L
#define PET_FN constexpr
#else
#define PET_FN inline
#endif

namespace PetLife {
enum Presence { UNKNOWN, AT_DESK, AWAY };
enum Event { WELCOME = 1, LEAVE = 2, REMIND = 4, WEAK = 8,
             DIED = 16, RESTED = 32, REBORN = 64 };

// 數字是遊戲設定，不是健康診斷
struct Clock {
    uint32_t remindMs, restGoalMs, decayMs;
    uint32_t workMs = 0, restMs = 0, lastMs = 0;
    unsigned energy = 100, cycles = 0, rests = 0, generation = 1, reminders = 0;
    bool initialized = false, started = false, dead = false, reminded = false, paused = true;
    Presence previous = UNKNOWN, lastKnown = UNKNOWN;

    PET_FN Clock(uint32_t remind, uint32_t rest, uint32_t decay)
        : remindMs(remind), restGoalMs(rest), decayMs(decay) {}

    // 斷線或看不清楚，不扣活力也不送休息獎勵
    PET_FN unsigned tick(uint32_t now, Presence presence, bool reliable) {
        unsigned events = 0;
        const uint32_t dt = initialized ? now - lastMs : 0;
        initialized = true;
        lastMs = now;
        paused = !reliable || presence == UNKNOWN || dt > 1500;
        if (paused) {
            previous = UNKNOWN;
            restMs = 0;
            return 0;
        }
        if (presence != lastKnown) {
            if (presence == AT_DESK) events |= WELCOME;
            else if (lastKnown == AT_DESK) events |= LEAVE;
            lastKnown = presence;
        }
        // 只算前後都確定的時間
        const uint32_t elapsed = previous == presence ? dt : 0;
        previous = presence;
        if (presence == AT_DESK) {
            started = true;
            restMs = 0;
            if (!dead) {
                const uint32_t limit = remindMs + decayMs;
                workMs += elapsed < limit - workMs ? elapsed : limit - workMs;
                if (workMs >= remindMs && !reminded) {
                    reminded = true;
                    ++reminders;
                    events |= REMIND;
                }
                const unsigned oldEnergy = energy;
                energy = workMs <= remindMs ? 100 :
                    100 - (unsigned)((uint64_t)(workMs - remindMs) * 100 / decayMs);
                if (oldEnergy > 50 && energy <= 50) events |= WEAK;
                if (!energy) { dead = true; events |= DIED; }
            }
        } else if (workMs > 0) {
            restMs += elapsed < restGoalMs - restMs ? elapsed : restGoalMs - restMs;
            if (restMs >= restGoalMs) {
                ++rests;
                events |= RESTED;
                if (dead) {
                    dead = false;
                    started = false;
                    cycles = 0;
                    ++generation;
                    events |= REBORN;
                } else if (workMs >= remindMs) {
                    ++cycles;
                }
                workMs = 0;
                energy = 100;
                reminded = false;
            }
        }
        return events;
    }
    PET_FN const char *phase() const {
        return dead ? "DEAD" : energy <= 50 ? "WEAK" : reminded ? "TIRED" : "HEALTHY";
    }
};
}
#undef PET_FN
