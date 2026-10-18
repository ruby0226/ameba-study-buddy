#include "../Day3_StudyBuddy_Verify/ObjectTracks.h"
#include "../Day3_StudyBuddy_Verify/StudyActivity.h"
#include "../Day3_StudyBuddy_Verify/PetCare.h"
#include "../Day3_StudyBuddy_Verify/PetLife.h"
using namespace SceneRelations;

constexpr bool tracksMotionAndReacquires() {
    ObjectTracks t;
    Item b{CLASS_BOOK,60,.30f,.50f,.45f,.75f};
    t.observe(&b,1,100); t.observe(&b,1,600); t.observe(&b,1,1200);
    if (!t.present(CLASS_BOOK)) return false;
    const uint32_t id=t.tracks[0].id;
    b.left+=.05f; b.right+=.05f;
    t.observe(&b,1,1600);
    if (t.tracks[0].id!=id || t.tracks[0].box.left!=b.left) return false;
    t.observe(nullptr,0,2500);
    if (!t.present(CLASS_BOOK)) return false;
    t.observe(&b,1,3000);
    if (t.tracks[0].id!=id) return false;
    t.observe(nullptr,0,4500); t.observe(nullptr,0,6001);
    if (t.present(CLASS_BOOK)) return false;
    t.observe(&b,1,6200);
    return t.tracks[0].id!=id && !t.present(CLASS_BOOK);
}
constexpr bool differentObjectsCannotPoolEvidence() {
    ObjectTracks t;
    Item b{CLASS_BOOK,60,.12f,.5f,.25f,.7f};
    t.observe(&b,1,100);
    b.left=.42f;b.right=.55f;t.observe(&b,1,600);
    b.left=.72f;b.right=.85f;t.observe(&b,1,1200);
    return !t.present(CLASS_BOOK) && t.tracks[0].id!=t.tracks[1].id && t.tracks[1].id!=t.tracks[2].id;
}
constexpr bool staleInferenceAndConflicts() {
    ObjectTracks t;
    Item phone{CLASS_PHONE,45,.3f,.5f,.4f,.7f};
    t.observe(&phone,1,100);t.observe(&phone,1,600);t.observe(&phone,1,1200);
    t.expire(3201);
    if (t.present(CLASS_PHONE)) return false;
    Item collision[2] = {phone,{CLASS_REMOTE,80,.3f,.5f,.4f,.7f}};
    t.observe(collision,2,3300);t.observe(collision,2,3900);t.observe(collision,2,4500);
    return !t.present(CLASS_PHONE);
}
constexpr bool expiryDoesNotCascade() {
    ObjectTracks t;
    Item phone{CLASS_PHONE,60,.3f,.5f,.4f,.7f};
    t.observe(&phone,1,100);t.observe(&phone,1,600);t.observe(&phone,1,1200);
    t.observe(nullptr,0,2200); t.observe(nullptr,0,3200);t.observe(nullptr,0,4200);
    if (!t.present(CLASS_PHONE)) return false;
    t.expire(4201);
    return !t.present(CLASS_PHONE);
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
    t.observe(&phone,1,UINT32_MAX-1000);t.observe(&phone,1,UINT32_MAX-500);t.observe(&phone,1,50);
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
static_assert(tracksMotionAndReacquires(),"Same object keeps ID; removed object expires and must reconfirm");
static_assert(differentObjectsCannotPoolEvidence(),"Different object locations cannot share confirmation hits");
static_assert(staleInferenceAndConflicts(),"Stale inference and conflicting classes never keep tracks alive");
static_assert(expiryDoesNotCascade(),"Missed frames never extend the last real detection");
static_assert(classifyActivity(true,true,true)==ST_PHONE,"Phone takes priority");
static_assert(classifyActivity(false,true,true)==ST_STUDYING,"A book takes priority over background computer");
static_assert(classifyActivity(false,false,true)==ST_COMPUTER,"Computer-only scene stays distinct");
static_assert(classifyActivity(false,false,false)==ST_PRESENT,"No object means no reward");
static_assert(phonePausesOnlyRewards(),"Phone preserves progress without earning food or counting as rest");
static_assert(rolloverAndCapacity(),"Clock wrap and extra candidates stay bounded");
