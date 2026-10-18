#pragma once
#include "SceneRelations.h"
#include "DetectionEvidence.h"

#if __cplusplus >= 201402L
#define TRACK_FN constexpr
#else
#define TRACK_FN inline
#endif

// 先多看幾次才鎖定；鎖定後放寬條件一路跟著，直到真的不見。
// 仍靠 YOLO 每幀的框，不是光流或外觀追蹤。
struct ObjectTracks {
    enum { CAPACITY = 12, LOST_MS = 3000, STALE_MS = 2000, KEEP_MIN_SCORE = 20,
           PREDICT_MS = 1000, EXIT_MS = 300 };
    struct Track {
        SceneRelations::Item box{0, 0, 0, 0, 0, 0};
        DetectionEvidence evidence{};
        uint32_t id = 0, seen = 0, lockedAt = 0;
        float vx = 0, vy = 0; // 每毫秒移動多少畫面比例
        bool locked = false;
    };
    struct View { SceneRelations::Item box; uint32_t id, trackedMs, missingMs; };
    Track tracks[CAPACITY]{};
    uint32_t nextId = 1, lastFrame = 0;
    bool received = false;
    unsigned bookPercent, phonePercent, contextPercent;

    TRACK_FN ObjectTracks(unsigned book = 15, unsigned phone = 25, unsigned context = 40)
        : bookPercent(book), phonePercent(phone), contextPercent(context) {}
    TRACK_FN unsigned percent(int type) const {
        return type == SceneRelations::CLASS_BOOK ? bookPercent : type == SceneRelations::CLASS_PHONE ? phonePercent : contextPercent;
    }
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
    // 依上次速度推估現在的位置
    TRACK_FN static SceneRelations::Item predicted(const Track &t, uint32_t now) {
        // 只往前推一秒，推太遠就不準了
        const uint32_t ahead = now - t.seen < (uint32_t)PREDICT_MS ? now - t.seen : (uint32_t)PREDICT_MS;
        const float dt = (float)ahead;
        // 框本身會抖，太慢的移動當成沒動
        const float still = 0.0003f;
        const float vx = t.vx < still && t.vx > -still ? 0 : t.vx;
        const float vy = t.vy < still && t.vy > -still ? 0 : t.vy;
        SceneRelations::Item p = t.box;
        p.left += vx*dt; p.right += vx*dt; p.top += vy*dt; p.bottom += vy*dt;
        return p;
    }
    TRACK_FN static float similarity(const Track &t, SceneRelations::Item b, uint32_t now) {
        using namespace SceneRelations;
        const Item a = t.locked ? predicted(t, now) : t.box;
        if (a.type != b.type || !valid(b) || a.right <= a.left || a.bottom <= a.top) return 0;
        const float areaA = (a.right-a.left)*(a.bottom-a.top), areaB = (b.right-b.left)*(b.bottom-b.top);
        // 拿起來靠近鏡頭會變大，鎖定後容許大小變化多一點
        if (smaller(areaA, areaB) < larger(areaA, areaB)*(t.locked ? 0.3f : 0.45f)) return 0;
        const float iou = overlap(a,b);
        if (iou >= 0.2f) return 1 + iou;
        const float dx = cx(a)-cx(b), dy = cy(a)-cy(b);
        float gate = smaller(0.12f, larger(a.right-a.left,a.bottom-a.top)*0.6f);
        // 漏看越久，可能走得越遠
        if (t.locked) gate = smaller(0.25f, gate*(1 + (float)(now - t.seen)/1000));
        const float distance = dx*dx + dy*dy;
        return distance < gate*gate ? 0.5f-distance : 0;
    }
    TRACK_FN static bool leftFrame(const Track &t, uint32_t now) {
        using namespace SceneRelations;
        const Item p = predicted(t, now);
        return cx(p) < 0 || cx(p) > 1 || cy(p) < 0 || cy(p) > 1;
    }
    TRACK_FN void expire(uint32_t now) {
        for (auto &t : tracks) {
            if (!t.id) continue;
            if ((received && now-lastFrame > STALE_MS) || now-t.seen > LOST_MS) t = Track{};
            // 往畫面外移動又沒再看到，就當它離開了
            else if (t.locked && now - t.seen >= EXIT_MS && leftFrame(t, now)) t = Track{};
            else if (!t.locked) t.evidence.expire(now);
        }
    }
    TRACK_FN void follow(Track &t, SceneRelations::Item b, uint32_t now) {
        using namespace SceneRelations;
        const uint32_t dt = now - t.seen;
        if (dt > 0 && dt <= 1000) {
            float vx = (cx(b)-cx(t.box))/dt, vy = (cy(b)-cy(t.box))/dt;
            const float cap = 0.002f; // 半秒最多跨過整個畫面
            vx = vx > cap ? cap : vx < -cap ? -cap : vx;
            vy = vy > cap ? cap : vy < -cap ? -cap : vy;
            t.vx = (t.vx + vx)*0.5f; t.vy = (t.vy + vy)*0.5f;
        } else {
            t.vx = t.vy = 0;
        }
        t.box = b; t.seen = now;
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
            if (!t.locked) {
                t.evidence.observe(best >= 0,now,percent(t.box.type));
                if (t.evidence.active) {t.locked = true; t.lockedAt = now;}
            }
            if (best >= 0) follow(t, items[best], now);
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
                t.evidence.observe(true,now,percent(t.box.type));
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
