#include "../Day3_StudyBuddy_Verify/PetCare.h"

constexpr bool earnOnlyWhileLearning() {
    PetCare c(3000);
    c.tick(0, true, true, 1);
    c.tick(1000, true, true, 1);
    c.tick(2000, false, true, 1);
    c.tick(3000, true, true, 1);
    if (c.progressMs != 1000 || c.food) return false;
    c.tick(9000, true, true, 1);
    if (c.progressMs != 1000) return false;
    c.tick(10000, true, true, 1);
    if (!c.tick(11000, true, true, 1)) return false;
    return c.food == 1 && c.learnedMs == 3000 && c.progressMs == 0;
}
constexpr bool feedOnceAndRequireFreshPresence() {
    PetCare c(1000);
    c.tick(0, true, true, 1); c.tick(1000, true, true, 1);
    c.tick(2000, true, true, 1);
    const uint32_t revision = c.revision;
    if (!c.feed(revision, 2000) || c.food != 1 || c.feeds != 1) return false;
    if (c.feed(revision, 2000) || c.feed(c.revision, 3000)) return false;
    if (c.feed(c.revision, 7000)) return false;
    c.tick(7000, false, false, 1);
    if (c.feed(c.revision, 7000)) return false;
    c.tick(7100, false, true, 1);
    if (!c.feed(c.revision, 7100) || c.food != 0 || c.feeds != 2) return false;
    return !c.feed(c.revision, 12000);
}
constexpr bool generationAndCapacity() {
    PetCare c(1000);
    for (unsigned n = 0; n <= 110; ++n) c.tick(n * 1000, true, true, 1);
    if (c.food != 99 || c.learnedMs != 110000) return false;
    c.feed(c.revision, 110000);
    c.tick(111000, true, true, 2);
    return c.food == 98 && c.feeds == 0 && c.progressMs == 0 && c.generation == 2;
}
constexpr bool uptimeWrap() {
    PetCare c(1000);
    c.tick(UINT32_MAX-499, true, true, 1);
    return c.tick(500, true, true, 1) && c.food == 1;
}
static_assert(earnOnlyWhileLearning(), "Only verified learning earns food");
static_assert(feedOnceAndRequireFreshPresence(), "No duplicate, stale, absent or empty feed");
static_assert(generationAndCapacity(), "Food capped; growth belongs to one generation");
static_assert(uptimeWrap(), "Clock rollover keeps elapsed time correct");
