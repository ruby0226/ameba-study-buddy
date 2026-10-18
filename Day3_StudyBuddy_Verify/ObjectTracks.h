#pragma once
#include <stdint.h>
#include "SceneRelations.h"

#if __cplusplus >= 201402L
#define TRACK_FN constexpr
#else
#define TRACK_FN inline
#endif

// 看到就開始跟：分數高立刻鎖定，分數普通 2 秒內再看到一次就鎖定。
// 鎖定後放寬條件一路跟著，直到真的不見。仍靠 YOLO 每幀的框，不是光流或外觀追蹤。
struct ObjectTracks {
    enum { CAPACITY = 12, LOST_MS = 3000, STALE_MS = 2000, KEEP_MIN_SCORE = 25,
           LOCK_NOW_SCORE = 50, CONFIRM_MS = 2000, EXIT_MS = 1000 };
    struct Track {
        SceneRelations::Item box{0, 0, 0, 0, 0, 0};
        uint32_t id = 0, seen = 0, lockedAt = 0;
        bool locked = false;
    };
    struct View { SceneRelations::Item box; uint32_t id, trackedMs, missingMs; };
    Track tracks[CAPACITY]{};
    uint32_t nextId = 1, lastFrame = 0;
    bool received = false;

    // 新物品要過完整門檻
    TRACK_FN static bool accepted(SceneRelations::Item a, const SceneRelations::Item *items, unsigned count) {
        using namespace SceneRelations;
        if (a.type == CLASS_BOOK) return acceptBook(a, items, count);
        if (a.type == CLASS_PHONE) return acceptPhone(a, items, count);
        return (a.type == CLASS_LAPTOP || a.type == CLASS_KEYBOARD || a.type == CLASS_MOUSE) &&
            a.score >= CONTEXT_MIN_SCORE && inDesk(a);
    }
    // 鎖定後只要同類、分數不太低、沒被別的東西蓋過，離開桌面區也跟著
    TRACK_FN static bool kept(SceneRelations::Item a, const SceneRelations::Item *items, unsigned count) {
        using namespace SceneRelations;
        return valid(a) && a.score >= KEEP_MIN_SCORE && !conflicting(a, items, count);
    }
    TRACK_FN static float similarity(const Track &t, SceneRelations::Item b, uint32_t now) {
        using namespace SceneRelations;
        const Item a = t.box;
        if (a.type != b.type || !valid(b)) return 0;
        const float areaA = (a.right-a.left)*(a.bottom-a.top), areaB = (b.right-b.left)*(b.bottom-b.top);
        // 拿起來靠近鏡頭會變大，鎖定後容許大小變化多一點
        if (smaller(areaA, areaB) < larger(areaA, areaB)*(t.locked ? 0.3f : 0.45f)) return 0;
        const float iou = overlap(a,b);
        if (iou >= (t.locked ? 0.1f : 0.2f)) return 1 + iou;
        const float dx = cx(a)-cx(b), dy = cy(a)-cy(b);
        const float size = larger(a.right-a.left,a.bottom-a.top);
        float gate = t.locked ? larger(0.15f, size*0.8f) : smaller(0.12f, size*0.6f);
        // 漏看越久，可能走得越遠
        if (t.locked) gate = smaller(0.35f, gate*(1 + (float)(now - t.seen)/1000));
        const float distance = dx*dx + dy*dy;
        return distance < gate*gate ? 0.5f-distance : 0;
    }
    // 最後看到時貼著左、右或上緣，之後不見就是拿出畫面了；下緣常是桌面，不算
    TRACK_FN static bool atEdge(SceneRelations::Item a) {
        return a.left <= 0.02f || a.right >= 0.98f || a.top <= 0.02f;
    }
    TRACK_FN void expire(uint32_t now) {
        for (auto &t : tracks) {
            if (!t.id) continue;
            const uint32_t missing = now - t.seen;
            if ((received && now-lastFrame > STALE_MS) || missing > LOST_MS ||
                (!t.locked && missing > CONFIRM_MS) ||
                (t.locked && missing >= EXIT_MS && atEdge(t.box))) t = Track{};
        }
    }
    TRACK_FN void lock(Track &t, uint32_t now) {
        if (!t.locked) {t.locked = true; t.lockedAt = now;}
    }
    TRACK_FN void match(bool lockedPass, const SceneRelations::Item *items, unsigned count, uint32_t now, int *used) {
        for (unsigned i=0; i<CAPACITY; ++i) {
            Track &t = tracks[i];
            if (!t.id || t.locked != lockedPass) continue;
            int best = -1; float bestScore = 0;
            for (unsigned j=0; j<count; ++j) {
                bool taken = false;
                for (unsigned k=0; k<CAPACITY; ++k) if (used[k] == (int)j) taken = true;
                if (taken || !(t.locked ? kept(items[j],items,count) : accepted(items[j],items,count))) continue;
                const float score = similarity(t,items[j],now);
                if (score > bestScore) {bestScore=score; best=(int)j;}
            }
            used[i] = best;
            if (best < 0) continue;
            // 候選再看到一次就確認
            if (!t.locked) lock(t, now);
            t.box = items[best]; t.seen = now;
        }
    }
    TRACK_FN void observe(const SceneRelations::Item *items, unsigned count, uint32_t now) {
        expire(now);
        if (received && now == lastFrame) return;
        lastFrame = now; received = true;
        int used[CAPACITY]{};
        for (unsigned i=0; i<CAPACITY; ++i) used[i] = -1;
        // 一個框只配一個 ID；已鎖定的先挑，別被新候選搶走
        match(true, items, count, now, used);
        match(false, items, count, now, used);
        for (unsigned j=0; j<count; ++j) {
            bool taken = false;
            for (unsigned i=0; i<CAPACITY; ++i) if (used[i] == (int)j) taken = true;
            if (taken || !accepted(items[j],items,count)) continue;
            for (unsigned i=0; i<CAPACITY; ++i) {
                Track &t = tracks[i];
                if (t.id) continue;
                t = Track{}; t.box=items[j]; t.seen=now; t.id=nextId++;
                if (!nextId) nextId=1;
                if (items[j].score >= LOCK_NOW_SCORE) lock(t, now);
                used[i]=(int)j; break;
            }
        }
    }
    TRACK_FN bool present(int type) const {
        for (const auto &t : tracks) if (t.id && t.box.type==type && t.locked) return true;
        return false;
    }
    TRACK_FN View view(unsigned i, uint32_t now) const {
        const Track &t = tracks[i];
        return {t.box, t.locked ? t.id : 0, t.locked ? now - t.lockedAt : 0, now - t.seen};
    }
};
#undef TRACK_FN
