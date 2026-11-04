#pragma once
#include <stdint.h>

#if __cplusplus >= 201402L
#define PLAN_FN constexpr
#else
#define PLAN_FN inline
#endif

// 學習計畫：開始前一定要設定專注間隔和休息時間，而且要在合理範圍內
struct PetPlan {
    enum Purpose { READING = 0, COMPUTER = 1, MIXED = 2 };
    enum Phase { NONE = 0, WAITING = 1, RUNNING = 2, DONE = 3 };
    enum Event { STARTED = 1, GOAL = 2, ENDED = 4 };
    struct Range { uint32_t minS, maxS; };
    // 一般版用分鐘等級；展示版縮短成秒，錄影才等得到
    Range focus, rest, goal;
    // revision 任何變化都加；serial 只在重新設定時加，主程式靠它知道要套用新時間
    uint32_t focusS = 0, restS = 0, goalS = 0, purpose = 0, revision = 0, serial = 0;
    uint32_t startAt = 0, learnedMs = 0, lastMs = 0;
    Phase phase = NONE;
    bool timed = false;

    PLAN_FN explicit PetPlan(bool demo)
        : focus(demo ? Range{30, 300} : Range{15 * 60, 60 * 60}),
          rest(demo ? Range{10, 120} : Range{3 * 60, 20 * 60}),
          goal(demo ? Range{60, 1800} : Range{30 * 60, 4 * 3600}) {}

    PLAN_FN static bool inside(uint32_t v, Range r) { return v >= r.minS && v <= r.maxS; }
    // 不合規就整個拒絕，不幫使用者偷偷改值
    PLAN_FN bool set(uint32_t f, uint32_t r, uint32_t g, uint32_t p, uint32_t delayS, uint32_t now) {
        if (!inside(f, focus) || !inside(r, rest) || !inside(g, goal) || p > MIXED || delayS > 12 * 3600 || g < f)
            return false;
        focusS = f; restS = r; goalS = g; purpose = p;
        startAt = now + delayS * 1000; learnedMs = 0; lastMs = now; timed = false;
        phase = delayS ? WAITING : RUNNING;
        ++serial;
        ++revision;
        return true;
    }
    PLAN_FN void stop() {
        if (phase == NONE) return;
        phase = NONE; ++revision;
    }
    PLAN_FN bool running() const { return phase == RUNNING; }
    // 達標後還是陪伴與提醒休息，只是總目標不再累加
    PLAN_FN bool active() const { return phase == RUNNING || phase == DONE; }
    // 開始後有效學習才算進總目標；達標後仍可繼續陪伴
    PLAN_FN unsigned tick(uint32_t now, bool learning) {
        unsigned events = 0;
        const uint32_t dt = timed ? now - lastMs : 0;
        timed = true;
        lastMs = now;
        if (phase == WAITING && (int32_t)(now - startAt) >= 0) { phase = RUNNING; ++revision; events |= STARTED; }
        if (phase != RUNNING) return events;
        if (learning && dt <= 1500) learnedMs += dt;
        if (learnedMs >= goalS * 1000) { phase = DONE; ++revision; events |= GOAL; }
        return events;
    }
    PLAN_FN uint32_t startsInS(uint32_t now) const {
        return phase == WAITING ? (startAt - now + 999) / 1000 : 0;
    }
};
#undef PLAN_FN
