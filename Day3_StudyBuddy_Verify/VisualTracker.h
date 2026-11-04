#pragma once
#include <stdint.h>
#include "SceneRelations.h"

#if __cplusplus >= 201402L
#define VISUAL_FN constexpr
#else
#define VISUAL_FN inline
#endif

// YOLO 框到一次後記住那塊畫面，之後在附近找最像的位置，模型漏看也能跟著。
// 用灰階小圖比對，扣掉平均亮度，光線稍微變化也還認得。
namespace VisualTracker {
enum { W = 160, H = 90, T = 16, SEARCH = 8 };
// 平均每點差異低於這個值才算找到；樣板太平（像白牆）不追，免得到處都像
const int MATCH_MAX = 22, FLAT_MIN = 6;
// 還要像 YOLO 框到時的原始樣子，避免樣板慢慢變成衣服或背景
const int ANCHOR_MAX = 30;

struct Frame { uint8_t px[W * H]; };
struct Template {
    int8_t v[T * T]{};      // 扣掉平均後的亮度，除以 2 存，會慢慢更新
    int8_t anchor[T * T]{}; // YOLO 框到時的原始樣子，不更新
    int spread = 0;         // 原始樣子的花紋強度
    bool valid = false;
};

VISUAL_FN int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

// 把框的中間 70% 取樣成 T×T，四周常是背景
VISUAL_FN bool sample(const Frame &f, SceneRelations::Item box, int dx, int dy, int *out) {
    const float bw = (box.right - box.left) * 0.7f, bh = (box.bottom - box.top) * 0.7f;
    const float x0 = (box.left + (box.right - box.left) * 0.15f) * W + dx;
    const float y0 = (box.top + (box.bottom - box.top) * 0.15f) * H + dy;
    if (bw * W < 4 || bh * H < 4) return false;
    if (x0 < 0 || y0 < 0 || x0 + bw * W > W || y0 + bh * H > H) return false;
    int sum = 0;
    for (int j = 0; j < T; ++j)
        for (int i = 0; i < T; ++i) {
            const int x = clampi((int)(x0 + (i + 0.5f) * bw * W / T), 0, W - 1);
            const int y = clampi((int)(y0 + (j + 0.5f) * bh * H / T), 0, H - 1);
            out[j * T + i] = f.px[y * W + x];
            sum += out[j * T + i];
        }
    const int mean = sum / (T * T);
    for (int k = 0; k < T * T; ++k) out[k] -= mean;
    return true;
}

VISUAL_FN Template capture(const Frame &f, SceneRelations::Item box) {
    Template t{};
    int s[T * T]{};
    if (!sample(f, box, 0, 0, s)) return t;
    int spread = 0;
    for (int k = 0; k < T * T; ++k) {
        spread += s[k] < 0 ? -s[k] : s[k];
        t.v[k] = t.anchor[k] = (int8_t)clampi(s[k] / 2, -127, 127);
    }
    t.spread = spread / (T * T);
    t.valid = t.spread >= FLAT_MIN;
    return t;
}

struct Match { bool found; int error; SceneRelations::Item box; };

// 在原位置附近 ±SEARCH 像素找差異最小的地方
VISUAL_FN Match find(const Frame &f, const Template &t, SceneRelations::Item box) {
    Match best{false, 1 << 30, box};
    if (!t.valid) return best;
    int s[T * T]{};
    int bestDx = 0, bestDy = 0;
    for (int dy = -SEARCH; dy <= SEARCH; ++dy)
        for (int dx = -SEARCH; dx <= SEARCH; ++dx) {
            if (!sample(f, box, dx, dy, s)) continue;
            int err = 0;
            const int limit = best.error < (1 << 20) ? best.error * T * T : (1 << 30);
            for (int k = 0; k < T * T && err < limit; ++k) {
                const int d = s[k] - t.v[k] * 2;
                err += d < 0 ? -d : d;
            }
            err /= T * T;
            if (err < best.error) {
                best.error = err;
                bestDx = dx; bestDy = dy;
                best.box = box;
                best.box.left += (float)dx / W; best.box.right += (float)dx / W;
                best.box.top += (float)dy / H; best.box.bottom += (float)dy / H;
            }
        }
    best.found = best.error <= MATCH_MAX;
    if (!best.found || !sample(f, box, bestDx, bestDy, s)) return best;
    // 找到的地方花紋太淡（素色衣服、牆）或不像原始樣子，就不算
    int spread = 0, drift = 0;
    for (int k = 0; k < T * T; ++k) {
        spread += s[k] < 0 ? -s[k] : s[k];
        const int d = s[k] - t.anchor[k] * 2;
        drift += d < 0 ? -d : d;
    }
    best.found = spread * 2 >= t.spread * T * T && drift <= ANCHOR_MAX * T * T;
    return best;
}

// 找得很像時慢慢更新樣板，翻頁或角度小改也跟得上
VISUAL_FN void blend(Template &t, const Frame &f, SceneRelations::Item box) {
    int s[T * T]{};
    if (!sample(f, box, 0, 0, s)) return;
    for (int k = 0; k < T * T; ++k)
        t.v[k] = (int8_t)clampi((t.v[k] * 4 + clampi(s[k] / 2, -127, 127)) / 5, -127, 127);
}
}
#undef VISUAL_FN
