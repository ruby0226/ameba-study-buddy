#pragma once

// 類別編號照模型，不自己換名字
namespace SceneRelations {
enum { CLASS_PERSON = 0, CLASS_TABLE = 60, CLASS_LAPTOP = 63,
       CLASS_MOUSE = 64, CLASS_REMOTE = 65, CLASS_KEYBOARD = 66,
       CLASS_PHONE = 67, CLASS_BOOK = 73 };
// 書和手機 YOLO 分數常偏低，門檻放低靠追蹤和兩次確認擋誤判
const int MIN_SCORE = 50, BOOK_MIN_SCORE = 20, PHONE_MIN_SCORE = 25;
const int LAPTOP_MIN_SCORE = 40, CONTEXT_MIN_SCORE = 40, ASSIST_MIN_SCORE = 15;

struct Item { int type, score; float left, top, right, bottom; };
struct Frame {
    bool person = false, book = false, phone = false, laptop = false;
    bool table = false, keyboard = false, mouse = false, remote = false;
    bool bookAssist = false, phoneAssist = false, phoneNear = false, computer = false;
};

// 新編譯器也能直接算測試案例
#if __cplusplus >= 201402L
#define SCENE_FN constexpr
#else
#define SCENE_FN inline
#endif

SCENE_FN float smaller(float a, float b) { return a < b ? a : b; }
SCENE_FN float larger(float a, float b) { return a > b ? a : b; }
SCENE_FN float cx(Item a) { return (a.left + a.right) * 0.5f; }
SCENE_FN float cy(Item a) { return (a.top + a.bottom) * 0.5f; }
SCENE_FN float area(Item a) { return (a.right - a.left) * (a.bottom - a.top); }
SCENE_FN bool valid(Item a) {
    return a.left >= 0 && a.top >= 0 && a.right <= 1 && a.bottom <= 1 &&
           a.right > a.left && a.bottom > a.top;
}
SCENE_FN int scoreThreshold(int type) {
    return type == CLASS_PERSON ? MIN_SCORE : type == CLASS_BOOK ? BOOK_MIN_SCORE :
           type == CLASS_PHONE ? PHONE_MIN_SCORE : CONTEXT_MIN_SCORE;
}
SCENE_FN float overlap(Item a, Item b) {
    const float w = larger(0, smaller(a.right, b.right) - larger(a.left, b.left));
    const float h = larger(0, smaller(a.bottom, b.bottom) - larger(a.top, b.top));
    const float total = area(a) + area(b) - w * h;
    return total > 0 ? w * h / total : 0;
}

// 桌子只當輔助，不要求每次都看到
SCENE_FN bool onTable(Item a, const Item *items, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        const Item t = items[i];
        if (t.type == CLASS_TABLE && t.score >= CONTEXT_MIN_SCORE && valid(t) &&
            cx(a) >= t.left && cx(a) <= t.right && cy(a) >= t.top && cy(a) <= t.bottom)
            return true;
    }
    return false;
}

// 好幾個人時只認框最大的那個，背景路過的人不算
SCENE_FN int mainPerson(const Item *items, unsigned count) {
    int best = -1;
    for (unsigned i = 0; i < count; ++i) {
        const Item p = items[i];
        if (p.type == CLASS_PERSON && p.score >= MIN_SCORE && valid(p) &&
            (best < 0 || area(p) > area(items[best]))) best = (int)i;
    }
    return best;
}
SCENE_FN bool nearPerson(Item a, Item p) {
    const float margin = (p.right - p.left) * 0.12f;
    return p.type == CLASS_PERSON && p.score >= MIN_SCORE && valid(p) &&
           cx(a) >= p.left - margin && cx(a) <= p.right + margin &&
           cy(a) >= p.top + (p.bottom - p.top) * 0.18f &&
           cy(a) <= p.bottom + (p.bottom - p.top) * 0.25f;
}
SCENE_FN bool nearMainPerson(Item a, const Item *items, unsigned count) {
    const int i = mainPerson(items, count);
    return i >= 0 && nearPerson(a, items[i]);
}

// 同一塊區域像別的東西，就先別硬算
SCENE_FN bool conflicting(Item a, const Item *items, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        const Item other = items[i];
        const bool rival = a.type == CLASS_BOOK
            ? (other.type == CLASS_KEYBOARD || other.type == CLASS_LAPTOP || other.type == CLASS_PHONE)
            : (other.type == CLASS_REMOTE || other.type == CLASS_MOUSE);
        if (rival && valid(other) && other.score >= CONTEXT_MIN_SCORE &&
            other.score >= a.score + 10 && overlap(a, other) >= 0.45f) return true;
    }
    return false;
}

SCENE_FN bool acceptBook(Item a, const Item *items, unsigned count) {
    if (a.type != CLASS_BOOK || !valid(a) || conflicting(a, items, count)) return false;
    return a.score >= BOOK_MIN_SCORE || (a.score >= ASSIST_MIN_SCORE &&
           nearMainPerson(a, items, count) && onTable(a, items, count));
}
// 分數低時要在主角身邊，拿在手上或放桌上都可以
SCENE_FN bool acceptPhone(Item a, const Item *items, unsigned count) {
    if (a.type != CLASS_PHONE || !valid(a) || conflicting(a, items, count)) return false;
    return a.score >= PHONE_MIN_SCORE ||
           (a.score >= ASSIST_MIN_SCORE && nearMainPerson(a, items, count));
}

// 主角附近的鍵盤加滑鼠，才算電腦線索
SCENE_FN bool computerPair(Item key, const Item *items, unsigned count) {
    if (!nearMainPerson(key, items, count)) return false;
    for (unsigned i = 0; i < count; ++i) {
        const Item mouse = items[i];
        if (mouse.type != CLASS_MOUSE || mouse.score < CONTEXT_MIN_SCORE || !valid(mouse)) continue;
        const float dx = cx(key) - cx(mouse), dy = cy(key) - cy(mouse);
        if (dx * dx + dy * dy <= 0.30f * 0.30f && nearMainPerson(mouse, items, count)) return true;
    }
    return false;
}

SCENE_FN Frame evaluate(const Item *items, unsigned count) {
    Frame f{};
    f.person = mainPerson(items, count) >= 0;
    for (unsigned i = 0; i < count; ++i) {
        const Item a = items[i];
        if (!valid(a)) continue;
        if (acceptBook(a, items, count)) {
            f.book = true;
            f.bookAssist = f.bookAssist || a.score < BOOK_MIN_SCORE;
        }
        if (acceptPhone(a, items, count)) {
            f.phone = true;
            f.phoneAssist = f.phoneAssist || a.score < PHONE_MIN_SCORE;
            f.phoneNear = f.phoneNear || nearMainPerson(a, items, count);
        }
        if (a.score < CONTEXT_MIN_SCORE) continue;
        if (a.type == CLASS_TABLE) f.table = true;
        if (a.type == CLASS_LAPTOP) f.laptop = true;
        if (a.type == CLASS_REMOTE) f.remote = true;
        if (a.type == CLASS_MOUSE) f.mouse = true;
        if (a.type == CLASS_KEYBOARD) {
            f.keyboard = true;
            f.computer = f.computer || computerPair(a, items, count);
        }
    }
    return f;
}
#undef SCENE_FN
}
