#pragma once
#include "StreamIO.h"
#include "AudioStream.h"
extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

// 音量門檻要實機量過再調，序列埠 STATE 最後三欄可看數值
#ifndef MIC_NOISE_LEVEL
#define MIC_NOISE_LEVEL 3000
#endif
#define MIC_SAMPLE_MS    20

// 只用板上麥克風聽，叫聲仍交給電腦播放
class PetSound {
public:
    volatile int level = 0, peak = 0;    // 上一秒平均與最高
    volatile bool noisy = false;

    bool begin() {
        audio.configAudio(setting);
        streamer.registerInput(audio);
        streamer.registerOutput(audio);
        audio.begin();
        if (streamer.begin() != 0) return false;
        return xTaskCreate(run, "PetSound", 2048, this, 1, NULL) == pdPASS;
    }

private:
    AudioSetting setting{0};    // 8kHz 類比麥克風
    Audio audio;
    StreamIO streamer{1, 1};

    static void run(void *arg) { static_cast<PetSound *>(arg)->listen(); }

    void listen() {
        unsigned long secondStart = millis();
        long sum = 0;
        int count = 0, secPeak = 0;
        unsigned history = 0;    // 最近 5 秒哪幾秒偏吵
        while (true) {
            const unsigned long now = millis();
            int v = audio.micLevel();
            if (v < 0) v = -v;
            sum += v; ++count;
            if (v > secPeak) secPeak = v;

            // 持續吵才算，偶爾一聲不算
            if (now - secondStart >= 1000) {
                const int avg = count ? (int)(sum / count) : 0;
                history = ((history << 1) | (avg >= MIC_NOISE_LEVEL)) & 0x1F;
                const int loud = __builtin_popcount(history);
                if (!noisy && loud >= 3) noisy = true;
                else if (noisy && loud <= 1) noisy = false;
                level = avg; peak = secPeak;
                sum = 0; count = 0; secPeak = 0;
                secondStart = now;
            }
            vTaskDelay(MIC_SAMPLE_MS / portTICK_PERIOD_MS);
        }
    }
};
