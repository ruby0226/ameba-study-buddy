#pragma once
#include "SceneRelations.h"
#include "DetectionEvidence.h"

#if __cplusplus >= 201402L
#define TRACK_FN constexpr
#else
#define TRACK_FN inline
#endif

// 用框的位置認同一件物品，不是光流追蹤。
struct ObjectTracks {
    enum { CAPACITY = 12, LOST_MS = 3000, STALE_MS = 2000 };
    struct Track {
        SceneRelations::Item box{0, 0, 0, 0, 0, 0};
        DetectionEvidence evidence{};
        uint32_t id = 0, seen = 0;
    };
    struct View { SceneRelations::Item box; uint32_t id; bool held; };
    Track tracks[CAPACITY]{};
    uint32_t nextId = 1, lastFrame = 0;
    bool received = false;
    unsigned bookPercent, phonePercent, contextPercent;

    TRACK_FN ObjectTracks(unsigned book = 15, unsigned phone = 25, unsigned context = 40)
        : bookPercent(book), phonePercent(phone), contextPercent(context) {}
    TRACK_FN unsigned percent(int type) const {
        return type == SceneRelations::CLASS_BOOK ? bookPercent : type == SceneRelations::CLASS_PHONE ? phonePercent : contextPercent;
    }
    TRACK_FN static bool accepted(SceneRelations::Item a, const SceneRelations::Item *items, unsigned count) {
        using namespace SceneRelations;
        if (a.type == CLASS_BOOK) return acceptBook(a, items, count);
        if (a.type == CLASS_PHONE) return acceptPhone(a, items, count);
        return (a.type == CLASS_LAPTOP || a.type == CLASS_KEYBOARD || a.type == CLASS_MOUSE) &&
            a.score >= CONTEXT_MIN_SCORE && inDesk(a);
    }
    TRACK_FN static float similarity(SceneRelations::Item a, SceneRelations::Item b) {
        using namespace SceneRelations;
        if (a.type != b.type || !valid(a) || !valid(b)) return 0;
        const float areaA = (a.right-a.left)*(a.bottom-a.top), areaB = (b.right-b.left)*(b.bottom-b.top);
        if (smaller(areaA, areaB) < larger(areaA, areaB)*0.45f) return 0;
        const float iou = overlap(a,b);
        if (iou >= 0.2f) return 1 + iou;
        const float dx = cx(a)-cx(b), dy = cy(a)-cy(b);
        const float gate = smaller(0.12f, larger(a.right-a.left,a.bottom-a.top)*0.6f);
        const float distance = dx*dx + dy*dy;
        return distance < gate*gate ? 0.5f-distance : 0;
    }
    TRACK_FN void expire(uint32_t now) {
        for (auto &t : tracks) {
            if (!t.id) continue;
            if ((received && now-lastFrame > STALE_MS) || now-t.seen > LOST_MS) t = Track{};
            else t.evidence.expire(now);
        }
    }
    TRACK_FN void observe(const SceneRelations::Item *items, unsigned count, uint32_t now) {
        expire(now);
        if (received && now == lastFrame) return;
        lastFrame = now; received = true;
        int used[CAPACITY]{};
        for (unsigned i=0; i<CAPACITY; ++i) used[i] = -1;
        // 一個框只配一個 ID，別把不同地方的物品湊成命中。
        for (unsigned i=0; i<CAPACITY; ++i) {
            Track &t = tracks[i];
            if (!t.id) continue;
            int best = -1; float bestScore = 0;
            for (unsigned j=0; j<count; ++j) {
                bool taken = false;
                for (unsigned k=0; k<CAPACITY; ++k) if (used[k] == (int)j) taken = true;
                if (taken || !accepted(items[j],items,count)) continue;
                const float score = similarity(t.box,items[j]);
                if (score > bestScore) {bestScore=score; best=(int)j;}
            }
            used[i] = best;
            t.evidence.observe(best >= 0,now,percent(t.box.type));
            if (best >= 0) {t.box=items[best]; t.seen=now;}
        }
        for (unsigned j=0; j<count; ++j) {
            bool taken = false;
            for (unsigned i=0; i<CAPACITY; ++i) if (used[i] == (int)j) taken = true;
            if (taken || !accepted(items[j],items,count)) continue;
            for (unsigned i=0; i<CAPACITY; ++i) {
                Track &t = tracks[i];
                if (t.id) continue;
                t = Track{}; t.box=items[j]; t.seen=now; t.id=nextId++;
                if (!nextId) nextId=1;
                t.evidence.observe(true,now,percent(t.box.type));
                used[i]=(int)j; break;
            }
        }
    }
    TRACK_FN bool present(int type) const {
        for (const auto &t : tracks) if (t.id && t.box.type==type && t.evidence.active) return true;
        return false;
    }
};
#undef TRACK_FN
