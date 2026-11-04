#include "../Day3_StudyBuddy_Verify/PetPlan.h"

constexpr bool rejectsOutOfRange() {
    PetPlan p(false);
    if (p.set(10 * 60, 5 * 60, 60 * 60, 0, 0, 0)) return false;          // 專注太短
    if (p.set(25 * 60, 60, 60 * 60, 0, 0, 0)) return false;              // 休息太短
    if (p.set(25 * 60, 5 * 60, 5 * 3600, 0, 0, 0)) return false;         // 總時間太長
    if (p.set(25 * 60, 5 * 60, 60 * 60, 3, 0, 0)) return false;          // 沒有這個目的
    if (p.set(50 * 60, 5 * 60, 40 * 60, 0, 0, 0)) return false;          // 總目標比一輪還短
    return p.phase == PetPlan::NONE && !p.active() && p.revision == 0;
}
constexpr bool startsNowAndReachesGoal() {
    PetPlan p(true);
    if (!p.set(60, 10, 60, PetPlan::MIXED, 0, 1000) || !p.running()) return false;
    p.tick(1000, true);
    for (uint32_t n = 2000; n <= 30000; n += 1000) p.tick(n, true);
    for (uint32_t n = 31000; n <= 40000; n += 1000) p.tick(n, false);      // 沒在學習不算
    if (p.learnedMs != 29000 || p.phase != PetPlan::RUNNING) return false;
    unsigned events = 0;
    for (uint32_t n = 41000; n <= 72000; n += 1000) events |= p.tick(n, true);
    return (events & PetPlan::GOAL) && p.phase == PetPlan::DONE && p.active();
}
constexpr bool delayedStart() {
    PetPlan p(true);
    if (!p.set(60, 10, 120, 0, 30, 5000) || p.active()) return false;
    if (p.startsInS(5000) != 30 || p.startsInS(20500) != 15) return false;
    p.tick(10000, true);
    if (p.active() || p.learnedMs) return false;
    const unsigned e = p.tick(35000, true);
    if (!(e & PetPlan::STARTED) || !p.running()) return false;
    p.tick(36000, true);
    return p.learnedMs == 1000;
}
constexpr bool stopAndReset() {
    PetPlan p(true);
    p.set(60, 10, 120, 0, 0, 0); p.tick(0, true); p.tick(1000, true);
    p.stop();
    if (p.active()) return false;
    p.set(90, 20, 300, 1, 0, 2000);
    return p.focusS == 90 && p.learnedMs == 0 && p.revision == 3;
}
static_assert(rejectsOutOfRange(), "plans outside the allowed ranges are rejected whole");
static_assert(startsNowAndReachesGoal(), "only effective learning counts toward the goal");
static_assert(delayedStart(), "a scheduled plan waits, then starts by itself");
static_assert(stopAndReset(), "stopping and setting a new plan starts fresh");
