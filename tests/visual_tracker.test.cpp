#include "../Day3_StudyBuddy_Verify/ObjectTracks.h"
using SceneRelations::Item; using SceneRelations::CLASS_BOOK;
using Gray = VisualTracker::Frame;

// 灰色桌面上放一塊有花紋的物品，左上角在 (x, y)
constexpr Gray scene(int x, int y, bool object = true, int light = 0) {
    Gray f{};
    for (int j = 0; j < VisualTracker::H; ++j)
        for (int i = 0; i < VisualTracker::W; ++i) {
            int v = 120 + light;
            const int u = i - x, w = j - y;
            if (object && u >= 0 && u < 32 && w >= 0 && w < 24)
                v = (((u / 4) + (w / 3)) % 3) * 50 + 40 + light + (u * w) % 7;
            f.px[j * VisualTracker::W + i] = (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v);
        }
    return f;
}
// 物品在 (60,50)，對應的正規化框
constexpr Item box{CLASS_BOOK, 60, 60.f/160, 50.f/90, 92.f/160, 74.f/90};

constexpr bool findsShiftedObject() {
    const Gray a = scene(60, 50);
    const VisualTracker::Template t = VisualTracker::capture(a, box);
    if (!t.valid) return false;
    const VisualTracker::Match m = VisualTracker::find(scene(65, 47, true, 15), t, box);
    const float dx = (m.box.left - box.left) * 160, dy = (m.box.top - box.top) * 90;
    return m.found && dx > 4.5f && dx < 5.5f && dy > -3.5f && dy < -2.5f;
}
constexpr bool removedObjectNotFound() {
    const VisualTracker::Template t = VisualTracker::capture(scene(60, 50), box);
    return !VisualTracker::find(scene(0, 0, false), t, box).found;
}
constexpr bool flatAreaNotTracked() {
    return !VisualTracker::capture(scene(0, 0, false), box).valid;
}
// YOLO 只框到一次，之後靠畫面跟著；拿走後 3 秒消失
constexpr bool tracksWithoutYolo() {
    ObjectTracks t;
    t.observe(&box, 1, 100);
    t.follow(scene(60, 50), 150);
    int x = 60;
    for (uint32_t n = 600; n <= 2600; n += 500) {
        t.observe(nullptr, 0, n);
        x += 1;
        t.follow(scene(x, 50), n);
    }
    if (!t.present(CLASS_BOOK) || !t.view(0, 2600).visual) return false;
    const float moved = (t.tracks[0].box.left - box.left) * 160;
    if (moved < 4.5f || moved > 5.5f) return false;
    for (uint32_t n = 3100; n <= 5100; n += 500) { t.observe(nullptr, 0, n); t.follow(scene(0, 0, false), n); }
    if (!t.present(CLASS_BOOK)) return false;
    t.observe(nullptr, 0, 5700); t.follow(scene(0, 0, false), 5700);
    return !t.present(CLASS_BOOK);
}
// 畫面一直比對得到，YOLO 超過 5 秒沒再看到也要放棄，免得黏在背景上
constexpr bool visualOnlyGivesUp() {
    ObjectTracks t;
    t.observe(&box, 1, 100);
    t.follow(scene(60, 50), 150);
    for (uint32_t n = 600; n <= 5100; n += 500) { t.observe(nullptr, 0, n); t.follow(scene(60, 50), n); }
    if (!t.present(CLASS_BOOK)) return false;
    t.observe(nullptr, 0, 5200); t.follow(scene(60, 50), 5200);
    return !t.present(CLASS_BOOK);
}
static_assert(visualOnlyGivesUp(), "Visual-only tracking ends 5 s after the last YOLO hit");
static_assert(findsShiftedObject(), "Template finds the object after it moves and the light changes");
static_assert(removedObjectNotFound(), "Plain desk does not match a removed object");
static_assert(flatAreaNotTracked(), "Featureless areas are never used as templates");
static_assert(tracksWithoutYolo(), "One YOLO hit is enough to follow by appearance until the object is gone");
