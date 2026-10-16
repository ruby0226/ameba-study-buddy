#include "../Day3_StudyBuddy_Verify/SceneRelations.h"
using namespace SceneRelations;

// 編譯時直接檢查，失敗就不會通過
constexpr Item person{CLASS_PERSON, 90, .20f, .05f, .80f, .85f};
constexpr Item table{CLASS_TABLE, 70, .05f, .45f, .95f, .99f};
constexpr Item weakBook{CLASS_BOOK, 27, .35f, .55f, .65f, .80f};
constexpr Item weakPhone{CLASS_PHONE, 27, .42f, .57f, .50f, .74f};
constexpr Item phone{CLASS_PHONE, 60, .42f, .57f, .50f, .74f};
constexpr Item key{CLASS_KEYBOARD, 80, .30f, .56f, .62f, .72f};
constexpr Item mouse{CLASS_MOUSE, 80, .65f, .59f, .73f, .68f};

constexpr Item emptyDesk[] = {person, table, key, mouse};
static_assert(!evaluate(emptyDesk, 4).book && !evaluate(emptyDesk, 4).phone, "context cannot invent targets");
static_assert(evaluate(emptyDesk, 4).computer, "keyboard and mouse near the same person support computer");

constexpr Item bookContext[] = {person, table, weakBook};
static_assert(evaluate(bookContext, 3).bookAssist && evaluate(bookContext, 3).book, "nearby person and table support weak book");
constexpr Item noPerson[] = {table, weakBook};
static_assert(!evaluate(noPerson, 2).book, "empty desk does not support weak book");
constexpr Item noTable[] = {person, weakBook};
static_assert(!evaluate(noTable, 2).book, "weak book requires table context");
constexpr Item distantPerson[] = {{CLASS_PERSON, 90, .0f, .05f, .15f, .50f}, table, weakBook};
static_assert(!evaluate(distantPerson, 3).book, "distant person cannot support weak book");
constexpr Item strongBook[] = {{CLASS_BOOK, 60, .35f, .55f, .65f, .80f}};
static_assert(evaluate(strongBook, 1).book && !evaluate(strongBook, 1).bookAssist, "strong book does not require table model");
constexpr Item outsideBook[] = {person, table, {CLASS_BOOK, 90, .01f, .55f, .05f, .80f}};
static_assert(!evaluate(outsideBook, 3).book, "book outside desk is rejected");
constexpr Item keyboardConflict[] = {person, table, {CLASS_BOOK, 27, .30f, .56f, .62f, .72f}, key};
static_assert(!evaluate(keyboardConflict, 4).book, "stronger overlapping keyboard blocks book");

constexpr Item phoneContext[] = {person, table, weakPhone};
static_assert(evaluate(phoneContext, 3).phoneAssist && evaluate(phoneContext, 3).phoneNear, "weak phone supported by person and table");
constexpr Item phoneOnly[] = {weakPhone};
static_assert(!evaluate(phoneOnly, 1).phone, "weak phone alone rejected");
constexpr Item raisedPhone[] = {person, {CLASS_PHONE, 27, .42f, .22f, .50f, .36f}};
static_assert(evaluate(raisedPhone, 2).phone && evaluate(raisedPhone, 2).phoneNear, "phone near body may be above desk");
constexpr Item remoteConflict[] = {person, table, phone, {CLASS_REMOTE, 90, .42f, .57f, .50f, .74f}};
static_assert(!evaluate(remoteConflict, 4).phone, "stronger remote at same box blocks phone");
constexpr Item remoteAway[] = {person, phone, {CLASS_REMOTE, 90, .80f, .60f, .88f, .77f}};
static_assert(evaluate(remoteAway, 3).phone, "remote elsewhere does not block phone");
constexpr Item mouseConflict[] = {phone, {CLASS_MOUSE, 90, .42f, .57f, .50f, .74f}};
static_assert(!evaluate(mouseConflict, 2).phone, "stronger mouse at same box blocks phone");
constexpr Item remoteOnly[] = {person, table, {CLASS_REMOTE, 90, .42f, .57f, .50f, .74f}};
static_assert(!evaluate(remoteOnly, 3).phone, "remote cannot become phone");

constexpr Item loneKey[] = {person, key};
static_assert(!evaluate(loneKey, 2).computer, "keyboard alone insufficient");
constexpr Item farMouse[] = {person, key, {CLASS_MOUSE, 80, .83f, .84f, .89f, .94f}};
static_assert(!evaluate(farMouse, 3).computer, "distant mouse not a pair");
constexpr Item nobodyAtComputer[] = {key, mouse};
static_assert(!evaluate(nobodyAtComputer, 2).computer, "computer context requires person");
constexpr Item conflictingActivity[] = {person, table, key, mouse, {CLASS_BOOK, 75, .25f, .76f, .65f, .96f}};
static_assert(evaluate(conflictingActivity, 5).book && evaluate(conflictingActivity, 5).computer, "both clues preserved for ambiguous state");
constexpr Item invalidBoxes[] = {{CLASS_PHONE, 90, .5f, .5f, .4f, .7f}, {CLASS_BOOK, 90, .4f, .5f, 1.1f, .7f}};
static_assert(!evaluate(invalidBoxes, 2).phone && !evaluate(invalidBoxes, 2).book, "invalid coordinates rejected");
static_assert(!evaluate(nullptr, 0).person, "empty inference is a negative sample");
