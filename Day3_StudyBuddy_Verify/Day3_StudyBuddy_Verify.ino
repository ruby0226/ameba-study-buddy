// 讀書小夥伴；來源與限制見驗證文件
#include "WiFi.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "RTSP.h"
#include "NNObjectDetection.h"
#include "VideoStreamOverlay.h"
#include "DetectionEvidence.h"
#include "SceneRelations.h"
using namespace SceneRelations;
extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
}

#define CHANNEL   0
#define CHANNELNN 3

// 送進模型的影像大小
#define NNWIDTH  576
#define NNHEIGHT 320

#define LOG_INTERVAL_MS 1000

// 分數不夠就先不算
#define NN_CONF_THRESH   0.25f
// 坐穩 3 秒才開始，離開 10 秒才算離席
#define HOLD_MS          3000
#define AWAY_MS          10000
#define GAP_TOLERANCE_MS 1500

// 最近 5 秒要看到多少比例
#define BOOK_MIN_PERCENT   15
#define PHONE_MIN_PERCENT  25
#define LAPTOP_MIN_PERCENT 40
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
NNObjectDetection ObjDet;
StreamIO videoStreamerNN(1, 1);
RTSP rtsp;
StreamIO videoStreamer(1, 1);

// 填入自己的 Wi-Fi（2.4 GHz）
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
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
DetectionEvidence bookEvidence, phoneEvidence, laptopEvidence;
DetectionEvidence phoneNearEvidence, computerEvidence;
bool bookStable = false, phoneStable = false, laptopStable = false;
bool phoneNearStable = false, computerStable = false;

// 寵物會用到的狀態
enum StudyState {
    ST_UNSURE,
    ST_AWAY,
    ST_PRESENT,
    ST_STUDYING,
    ST_COMPUTER
};

// 記住開始和最後看到的時間
struct Hold {
    unsigned long since = 0;
    unsigned long lastSeen = 0;
};

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
    return {item.type(), item.score(), item.xMin(), item.yMin(), item.xMax(), item.yMax()};
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
    bookEvidence.observe(frame.book, now, BOOK_MIN_PERCENT);
    phoneEvidence.observe(frame.phone, now, PHONE_MIN_PERCENT);
    laptopEvidence.observe(frame.laptop, now, LAPTOP_MIN_PERCENT);
    phoneNearEvidence.observe(frame.phoneNear, now, PHONE_MIN_PERCENT);
    computerEvidence.observe(frame.computer, now, LAPTOP_MIN_PERCENT);
    latestPerson = frame.person; latestBook = frame.book; latestPhone = frame.phone; latestLaptop = frame.laptop;
    latestObjects = results;
    objectFrameMs = now;
    objectReceived = true;
    xSemaphoreGive(detectionMutex);
}

// 線索打架就先只算在桌前
StudyState seatedState(void)
{
    const bool computer = laptopStable || computerStable;
    if (bookStable && computer) return ST_PRESENT;
    if (bookStable) return ST_STUDYING;
    if (computer) return ST_COMPUTER;
    return ST_PRESENT;
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

    return ST_UNSURE;
}

bool isSeatedState(StudyState s)
{
    return s == ST_PRESENT || s == ST_STUDYING || s == ST_COMPUTER;
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
        case CLASS_KEYBOARD:
            return "keyboard";
        case CLASS_MOUSE:
            return "mouse";
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

    OSD.configVideo(CHANNEL, config);
    OSD.begin();

    printRtspUrl();

    // CSV 給校正看，STATE 給寵物看
    Serial.println("CSV,t_s,samples,person_hits,person_max,book_hits,book_max,phone_hits,phone_max,laptop_hits,laptop_max,face_hits,face_max,face_pitch,face_samples,object_age_ms,face_age_ms,book_stable,phone_stable,laptop_stable,interval_ms,book_threshold,phone_threshold,laptop_threshold,table_hits,table_max,keyboard_hits,keyboard_max,mouse_hits,mouse_max,remote_hits,remote_max,book_assist_hits,phone_assist_hits,phone_near_person,computer_context");

    Serial.println("STATE,t_s,state,desk_s,study_s,computer_s,mode,away_count,last_away_s,head,headdown_s,remind,phone_present,phone_near_person");
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
    bookEvidence.expire(now); phoneEvidence.expire(now); laptopEvidence.expire(now);
    phoneNearEvidence.expire(now); computerEvidence.expire(now);
    bookStable = objectFresh && bookEvidence.active;
    phoneStable = objectFresh && phoneEvidence.active;
    laptopStable = objectFresh && laptopEvidence.active;
    phoneNearStable = objectFresh && phoneNearEvidence.active && phoneStable;
    computerStable = objectFresh && computerEvidence.active;

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

    // 舊網頁傳的切換指令直接丟掉
    while (Serial.available() > 0) Serial.read();

    // 白框是有效目標，黃框是候選
    OSD.createBitmap(CHANNEL);
    OSD.drawRect(CHANNEL, (int)(DESK_LEFT * im_w), (int)(DESK_TOP * im_h),
                 (int)(DESK_RIGHT * im_w), (int)(DESK_BOTTOM * im_h), 2, OSD_COLOR_CYAN);

    if (DESK_ROI_TOP > 0) {
        OSD.drawRect(CHANNEL, 0, (int)(DESK_ROI_TOP * im_h), im_w - 1, im_h - 1, 2, OSD_COLOR_YELLOW);
    }

    const auto items = sceneItems(results);
    for (size_t i = 0; objectFresh && i < results.size(); i++) {
        int obj_type = results[i].type();
        const char *name = className(obj_type);
        if (name == NULL) {
            continue;
        }

        ObjectDetectionResult item = results[i];
        int score = item.score();
        const Item candidate = items[i];
        bool confident = score >= scoreThreshold(obj_type) &&
            (obj_type == CLASS_PERSON || obj_type == CLASS_TABLE || inDesk(candidate));
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

    lastLoopMs = now;

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
    OSD.update(CHANNEL);

    // 每秒送給網頁一次
    if (logDue) {
        printf("CSV,%lu", now / 1000);
        for (int i = 0; i < 13; ++i) printf(",%d", stats[i]);
        // 舊欄位留空值，舊網頁也能接
        printf(",%ld,%ld,%d,%d,%d,%lu,%d,%d,%d", objectAge, -1L,
               bookStable, phoneStable, laptopStable, intervalMs, BOOK_MIN_SCORE, PHONE_MIN_SCORE, LAPTOP_MIN_SCORE);
        for (int i = 0; i < 10; ++i) printf(",%d", contextStats[i]);
        printf(",%d,%d\r\n", phoneNearStable, computerStable);
        printf("STATE,%lu,%s,%lu,%lu,%lu,%c,%d,%lu,%c,%lu,%d,%d,%d\r\n",
               now / 1000, stateName(state), deskMs / 1000, studyMs / 1000, computerMs / 1000, 'A', awayCount, lastAwayMs / 1000,
                '-', 0UL, 0, phoneStable, phoneNearStable);
    }

    delay(100);
}
