// 讀書小夥伴；來源與限制見驗證文件
#include "WiFi.h"
#include "WiFiSecrets.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "RTSP.h"
#include "NNObjectDetection.h"
#include "VideoStreamOverlay.h"
#include "DetectionEvidence.h"
#include "SceneRelations.h"
#include "ObjectTracks.h"
#include "StudyActivity.h"
#include "PetLife.h"
#include "PetNetwork.h"
#include "PetSound.h"

// YOLO 漏看時用畫面比對繼續追，改成 0 就只靠 YOLO
#ifndef USE_VISUAL_TRACK
#define USE_VISUAL_TRACK 1
#endif
#if USE_VISUAL_TRACK
#include "SPI.h"
#include <JPEGDEC_Libraries/JPEGDEC.h>
#include "GrayCamera.h"
#endif
using namespace SceneRelations;
extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
}

#define CHANNEL   0
#define CHANNELNN 3
// 追蹤用的小張 JPEG，解成 160×90 灰階
#define CHANNELJPEG 1

// 送進模型的影像大小
#define NNWIDTH  576
#define NNHEIGHT 320

#define LOG_INTERVAL_MS 1000

// 拍影片才改成 1，仍然靠真實辨識
#ifndef PET_DEMO
#define PET_DEMO 0
#endif
#if PET_DEMO
PetLife::Clock petLife(60000, 10000, 30000);
#else
PetLife::Clock petLife(25UL * 60000, 5UL * 60000, 10UL * 60000);
#endif
PetNetwork petNetwork;
unsigned long petEventId = 0;
// 已套用到小雞計時的計畫編號，上一圈是否在有效學習
uint32_t appliedPlan = 0;
bool lastLearning = false;

// 麥克風只看環境音量
PetSound petSound;
bool soundReady = false, lastNoisy = false;

// 分數不夠就先不算
#define NN_CONF_THRESH   0.15f
// 坐穩 3 秒才開始，離開 10 秒才算離席
#define HOLD_MS          3000
#define AWAY_MS          10000
#define GAP_TOLERANCE_MS 1500

// 最近 5 秒要看到多少比例
#define BOOK_MIN_PERCENT   15
#define PHONE_MIN_PERCENT  25
#define RESULT_STALE_MS 2000

// 0 是全畫面，先別裁掉人
#define DESK_ROI_TOP 0.0f

// 有接燈再改成 1
#define USE_RGB_LED 0
#define LED_R_PIN   7
#define LED_G_PIN   8
#define LED_B_PIN   4

VideoSetting config(VIDEO_FHD, 30, VIDEO_H264, 0);
VideoSetting configNN(NNWIDTH, NNHEIGHT, 10, VIDEO_RGB, 0);
#if USE_VISUAL_TRACK
VideoSetting configJpeg(VisualTracker::W * 4, VisualTracker::H * 4, 10, VIDEO_JPEG, 1);
GrayCamera grayCamera(CHANNELJPEG);
bool grayReady = false;
// 每秒統計拍照成功次數、耗時與靠畫面追到的次數，序列埠看得出畫面追蹤有沒有在動
unsigned grabTries = 0, grabOk = 0, grabMsSum = 0, visualHits = 0;
#endif
NNObjectDetection ObjDet;
StreamIO videoStreamerNN(1, 1);
RTSP rtsp;
StreamIO videoStreamer(1, 1);

// Wi-Fi 放在本機設定檔
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;
int status = WL_IDLE_STATUS;

IPAddress ip;
int rtsp_portnum;

// 每秒整理一次結果
unsigned long windowStart = 0;
int samples = 0;
int personHits = 0, bookHits = 0, phoneHits = 0, laptopHits = 0;
int personMax = 0, bookMax = 0, phoneMax = 0, laptopMax = 0;
int tableHits = 0, tableMax = 0, keyboardHits = 0, keyboardMax = 0;
int mouseHits = 0, mouseMax = 0, remoteHits = 0, remoteMax = 0;
int bookAssistHits = 0, phoneAssistHits = 0;
SemaphoreHandle_t detectionMutex;
std::vector<ObjectDetectionResult> latestObjects;
unsigned long objectFrameMs = 0;
bool objectReceived = false;
bool latestPerson = false, latestBook = false, latestPhone = false, latestLaptop = false;
ObjectTracks objectTracks;
DetectionEvidence phoneNearEvidence;
bool bookStable = false, phoneStable = false, laptopStable = false;
bool phoneNearStable = false;

// 記住開始和最後看到的時間
struct Hold {
    unsigned long since = 0;
    unsigned long lastSeen = 0;
};

// 事件只送一次，網頁自己留紀錄
void petEvent(const char *name, unsigned long now)
{
    char line[80];
    snprintf(line, sizeof(line), "EVENT,%lu,%lu,%s", now, ++petEventId, name);
    Serial.println(line);
    petNetwork.event(petEventId, line);
}

const char *stateName(StudyState s)
{
    switch (s) {
        case ST_AWAY:
            return "AWAY";
        case ST_PRESENT:
            return "PRESENT";
        case ST_STUDYING:
            return "STUDYING";
        case ST_COMPUTER:
            return "COMPUTER";
        case ST_PHONE:
            return "PHONE";
        default:
            return "UNSURE";
    }
}

Hold personHold;
Hold callbackPersonHold;
StudyState state = ST_UNSURE;
unsigned long deskMs = 0;
unsigned long studyMs = 0;
unsigned long computerMs = 0;
unsigned long lastLoopMs = 0;

bool seated = false;
bool awayOpen = false;
unsigned long awayStartMs = 0;
int awayCount = 0;
unsigned long lastAwayMs = 0;

// 短暫漏看不用全部重算
void updateHold(Hold &h, bool seen, unsigned long now)
{
    if (!seen) {
        return;
    }
    if (h.since == 0 || now - h.lastSeen > GAP_TOLERANCE_MS) {
        h.since = now;
    }
    h.lastSeen = now;
}

bool isActive(const Hold &h, unsigned long now)
{
    return h.since != 0 && now - h.lastSeen <= GAP_TOLERANCE_MS;
}

bool heldFor(const Hold &h, unsigned long now, unsigned long ms)
{
    return isActive(h, now) && now - h.since >= ms;
}

Item sceneItem(ObjectDetectionResult item)
{
    // 靠近鏡頭時框會超出畫面一點，裁回來才不會整個被丟掉
    auto clip = [](float v) { return v < 0 ? 0.0f : v > 1 ? 1.0f : v; };
    return {item.type(), item.score(), clip(item.xMin()), clip(item.yMin()), clip(item.xMax()), clip(item.yMax())};
}

std::vector<Item> sceneItems(const std::vector<ObjectDetectionResult> &results)
{
    std::vector<Item> items;
    items.reserve(results.size());
    for (auto item : results) items.push_back(sceneItem(item));
    return items;
}

// 每次新辨識才算一次，先鎖住共用資料
void onObjects(std::vector<ObjectDetectionResult> results)
{
    const auto items = sceneItems(results);
    const Frame frame = evaluate(items.data(), items.size());
    xSemaphoreTake(detectionMutex, portMAX_DELAY);
    unsigned long now = millis();
    for (auto item : results) {
        int type = item.type(), score = item.score();

        // 低分也留著，方便調整
        if (type == CLASS_PERSON) personMax = std::max(personMax, score);
        else if (type == CLASS_BOOK) bookMax = std::max(bookMax, score);
        else if (type == CLASS_PHONE) phoneMax = std::max(phoneMax, score);
        else if (type == CLASS_LAPTOP) laptopMax = std::max(laptopMax, score);
        else if (type == CLASS_TABLE) tableMax = std::max(tableMax, score);
        else if (type == CLASS_KEYBOARD) keyboardMax = std::max(keyboardMax, score);
        else if (type == CLASS_MOUSE) mouseMax = std::max(mouseMax, score);
        else if (type == CLASS_REMOTE) remoteMax = std::max(remoteMax, score);
    }
    ++samples;
    personHits += frame.person; bookHits += frame.book; phoneHits += frame.phone; laptopHits += frame.laptop;
    tableHits += frame.table; keyboardHits += frame.keyboard; mouseHits += frame.mouse; remoteHits += frame.remote;
    bookAssistHits += frame.bookAssist; phoneAssistHits += frame.phoneAssist;
    updateHold(callbackPersonHold, frame.person, now);
    objectTracks.observe(items.data(), items.size(), now);
    phoneNearEvidence.observe(frame.phoneNear, now, PHONE_MIN_PERCENT);
    latestPerson = frame.person; latestBook = frame.book; latestPhone = frame.phone; latestLaptop = frame.laptop;
    latestObjects = results;
    objectFrameMs = now;
    objectReceived = true;
    xSemaphoreGive(detectionMutex);
}

// 手機優先，再看書本，有筆電就算用電腦
StudyState seatedState(void)
{
    return classifyActivity(phoneStable, bookStable, laptopStable);
}

// 先看人在不在，再決定活動
StudyState decideState(unsigned long now)
{

    if (personHold.lastSeen == 0 || now - personHold.lastSeen >= AWAY_MS) {
        return ST_AWAY;
    }
    if (heldFor(personHold, now, HOLD_MS)) {
        return seatedState();
    }
    // 已經確認離席，零星誤判成人不算回座，要穩定 3 秒才算
    if (state == ST_AWAY) {
        return ST_AWAY;
    }

    return ST_UNSURE;
}

const char *className(int type)
{
    switch (type) {
        case CLASS_PERSON:
            return "person";
        case CLASS_PHONE:
            return "phone";
        case CLASS_BOOK:
            return "book";
        case CLASS_LAPTOP:
            return "laptop";
        case CLASS_TABLE:
            return "table";
        case CLASS_REMOTE:
            return "remote";
        default:
            return NULL;
    }
}

#if USE_RGB_LED

int ledNow[3] = {0, 0, 0};

// 燈慢慢變色，不要突然閃
void updateLed(StudyState s, unsigned long now)
{
    int target[3];
    switch (s) {
        case ST_STUDYING:
            target[0] = 0;  target[1] = 120; target[2] = 0;
            break;
        case ST_COMPUTER:
            target[0] = 60; target[1] = 0; target[2] = 90;
            break;
        case ST_PHONE:
            target[0] = 120; target[1] = 45; target[2] = 0;
            break;
        case ST_UNSURE:
            target[0] = 0;  target[1] = 0;  target[2] = 120;
            break;
        case ST_AWAY: {
            int phase = (now / 50) % 120;
            int level = phase < 60 ? phase : 120 - phase;
            target[0] = level / 3; target[1] = level / 3; target[2] = level / 3;
            break;
        }
        default:
            target[0] = 20; target[1] = 20; target[2] = 20;
            break;
    }
    const int pins[3] = {LED_R_PIN, LED_G_PIN, LED_B_PIN};
    for (int i = 0; i < 3; i++) {
        int diff = target[i] - ledNow[i];
        ledNow[i] += constrain(diff, -6, 6);
        analogWrite(pins[i], ledNow[i]);
    }
}
#endif

void printRtspUrl()
{
    Serial.print("Network URL for RTSP Streaming: rtsp://");
    Serial.print(ip);
    Serial.print(":");
    Serial.println(rtsp_portnum);
}

// 開機先連網，再啟動模型
void setup()
{
    Serial.begin(115200);
    detectionMutex = xSemaphoreCreateMutex();
    if (!detectionMutex) {
        Serial.println("ERROR,detection mutex allocation failed");
        while (true) delay(1000);
    }

#if USE_RGB_LED
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    pinMode(LED_B_PIN, OUTPUT);
#endif

    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);

        delay(2000);
    }
    ip = WiFi.localIP();

    config.setBitrate(2 * 1024 * 1024);
    Camera.configVideoChannel(CHANNEL, config);
    Camera.configVideoChannel(CHANNELNN, configNN);
#if USE_VISUAL_TRACK
    Camera.configVideoChannel(CHANNELJPEG, configJpeg);
#endif
    Camera.videoInit();

    rtsp.configVideo(config);
    rtsp.begin();
    rtsp_portnum = rtsp.getPort();

    ObjDet.configVideo(configNN);
    ObjDet.setResultCallback(onObjects);

    ObjDet.configThreshold(NN_CONF_THRESH, 0.3);
    if (DESK_ROI_TOP > 0) {
        ObjDet.configRegionOfInterest(0, NNWIDTH, (int)(DESK_ROI_TOP * NNHEIGHT), NNHEIGHT);
    }

    // 只載入物件模型
    ObjDet.modelSelect(OBJECT_DETECTION, DEFAULT_YOLOV7TINY, NA_MODEL, NA_MODEL);
    ObjDet.begin();

    videoStreamer.registerInput(Camera.getStream(CHANNEL));
    videoStreamer.registerOutput(rtsp);
    if (videoStreamer.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    Camera.channelBegin(CHANNEL);

    videoStreamerNN.registerInput(Camera.getStream(CHANNELNN));
    videoStreamerNN.setStackSize();
    videoStreamerNN.setTaskPriority();
    videoStreamerNN.registerOutput(ObjDet);
    if (videoStreamerNN.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    Camera.channelBegin(CHANNELNN);
#if USE_VISUAL_TRACK
    Camera.channelBegin(CHANNELJPEG);
#endif

    OSD.configVideo(CHANNEL, config);
    OSD.begin();

    soundReady = petSound.begin();
    if (!soundReady) Serial.println("ERROR,microphone start failed");

    printRtspUrl();
    if (!petNetwork.begin(ssid, pass, PET_DEMO ? 15000 : 300000, PET_DEMO)) Serial.println("ERROR,web server task failed");
    Serial.print("Pet web: http://");
    Serial.print(ip);
    Serial.println("/");

    // CSV 給校正看，STATE 給寵物看
    Serial.println("CSV,t_s,samples,person_hits,person_max,book_hits,book_max,phone_hits,phone_max,laptop_hits,laptop_max,face_hits,face_max,face_pitch,face_samples,object_age_ms,face_age_ms,book_stable,phone_stable,laptop_stable,interval_ms,book_threshold,phone_threshold,laptop_threshold,table_hits,table_max,keyboard_hits,keyboard_max,mouse_hits,mouse_max,remote_hits,remote_max,book_assist_hits,phone_assist_hits,phone_near_person,computer_context");

    Serial.println("STATE,t_s,state,desk_s,study_s,computer_s,mode,away_count,last_away_s,head,headdown_s,remind,phone_present,phone_near_person,mic_level,mic_peak,noisy,tracks");
    windowStart = millis();
    lastLoopMs = millis();
}

// 拿一份完整結果，再更新畫面
void loop()
{
    uint16_t im_h = config.height();
    uint16_t im_w = config.width();
    xSemaphoreTake(detectionMutex, portMAX_DELAY);
    unsigned long now = millis();
    long objectAge = objectReceived ? (long)(now - objectFrameMs) : -1;
    bool objectFresh = objectAge >= 0 && objectAge <= RESULT_STALE_MS;
    std::vector<ObjectDetectionResult> results = latestObjects;
    bool hasPerson = objectFresh && latestPerson, hasBook = objectFresh && latestBook;
    bool hasPhone = objectFresh && latestPhone, hasLaptop = objectFresh && latestLaptop;
    personHold = callbackPersonHold;
    objectTracks.expire(now);
#if USE_VISUAL_TRACK
    if (grayReady) visualHits += objectTracks.follow(grayCamera.frame, now);
#endif
    phoneNearEvidence.expire(now);
    bookStable = objectFresh && objectTracks.present(CLASS_BOOK);
    phoneStable = objectFresh && objectTracks.present(CLASS_PHONE);
    laptopStable = objectFresh && objectTracks.present(CLASS_LAPTOP);
    phoneNearStable = objectFresh && phoneNearEvidence.active && phoneStable;
    ObjectTracks::View tracked[ObjectTracks::CAPACITY]{};
    for (unsigned i=0; i<ObjectTracks::CAPACITY; ++i) tracked[i] = objectTracks.view(i, now);

    // 先複製統計，解鎖後再輸出
    bool logDue = now - windowStart >= LOG_INTERVAL_MS;
    int stats[13] = {0};
    int contextStats[10] = {0};
    unsigned long intervalMs = now - windowStart;
    if (logDue) {
        int values[] = {samples, personHits, personMax, bookHits, bookMax, phoneHits, phoneMax,
                        laptopHits, laptopMax, 0, 0, -1, 0};
        for (int i = 0; i < 13; ++i) stats[i] = values[i];
        const int context[] = {tableHits, tableMax, keyboardHits, keyboardMax, mouseHits, mouseMax,
                               remoteHits, remoteMax, bookAssistHits, phoneAssistHits};
        for (int i = 0; i < 10; ++i) contextStats[i] = context[i];
        samples = personHits = bookHits = phoneHits = laptopHits = 0;
        personMax = bookMax = phoneMax = laptopMax = 0;
        tableHits = tableMax = keyboardHits = keyboardMax = mouseHits = mouseMax = remoteHits = remoteMax = 0;
        bookAssistHits = phoneAssistHits = 0;
        windowStart = now;
    }
    xSemaphoreGive(detectionMutex);

    // USB 只留除錯，不控制養成
    while (Serial.available() > 0) Serial.read();

    // 白框是有效目標，黃框是候選；人只有最大的那個是白框
    OSD.createBitmap(CHANNEL);

    if (DESK_ROI_TOP > 0) {
        OSD.drawRect(CHANNEL, 0, (int)(DESK_ROI_TOP * im_h), im_w - 1, im_h - 1, 2, OSD_COLOR_YELLOW);
    }

    const auto items = sceneItems(results);
    const int mainIndex = mainPerson(items.data(), items.size());
    for (size_t i = 0; objectFresh && i < results.size(); i++) {
        int obj_type = results[i].type();
        const char *name = className(obj_type);
        if (name == NULL) {
            continue;
        }

        ObjectDetectionResult item = results[i];
        int score = item.score();
        const Item candidate = items[i];
        bool confident = score >= scoreThreshold(obj_type);
        if (obj_type == CLASS_PERSON) confident = (int)i == mainIndex;
        if (obj_type == CLASS_BOOK) confident = acceptBook(candidate, items.data(), items.size());
        if (obj_type == CLASS_PHONE) confident = acceptPhone(candidate, items.data(), items.size());

        int xmin = (int)(item.xMin() * im_w);
        int xmax = (int)(item.xMax() * im_w);
        int ymin = (int)(item.yMin() * im_h);
        int ymax = (int)(item.yMax() * im_h);

        OSD.drawRect(CHANNEL, xmin, ymin, xmax, ymax, 3, confident ? OSD_COLOR_WHITE : OSD_COLOR_YELLOW);

        char text_str[20];
        snprintf(text_str, sizeof(text_str), "%s %d", name, score);
        OSD.drawText(CHANNEL, xmin, std::max(0, ymin - (int)OSD.getTextHeight(CHANNEL)), text_str, OSD_COLOR_CYAN);
    }

    // 鎖定的物品標 ID 和追了多久；漏掉時畫最後位置，黃框表示等它回來，不是假裝又看見。
    String trackSummary;
    for (const auto &t : tracked) {
        if (!objectFresh || !t.id) continue;
        const Item box = t.box;
        const bool held = t.missingMs > 500;
        const int left=(int)(box.left*im_w), top=(int)(box.top*im_h);
        const int right=(int)(box.right*im_w), bottom=(int)(box.bottom*im_h);
        if (held) OSD.drawRect(CHANNEL,left,top,right,bottom,2,OSD_COLOR_YELLOW);
        char label[48];
        snprintf(label,sizeof(label),"%s #%lu %lus%s",className(box.type),(unsigned long)t.id,
                 (unsigned long)(t.trackedMs/1000),held?" lost":t.visual?" vis":" track");
        OSD.drawText(CHANNEL,left,std::min(bottom,(int)im_h-(int)OSD.getTextHeight(CHANNEL)),label,
                     held?OSD_COLOR_YELLOW:OSD_COLOR_CYAN);
        // 給網頁：類別-ID-秒數-是否漏看，最多四件
        if (trackSummary.length() < 80) {
            char part[32];
            snprintf(part,sizeof(part),"%s%s-%lu-%lu-%d",trackSummary.length()?"|":"",className(box.type),
                     (unsigned long)t.id,(unsigned long)(t.trackedMs/1000),held);
            trackSummary += part;
        }
    }

    // 模型停住時先顯示無法確認
    state = objectFresh ? decideState(now) : ST_UNSURE;

    unsigned long dt = now - lastLoopMs;
    bool seatedNow = isSeatedState(state);
    if (seatedNow) {
        deskMs += dt;
        if (state == ST_STUDYING) {
            studyMs += dt;
        } else if (state == ST_COMPUTER) {
            computerMs += dt;
        }

        // 回座後算這次離開多久
        if (awayOpen) {
            lastAwayMs = personHold.since - awayStartMs;
            awayOpen = false;
        }
        seated = true;
    } else if (state == ST_AWAY && seated) {

        awayCount++;
        awayStartMs = personHold.lastSeen;
        awayOpen = true;
        seated = false;
    }

    const bool webAlive = petNetwork.alive();
    const PetLife::Presence presence = seatedNow ? PetLife::AT_DESK :
        state == ST_AWAY ? PetLife::AWAY : PetLife::UNKNOWN;
    // 網頁設定的計畫：換了新計畫就套用專注與休息時間，這一輪重新開始
    PetPlan plan(PET_DEMO);
    const unsigned planEvents = petNetwork.updatePlan(now, lastLearning, plan);
    if (plan.serial != appliedPlan) {
        appliedPlan = plan.serial;
        petLife.remindMs = plan.focusS * 1000;
        petLife.restGoalMs = plan.restS * 1000;
        // 提醒後再撐專注時間的四成才歸零，最少 15 秒
        petLife.decayMs = std::max<uint32_t>(15000, plan.focusS * 400);
        petLife.workMs = petLife.restMs = 0;
        petLife.reminded = false;
        if (!petLife.dead) petLife.energy = 100;
        petEvent("PLAN_SET", now);
    }
    if (planEvents & PetPlan::STARTED) petEvent("PLAN_START", now);
    if (planEvents & PetPlan::GOAL) petEvent("GOAL", now);
    // 沒有計畫或還沒開始就不計時；離開時手機常鎖屏，休息不需要網頁開著，在座才要網頁
    const unsigned petEvents = petLife.tick(now, presence,
        objectFresh && plan.active() && (webAlive || presence == PetLife::AWAY));
    const bool learning = earnsLearningReward(state) && !petLife.paused && !petLife.dead;
    lastLearning = learning;
    if (petNetwork.updateCare(now, learning, seatedNow && !petLife.paused && !petLife.dead && petLife.started, petLife.generation))
        petEvent("REWARD", now);
    const unsigned flags[] = {PetLife::WELCOME, PetLife::LEAVE, PetLife::REMIND,
        PetLife::WEAK, PetLife::DIED, PetLife::RESTED, PetLife::REBORN};
    const char *names[] = {"WELCOME", "LEAVE", "REMIND", "WEAK", "DIED", "RESTED", "REBORN"};
    for (unsigned i = 0; i < 7; ++i) if (petEvents & flags[i]) petEvent(names[i], now);
    lastLoopMs = now;

    // 麥克風在別的工作跑，這裡只轉成事件
    if (soundReady) {
        const bool noisy = petSound.noisy;
        if (noisy != lastNoisy) { lastNoisy = noisy; petEvent(noisy ? "NOISY" : "QUIET", now); }
    }

#if USE_RGB_LED
    updateLed(state, now);
#endif

    char status_str[64];
    if (!hasPerson && !hasBook && !hasLaptop && !hasPhone) {
        snprintf(status_str, sizeof(status_str), "No target - %s", stateName(state));
    } else {
        snprintf(status_str, sizeof(status_str), "P:%d B:%d M:%d L:%d - %s", hasPerson, hasBook, hasPhone, hasLaptop, stateName(state));
    }
    OSD.drawText(CHANNEL, 20, 20, status_str, OSD_COLOR_CYAN);
    const IPAddress liveIp = WiFi.localIP();
    char webUrl[64];
    snprintf(webUrl, sizeof(webUrl), "WEB: http://%u.%u.%u.%u/", liveIp[0], liveIp[1], liveIp[2], liveIp[3]);
    OSD.drawText(CHANNEL, 20, 60, webUrl, OSD_COLOR_CYAN);
    OSD.update(CHANNEL);

    // 每秒送給網頁一次
    if (logDue) {
        String csvLine;
        csvLine.reserve(700);
        csvLine = "CSV,"; csvLine += now / 1000;
        for (int i = 0; i < 13; ++i) { csvLine += ','; csvLine += stats[i]; }
        char tail[180], stateLine[300], petLine[250];
        // 舊欄位留空值，舊網頁也能接
        snprintf(tail, sizeof(tail), ",%ld,%ld,%d,%d,%d,%lu,%d,%d,%d", objectAge, -1L,
               bookStable, phoneStable, laptopStable, intervalMs, BOOK_MIN_SCORE, PHONE_MIN_SCORE, LAPTOP_MIN_SCORE);
        csvLine += tail;
        for (int i = 0; i < 10; ++i) { csvLine += ','; csvLine += contextStats[i]; }
        csvLine += ','; csvLine += (int)phoneNearStable;
        csvLine += ','; csvLine += (int)laptopStable;
        // 麥克風沒啟動就送 -1，網頁會顯示無資料
        snprintf(stateLine, sizeof(stateLine), "STATE,%lu,%s,%lu,%lu,%lu,%c,%d,%lu,%c,%lu,%d,%d,%d,%d,%d,%d,%s",
               now / 1000, stateName(state), deskMs / 1000, studyMs / 1000, computerMs / 1000, 'A', awayCount, lastAwayMs / 1000,
                '-', 0UL, 0, phoneStable, phoneNearStable,
                soundReady ? (int)petSound.level : -1, soundReady ? (int)petSound.peak : -1, soundReady && lastNoisy,
                trackSummary.length() ? trackSummary.c_str() : "-");
        snprintf(petLine, sizeof(petLine), "PET,%lu,%s,%u,%lu,%lu,%u,%u,%u,%u,%d,%lu,%lu,%lu,%d,%d",
               now, petLife.phase(), petLife.energy, (unsigned long)(petLife.workMs / 1000),
               (unsigned long)(petLife.restMs / 1000), petLife.cycles, petLife.rests,
               petLife.generation, petLife.reminders, petLife.paused,
               (unsigned long)(petLife.remindMs / 1000), (unsigned long)(petLife.restGoalMs / 1000),
               (unsigned long)(petLife.decayMs / 1000), PET_DEMO, petLife.started);
        petNetwork.publish(csvLine.c_str(), stateLine, petLine);
        Serial.println(csvLine); Serial.println(stateLine); Serial.println(petLine);
#if USE_VISUAL_TRACK
        char visLine[64];
        snprintf(visLine, sizeof(visLine), "VIS,%lu,%u,%u,%u,%u", now / 1000, grabOk, grabTries,
                 grabOk ? grabMsSum / grabOk : 0, visualHits);
        Serial.println(visLine);
        grabTries = grabOk = grabMsSum = visualHits = 0;
#endif
    }

#if USE_VISUAL_TRACK
    // 拍照本身會等下一張畫面，不用再 delay
    grayReady = grayCamera.grab();
    ++grabTries;
    if (grayReady) { ++grabOk; grabMsSum += grayCamera.lastMs; }
    if (!grayReady) delay(100);
#else
    delay(100);
#endif
}
