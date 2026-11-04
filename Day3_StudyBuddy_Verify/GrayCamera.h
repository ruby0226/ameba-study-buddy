#pragma once
#include "VideoStream.h"
#include <JPEGDEC_Libraries/JPEGDEC.h>
#include "VisualTracker.h"

// 從 JPEG 拍照通道拿一張，用 1/4 縮放解成 160×90 灰階給畫面追蹤
class GrayCamera {
public:
    explicit GrayCamera(int channel) : channel(channel) {}
    VisualTracker::Frame frame{};
    unsigned long lastMs = 0;    // 拍照加解碼花多久，方便看有沒有拖慢

    bool grab() {
        uint32_t addr = 0, len = 0;
        const unsigned long start = millis();
        Camera.getImage(channel, &addr, &len);
        if (!addr || !len) return false;
        target = &frame;
        if (!jpeg.openRAM((uint8_t *)addr, len, copy)) return false;
        jpeg.setPixelType(EIGHT_BIT_GRAYSCALE);
        const bool ok = jpeg.decode(0, 0, JPEG_SCALE_QUARTER) == 1;
        jpeg.close();
        lastMs = millis() - start;
        return ok;
    }

private:
    int channel;
    JPEGDEC jpeg;
    static VisualTracker::Frame *target;

    // 解碼器一塊一塊吐灰階像素，抄進小圖
    static int copy(JPEGDRAW *draw) {
        const uint8_t *src = (const uint8_t *)draw->pPixels;
        for (int y = 0; y < draw->iHeight; ++y) {
            const int row = draw->y + y;
            if (row < 0 || row >= VisualTracker::H) continue;
            for (int x = 0; x < draw->iWidthUsed; ++x) {
                const int col = draw->x + x;
                if (col >= 0 && col < VisualTracker::W)
                    target->px[row * VisualTracker::W + col] = src[y * draw->iWidth + x];
            }
        }
        return 1;
    }
};
VisualTracker::Frame *GrayCamera::target = nullptr;
