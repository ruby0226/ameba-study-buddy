#pragma once
#include <stdint.h>

#if __cplusplus >= 201402L
#define EVIDENCE_FN constexpr
#else
#define EVIDENCE_FN inline
#endif

// 多看幾次才算，短暫漏掉先保留
struct DetectionEvidence {
    struct Bucket { uint32_t start = 0; unsigned samples = 0; unsigned hits = 0; };
    Bucket buckets[5]{};
    uint32_t lastFrame = 0, lastHit = 0, evidenceSince = 0;
    bool received = false, active = false, seeded = false;

    // 太舊的結果就清掉
    EVIDENCE_FN void expire(uint32_t now) {
        for (auto &b : buckets) {
            if (b.samples && now - b.start >= 5000) b = Bucket();
        }
        if (received && now - lastFrame > 2000) {
            for (auto &b : buckets) b = Bucket();
            active = false;
            seeded = false;
        }
        if (now - lastHit > 3000) { active = false; seeded = false; }
    }

    // 只放新推論，至少跨 1 秒才成立
    EVIDENCE_FN void observe(bool hit, uint32_t now, unsigned minPercent) {
        expire(now);
        received = true;
        lastFrame = now;
        uint32_t second = now - now % 1000;
        Bucket &b = buckets[(now / 1000) % 5];
        if (!b.samples || b.start != second) {
            b = Bucket();
            b.start = second;
        }
        ++b.samples;
        if (hit) {
            if (!seeded) { evidenceSince = now; seeded = true; }
            ++b.hits;
            lastHit = now;
        }
        unsigned samples = 0, hits = 0, hitSeconds = 0;
        for (const auto &entry : buckets) {
            samples += entry.samples;
            hits += entry.hits;
            hitSeconds += entry.hits > 0;
        }
        if (hit && now - evidenceSince >= 1000 && hits >= 3 && hitSeconds >= 2 && hits * 100 >= samples * minPercent)
            active = true;
    }
};
#undef EVIDENCE_FN
