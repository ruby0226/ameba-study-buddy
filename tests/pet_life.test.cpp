#include "../Day3_StudyBuddy_Verify/PetLife.h"
using namespace PetLife;

// 用短時間驗證整輪流程
constexpr bool deathAndRebirth() {
    Clock c(1000, 1000, 2000);
    if (!(c.tick(0, AT_DESK, true) & WELCOME)) return false;
    if (!(c.tick(1000, AT_DESK, true) & REMIND)) return false;
    if (c.tick(1500, AT_DESK, true) & REMIND) return false;
    if (c.energy != 75) return false;
    if (!(c.tick(2000, AT_DESK, true) & WEAK)) return false;
    if (!(c.tick(3000, AT_DESK, true) & DIED) || !c.dead) return false;
    c.tick(4000, AWAY, true);
    c.tick(4500, AWAY, true);
    if (c.restMs != 500 || !c.dead) return false;
    const unsigned events = c.tick(5000, AWAY, true);
    if (!(events & RESTED) || !(events & REBORN) || c.dead || c.energy != 100) return false;
    if (c.generation != 2 || c.cycles != 0 || c.workMs != 0 || c.started) return false;
    if (c.tick(6000, AWAY, true) & RESTED) return false;
    c.tick(7000, AT_DESK, true);
    return c.started && c.rests == 1;
}
constexpr bool shortBreakAndReward() {
    Clock c(1000, 1000, 2000);
    c.tick(0, AT_DESK, true); c.tick(1000, AT_DESK, true);
    c.tick(1200, AWAY, true); c.tick(1700, AWAY, true);
    c.tick(1900, AT_DESK, true);
    if (c.workMs != 1000 || c.restMs != 0 || c.cycles != 0) return false;
    c.tick(2100, AWAY, true); c.tick(3100, AWAY, true);
    return c.cycles == 1 && c.energy == 100 && c.workMs == 0 && !c.reminded;
}
constexpr bool pauseAndNoFreeRest() {
    Clock c(1000, 1000, 2000);
    c.tick(0, AT_DESK, true); c.tick(1000, AT_DESK, true);
    c.tick(2000, AT_DESK, false);
    if (!c.paused || c.energy != 100 || c.workMs != 1000) return false;
    c.tick(3000, AT_DESK, true);
    if (c.workMs != 1000) return false;
    c.tick(3200, AWAY, true); c.tick(3700, AWAY, true);
    // 看不清楚時休息不增加，但已休息的部分保留
    c.tick(3900, UNKNOWN, true);
    if (c.restMs != 500 || c.cycles != 0) return false;
    c.tick(4100, AWAY, true);
    if (c.restMs != 500) return false;
    c.tick(4600, AWAY, true);
    if (c.cycles != 1 || c.workMs != 0) return false;
    // 回座才把沒休息完的進度歸零
    Clock d(1000, 1000, 2000);
    d.tick(0, AT_DESK, true); d.tick(1000, AT_DESK, true);
    d.tick(1200, AWAY, true); d.tick(1700, AWAY, true);
    d.tick(1900, UNKNOWN, true); d.tick(2100, AT_DESK, true);
    if (d.restMs != 0) return false;
    d.tick(10000, AWAY, true);
    return d.restMs == 0 && d.paused;
}
// 死掉後離席，中途看不清楚幾次，仍能休息滿而重生
constexpr bool deadRecoversThroughFlicker() {
    Clock c(1000, 1000, 2000);
    for (uint32_t n = 0; n <= 3000; n += 500) c.tick(n, AT_DESK, true);
    if (!c.dead) return false;
    uint32_t n = 3500;
    // 每段只連續休息 300 ms 就被打斷，累積起來仍算數
    for (int i = 0; i < 3; ++i) {
        c.tick(n, AWAY, true); n += 300;
        c.tick(n, AWAY, true); n += 300;
        c.tick(n, UNKNOWN, true); n += 300;
    }
    if (!c.dead || c.restMs != 900) return false;
    c.tick(n, AWAY, true); n += 300;
    c.tick(n, AWAY, true);
    return !c.dead && c.generation == 2 && c.energy == 100;
}
constexpr bool noFarming() {
    Clock c(1000, 1000, 2000);
    c.tick(0, AWAY, true); c.tick(1000, AWAY, true);
    if (c.rests != 0) return false;
    c.tick(1200, AT_DESK, true); c.tick(1400, AT_DESK, true);
    c.tick(1600, AWAY, true); c.tick(2600, AWAY, true);
    return c.cycles == 0 && c.rests == 1 && c.workMs == 0;
}
constexpr bool rollover() {
    Clock c(1000, 1000, 2000);
    c.tick(0xffffff00U, AT_DESK, true);
    c.tick(244, AT_DESK, true);
    return c.workMs == 500 && !c.paused;
}
static_assert(deathAndRebirth(), "death, rest, rebirth and one-shot events");
static_assert(shortBreakAndReward(), "short breaks preserve debt; full cycles reward growth");
static_assert(pauseAndNoFreeRest(), "unknown, disconnection and stalls cannot punish or reward");
static_assert(deadRecoversThroughFlicker(), "brief unknown frames while away cannot block rebirth");
static_assert(noFarming(), "empty room and short visits cannot farm growth");
static_assert(rollover(), "millis wraparound preserves elapsed time");
