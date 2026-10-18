#pragma once
#include "WiFi.h"
#include "PetWebPage.h"
#include "PetCare.h"
extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
}

// 網路另外跑，不讓慢速瀏覽器卡住辨識
class PetNetwork {
public:
    bool begin(const char *ssid, const char *pass, unsigned long rewardMs) {
        networkName = ssid; networkPassword = pass;
        care.goalMs = rewardMs;
        bootTag = micros();
        lock = xSemaphoreCreateMutex();
        if (!lock) return false;
        return xTaskCreate(run, "PetWeb", 8192, this, 1, NULL) == pdPASS;
    }
    void publish(const char *csvLine, const char *stateLine, const char *petLine) {
        if (!lock) return;
        xSemaphoreTake(lock, portMAX_DELAY);
        snprintf(csv, sizeof(csv), "%s", csvLine);
        snprintf(state, sizeof(state), "%s", stateLine);
        snprintf(pet, sizeof(pet), "%s", petLine);
        ++sequence;
        xSemaphoreGive(lock);
    }
    void event(unsigned long id, const char *line) {
        if (!lock) return;
        xSemaphoreTake(lock, portMAX_DELAY);
        eventId = id;
        snprintf(events[id % EVENT_COUNT], sizeof(events[0]), "%s", line);
        xSemaphoreGive(lock);
    }
    bool alive() {
        if (!lock) return false;
        xSemaphoreTake(lock, portMAX_DELAY);
        const bool active = seen && millis() - heartbeat <= 3000;
        xSemaphoreGive(lock);
        return active;
    }
    bool updateCare(unsigned long now, bool learning, bool available, unsigned generation) {
        if (!lock) return false;
        xSemaphoreTake(lock, portMAX_DELAY);
        const bool reward = care.tick(now, learning, available, generation);
        xSemaphoreGive(lock);
        return reward;
    }

private:
    enum { EVENT_COUNT = 32 };
    WiFiServer server{80, TCP_MODE, NON_BLOCKING_MODE};
    SemaphoreHandle_t lock = NULL;
    char csv[700] = {}, state[300] = {}, pet[250] = {};
    char events[EVENT_COUNT][80] = {};
    unsigned long sequence = 0, eventId = 0, heartbeat = 0;
    bool seen = false;
    PetCare care{300000};
    unsigned long bootTag = 0;
    const char *networkName = NULL, *networkPassword = NULL;

    static void run(void *arg) { static_cast<PetNetwork *>(arg)->serve(); }

    // 呼叫時已拿到鎖
    String careLine() {
        char line[180];
        snprintf(line, sizeof(line), "CARE,%lu,%lu,%lu,%u,%lu,%lu,%lu,%lu,%lu,%d\n",
            bootTag, (unsigned long)care.revision, (unsigned long)care.generation, care.food,
            (unsigned long)care.feeds, (unsigned long)(care.learnedMs / 1000),
            (unsigned long)((care.goalMs - care.progressMs + 999) / 1000),
            (unsigned long)(care.goalMs / 1000), (unsigned long)care.lastFeedMs, care.canFeed);
        return String(line);
    }

    // 分段送，記得處理沒送完的部分
    bool writeAll(WiFiClient &client, const char *data, size_t length) {
        const unsigned long start = millis();
        size_t offset = 0;
        while (offset < length && millis() - start < 5000) {
            const size_t chunk = length - offset > 1024 ? 1024 : length - offset;
            const size_t sent = client.write((const uint8_t *)data + offset, chunk);
            if (!sent || sent > chunk) return false;
            offset += sent;
            vTaskDelay(1);
        }
        return offset == length;
    }
    void reply(WiFiClient &client, const char *status, const char *type, const char *body, size_t length) {
        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %lu\r\n"
            "Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nConnection: close\r\n\r\n",
            status, type, (unsigned long)length);
        if (writeAll(client, header, strlen(header))) writeAll(client, body, length);
    }
    String snapshot(long after) {
        String body;
        body.reserve(4096);
        xSemaphoreTake(lock, portMAX_DELAY);
        heartbeat = millis();
        seen = true;
        body += "NET,"; body += sequence; body += ','; body += eventId; body += ','; body += heartbeat; body += '\n';
        if (sequence) {
            body += csv; body += '\n'; body += state; body += '\n'; body += pet; body += '\n';
        }
        body += careLine();
        // 第一次只同步現況，重試才補齊短暫漏收的事件
        if (after >= 0 && (unsigned long)after <= eventId) {
            unsigned long first = (unsigned long)after + 1;
            if (eventId >= EVENT_COUNT && first <= eventId - EVENT_COUNT) {
                first = eventId - EVENT_COUNT + 1;
                body += "NOTICE,EVENT_GAP\n";
            }
            for (unsigned long id = first; id <= eventId; ++id) {
                body += events[id % EVENT_COUNT]; body += '\n';
            }
        }
        xSemaphoreGive(lock);
        return body;
    }
    void request(WiFiClient &client) {
        // 限制標頭大小和等待時間，壞連線就關掉
        char requestLine[160] = {}, line[256] = {};
        unsigned linePos = 0, total = 0;
        bool first = true, complete = false, appRequest = false;
        const unsigned long start = millis();
        while (millis() - start < 1500 && total < 2048) {
            if (!client.available()) { vTaskDelay(1); continue; }
            const int value = client.read();
            if (value < 0) continue;
            ++total;
            const char c = (char)value;
            if (c == '\r') continue;
            if (c == '\n') {
                line[linePos] = 0;
                if (first) {
                    if (linePos >= sizeof(requestLine)) return;
                    strcpy(requestLine, line); first = false;
                } else if (!linePos) { complete = true; break; }
                else if (strcmp(line, "X-StudyBuddy: 1") == 0 || strcmp(line, "x-studybuddy: 1") == 0) appRequest = true;
                linePos = 0;
            } else {
                if (linePos >= sizeof(line) - 1) return;
                line[linePos++] = c;
            }
        }
        if (!complete) return;
        char method[8], path[128], version[16];
        if (sscanf(requestLine, "%7s %127s %15s", method, path, version) != 3) return;
        if (strcmp(method, "GET") == 0 && (strcmp(path, "/") == 0 || strcmp(path, "/pet_test.html") == 0)) {
            reply(client, "200 OK", "text/html; charset=utf-8", PET_WEB_PAGE, sizeof(PET_WEB_PAGE) - 1);
        } else if (strcmp(method, "POST") == 0 && strncmp(path, "/api/poll?after=", 16) == 0 && appRequest) {
            char *end;
            const long after = strtol(path + 16, &end, 10);
            if (*end || end == path + 16 || after < -1) {
                reply(client, "400 Bad Request", "text/plain", "Bad cursor", 10); return;
            }
            const String body = snapshot(after);
            reply(client, "200 OK", "text/plain; charset=utf-8", body.c_str(), body.length());
        } else if (strcmp(method, "POST") == 0 && strncmp(path, "/api/feed?boot=", 15) == 0 && appRequest) {
            char *end;
            const unsigned long boot = strtoul(path + 15, &end, 10);
            if (end == path + 15 || strncmp(end, "&revision=", 10) != 0) {
                reply(client, "400 Bad Request", "text/plain", "Invalid request", 15); return;
            }
            const char *number = end + 10;
            const unsigned long expected = strtoul(number, &end, 10);
            if (*end || end == number) {
                reply(client, "400 Bad Request", "text/plain", "Invalid request", 15); return;
            }
            xSemaphoreTake(lock, portMAX_DELAY);
            const unsigned long now = millis();
            const bool accepted = boot == bootTag && seen && now - heartbeat <= 3000 && care.feed(expected, now);
            const String body = careLine();
            xSemaphoreGive(lock);
            reply(client, accepted ? "200 OK" : "409 Conflict", "text/plain; charset=utf-8", body.c_str(), body.length());
        } else {
            reply(client, "404 Not Found", "text/plain", "Not found", 9);
        }
    }
    void serve() {
        server.begin();
        bool online = true;
        unsigned long lastReconnect = 0;
        while (true) {
            if (WiFi.status() != WL_CONNECTED) {
                if (online) { server.stop(); online = false; }
                if (millis() - lastReconnect >= 10000) {
                    lastReconnect = millis();
                    WiFi.begin((char *)networkName, (char *)networkPassword);
                }
                vTaskDelay(100 / portTICK_PERIOD_MS); continue;
            }
            if (!online || !server.connected()) { server.begin(); online = true; }
            WiFiClient client = server.available();
            if (client) { request(client); client.stop(); }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
};
