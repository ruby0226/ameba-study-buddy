#include "../Day3_StudyBuddy_Verify/SceneRelations.h"
using namespace SceneRelations;

// 編譯時直接檢查，失敗就不會通過
constexpr Item person{CLASS_PERSON, 90, .20f, .05f, .80f, .85f};
constexpr Item table{CLASS_TABLE, 70, .05f, .45f, .95f, .99f};
constexpr Item weakBook{CLASS_BOOK, 17, .35f, .55f, .65f, .80f};
constexpr Item weakPhone{CLASS_PHONE, 17, .42f, .57f, .50f, .74f};
constexpr Item phone{CLASS_PHONE, 60, .42f, .57f, .50f, .74f};
constexpr Item key{CLASS_KEYBOARD, 80, .30f, .56f, .62f, .72f};
constexpr Item mouse{CLASS_MOUSE, 80, .65f, .59f, .73f, .68f};

constexpr Item emptyDesk[] = {person, table, key, mouse};
static_assert(!evaluate(emptyDesk, 4).book && !evaluate(emptyDesk, 4).phone, "context cannot invent targets");
static_assert(!evaluate(emptyDesk, 4).computer, "keyboard and mouse alone are not computer, only a laptop is");

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
constexpr Item edgeBook[] = {person, table, {CLASS_BOOK, 90, .01f, .55f, .05f, .80f}};
static_assert(evaluate(edgeBook, 3).book, "no desk zone: a clear book anywhere counts");

// 只認最大的人：背景小人旁邊的低分物品不算
constexpr Item bystander{CLASS_PERSON, 95, .02f, .10f, .18f, .45f};
constexpr Item mainUser{CLASS_PERSON, 80, .45f, .05f, .98f, .95f};
constexpr Item twoPeople[] = {bystander, mainUser};
static_assert(mainPerson(twoPeople, 2) == 1 && mainPerson(nullptr, 0) < 0, "largest person wins over higher score");
constexpr Item bystanderBook[] = {bystander, mainUser, table, {CLASS_BOOK, 17, .03f, .30f, .12f, .44f}};
static_assert(!evaluate(bystanderBook, 4).book, "weak book next to a bystander is ignored");
constexpr Item bystanderPhone[] = {bystander, mainUser, {CLASS_PHONE, 17, .06f, .20f, .12f, .32f}};
static_assert(!evaluate(bystanderPhone, 3).phone, "weak phone next to a bystander is ignored");
constexpr Item mainPhone[] = {bystander, mainUser, {CLASS_PHONE, 17, .68f, .30f, .76f, .45f}};
static_assert(evaluate(mainPhone, 3).phone && evaluate(mainPhone, 3).phoneNear, "weak phone next to the main person counts");
constexpr Item bystanderComputer[] = {mainUser, {CLASS_PERSON, 90, .0f, .40f, .30f, .99f},
    {CLASS_KEYBOARD, 80, .02f, .70f, .14f, .80f}, {CLASS_MOUSE, 80, .16f, .72f, .22f, .80f}};
static_assert(!evaluate(bystanderComputer, 4).computer, "keyboard and mouse never count as computer");
// 攤開的書被看成 50 分的筆電：不算電腦，也不擋書
constexpr Item bookAsLaptop[] = {person, table, {CLASS_BOOK, 30, .35f, .55f, .65f, .80f}, {CLASS_LAPTOP, 50, .35f, .55f, .65f, .80f}};
static_assert(!evaluate(bookAsLaptop, 4).laptop && evaluate(bookAsLaptop, 4).book, "weak laptop is not computer and does not hide the book");
constexpr Item realLaptop[] = {person, {CLASS_LAPTOP, 70, .30f, .50f, .70f, .85f}};
static_assert(evaluate(realLaptop, 2).laptop && evaluate(realLaptop, 2).computer, "clear laptop is computer");
constexpr Item keyboardConflict[] = {person, table, {CLASS_BOOK, 17, .30f, .56f, .62f, .72f}, key};
static_assert(!evaluate(keyboardConflict, 4).book, "stronger overlapping keyboard blocks book");

constexpr Item phoneContext[] = {person, table, weakPhone};
static_assert(evaluate(phoneContext, 3).phoneAssist && evaluate(phoneContext, 3).phoneNear, "weak phone supported by person and table");
constexpr Item phoneOnly[] = {weakPhone};
static_assert(!evaluate(phoneOnly, 1).phone, "weak phone alone rejected");
constexpr Item raisedPhone[] = {person, {CLASS_PHONE, 17, .42f, .22f, .50f, .36f}};
static_assert(evaluate(raisedPhone, 2).phone && evaluate(raisedPhone, 2).phoneNear, "phone near body may be above desk");
constexpr Item remoteConflict[] = {person, table, phone, {CLASS_REMOTE, 90, .42f, .57f, .50f, .74f}};
static_assert(!evaluate(remoteConflict, 4).phone, "stronger remote at same box blocks phone");
constexpr Item remoteAway[] = {person, phone, {CLASS_REMOTE, 90, .80f, .60f, .88f, .77f}};
static_assert(evaluate(remoteAway, 3).phone, "remote elsewhere does not block phone");
constexpr Item mouseConflict[] = {phone, {CLASS_MOUSE, 90, .42f, .57f, .50f, .74f}};
static_assert(!evaluate(mouseConflict, 2).phone, "stronger mouse at same box blocks phone");
constexpr Item remoteOnly[] = {person, table, {CLASS_REMOTE, 90, .42f, .57f, .50f, .74f}};
static_assert(!evaluate(remoteOnly, 3).phone, "remote cannot become phone");

constexpr Item conflictingActivity[] = {person, table, {CLASS_LAPTOP, 80, .30f, .40f, .70f, .70f}, {CLASS_BOOK, 75, .25f, .76f, .65f, .96f}};
static_assert(evaluate(conflictingActivity, 4).book && evaluate(conflictingActivity, 4).computer, "both clues preserved for ambiguous state");
constexpr Item invalidBoxes[] = {{CLASS_PHONE, 90, .5f, .5f, .4f, .7f}, {CLASS_BOOK, 90, .4f, .5f, 1.1f, .7f}};
static_assert(!evaluate(invalidBoxes, 2).phone && !evaluate(invalidBoxes, 2).book, "invalid coordinates rejected");
static_assert(!evaluate(nullptr, 0).person, "empty inference is a negative sample");
