#include "../Day3_StudyBuddy_Verify/ObjectTracks.h"
#include "../Day3_StudyBuddy_Verify/StudyActivity.h"
#include "../Day3_StudyBuddy_Verify/PetCare.h"
#include "../Day3_StudyBuddy_Verify/PetLife.h"
using namespace SceneRelations;

constexpr bool highScoreLocksAtOnce() {
    ObjectTracks t;
    Item b{CLASS_BOOK,60,.30f,.50f,.45f,.75f};
    t.observe(&b,1,100);
    return t.present(CLASS_BOOK) && t.view(0,100).id;
}
constexpr bool normalScoreNeedsSecondLook() {
    ObjectTracks t;
    Item b{CLASS_BOOK,40,.30f,.50f,.45f,.75f};
    t.observe(&b,1,100);
    if (t.present(CLASS_BOOK)) return false;
    t.observe(&b,1,300);
    if (!t.present(CLASS_BOOK)) return false;
    // 兩次相隔太久，第一次的候選先過期，不能湊成確認
    ObjectTracks late;
    late.observe(&b,1,100);
    for (uint32_t n=600; n<=2100; n+=500) late.observe(nullptr,0,n);
    late.observe(&b,1,2200);
    return !late.present(CLASS_BOOK);
}
constexpr bool followsUntilGone() {
    ObjectTracks t;
    Item b{CLASS_BOOK,60,.30f,.50f,.45f,.75f};
    t.observe(&b,1,100);
    const uint32_t id=t.tracks[0].id;
    // 拿起來：分數掉到 26、離開桌面區，仍是同一本
    for (uint32_t n=200; n<=1200; n+=200) {
        b.score=26; b.top-=.05f; b.bottom-=.05f;
        t.observe(&b,1,n);
    }
    if (!t.present(CLASS_BOOK) || t.tracks[0].id!=id || inDesk(b)) return false;
    if (t.view(0,1200).trackedMs!=1100) return false;
    // 漏看 2.5 秒仍保留，超過 3 秒才消失
    for (uint32_t n=1700; n<=3700; n+=500) t.observe(nullptr,0,n);
    if (!t.present(CLASS_BOOK) || t.view(0,3700).missingMs!=2500) return false;
    t.observe(nullptr,0,4201);
    return !t.present(CLASS_BOOK);
}
constexpr bool lowScoreNewObjectIgnored() {
    ObjectTracks t;
    Item b{CLASS_BOOK,26,.30f,.10f,.45f,.30f};
    t.observe(&b,1,100); t.observe(&b,1,300); t.observe(&b,1,500);
    return !t.present(CLASS_BOOK) && !t.tracks[0].id;
}
constexpr bool edgeExitIsFastButDeskEdgeIsNot() {
    ObjectTracks side;
    Item p{CLASS_PHONE,60,.81f,.55f,.99f,.70f};
    side.observe(&p,1,100);
    side.observe(nullptr,0,600);
    if (!side.present(CLASS_PHONE)) return false;
    side.observe(nullptr,0,1100);
    if (side.present(CLASS_PHONE)) return false;
    ObjectTracks desk;
    Item b{CLASS_BOOK,60,.30f,.70f,.50f,1.0f};
    desk.observe(&b,1,100);
    for (uint32_t n=600; n<=2100; n+=500) desk.observe(nullptr,0,n);
    return desk.present(CLASS_BOOK);
}
constexpr bool differentPlacesNeverConfirmEachOther() {
    ObjectTracks t;
    Item b{CLASS_BOOK,40,.12f,.5f,.25f,.7f};
    t.observe(&b,1,100);
    b.left=.42f;b.right=.55f;t.observe(&b,1,300);
    b.left=.72f;b.right=.85f;t.observe(&b,1,500);
    return !t.present(CLASS_BOOK) && t.tracks[0].id!=t.tracks[1].id && t.tracks[1].id!=t.tracks[2].id;
}
constexpr bool staleInferenceAndConflicts() {
    ObjectTracks t;
    Item phone{CLASS_PHONE,60,.3f,.5f,.4f,.7f};
    t.observe(&phone,1,100);
    t.expire(2101);
    if (t.present(CLASS_PHONE)) return false;
    Item collision[2] = {phone,{CLASS_REMOTE,80,.3f,.5f,.4f,.7f}};
    t.observe(collision,2,2200);t.observe(collision,2,2400);t.observe(collision,2,2600);
    return !t.present(CLASS_PHONE);
}
constexpr bool lockedTrackKeepsItsBox() {
    ObjectTracks t;
    Item b{CLASS_BOOK,60,.30f,.50f,.45f,.75f};
    t.observe(&b,1,100);
    const uint32_t id=t.tracks[0].id;
    Item two[2]={{CLASS_BOOK,40,.60f,.50f,.75f,.75f},b};
    t.observe(two,2,300);
    return t.tracks[0].id==id && t.tracks[0].box.left==b.left && t.tracks[1].id && !t.tracks[1].locked;
}
constexpr bool phonePausesOnlyRewards() {
    PetCare c(3000);
    PetLife::Clock life(3000,1000,10000);
    life.tick(0,PetLife::AT_DESK,true);
    c.tick(0,earnsLearningReward(ST_STUDYING),true,1);
    c.tick(1000,earnsLearningReward(ST_STUDYING),true,1);
    life.tick(1000,PetLife::AT_DESK,true);
    for (unsigned n=2000;n<=8000;n+=1000) {
        c.tick(n,earnsLearningReward(ST_PHONE),true,1);
        life.tick(n,isSeatedState(ST_PHONE)?PetLife::AT_DESK:PetLife::AWAY,true);
    }
    if (life.reminders!=1 || life.workMs!=8000 || life.restMs) return false;
    if (c.progressMs!=1000 || c.food || !isSeatedState(ST_PHONE)) return false;
    c.tick(9000,earnsLearningReward(ST_COMPUTER),true,1);
    c.tick(10000,earnsLearningReward(ST_COMPUTER),true,1);
    c.tick(11000,earnsLearningReward(ST_COMPUTER),true,1);
    return c.food==1 && c.progressMs==0 && c.learnedMs==3000;
}
constexpr bool rolloverAndCapacity() {
    ObjectTracks t;
    Item phone{CLASS_PHONE,60,.3f,.5f,.4f,.7f};
    t.observe(&phone,1,UINT32_MAX-500);t.observe(&phone,1,50);
    if (!t.present(CLASS_PHONE)) return false;
    t.observe(nullptr,0,1050);t.observe(nullptr,0,2050);t.observe(nullptr,0,3051);
    if (t.present(CLASS_PHONE)) return false;
    Item many[20]{};
    for(unsigned i=0;i<20;++i) many[i]=phone;
    t.observe(many,20,3200);
    unsigned count=0;
    for(const auto &track:t.tracks) if(track.id)++count;
    return count==ObjectTracks::CAPACITY;
}
static_assert(highScoreLocksAtOnce(),"A confident detection is followed immediately");
static_assert(normalScoreNeedsSecondLook(),"A normal detection locks on the second look within 2 seconds");
static_assert(followsUntilGone(),"Locked object follows low score and off-desk movement until missing over 3 seconds");
static_assert(lowScoreNewObjectIgnored(),"A new low score object outside the desk never starts a track");
static_assert(edgeExitIsFastButDeskEdgeIsNot(),"Leaving by a side edge ends in 1 second; touching the desk edge does not");
static_assert(differentPlacesNeverConfirmEachOther(),"Different object locations cannot confirm each other");
static_assert(staleInferenceAndConflicts(),"Stale inference and conflicting classes never keep tracks alive");
static_assert(lockedTrackKeepsItsBox(),"A second object never steals the locked object's box");
static_assert(classifyActivity(true,true,true)==ST_PHONE,"Phone takes priority");
static_assert(classifyActivity(false,true,true)==ST_STUDYING,"A book takes priority over background computer");
static_assert(classifyActivity(false,false,true)==ST_COMPUTER,"Computer-only scene stays distinct");
static_assert(classifyActivity(false,false,false)==ST_PRESENT,"No object means no reward");
static_assert(phonePausesOnlyRewards(),"Phone preserves progress without earning food or counting as rest");
static_assert(rolloverAndCapacity(),"Clock wrap and extra candidates stay bounded");
