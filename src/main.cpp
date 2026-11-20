
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "all_frames.h"

TFT_eSPI tft;

static uint16_t *lineBuffer = nullptr;

static const uint32_t FRAME_DELAY = 60;

bool getPixel(const uint8_t *bitmap, uint16_t x, uint16_t y) {
    uint32_t index = (uint32_t)y * FRAME_WIDTH + x;
    uint8_t byteValue = pgm_read_byte(bitmap + index / 8);
    return (byteValue & (0x80 >> (index % 8))) != 0;
}

void drawFrame(const uint8_t *bitmap) {
    int16_t screenW = tft.width();
    int16_t screenH = tft.height();

    // Масштабирование с сохранением пропорций 2:1
    int16_t imageW = screenW;
    int16_t imageH = imageW * FRAME_HEIGHT / FRAME_WIDTH;

    if (imageH > screenH) {
        imageH = screenH;
        imageW = imageH * FRAME_WIDTH / FRAME_HEIGHT;
    }

    int16_t startX = (screenW - imageW) / 2;
    int16_t startY = (screenH - imageH) / 2;

    tft.fillScreen(TFT_BLACK);

    for (int16_t y = 0; y < imageH; y++) {
        uint16_t sourceY = (uint32_t)y * FRAME_HEIGHT / imageH;

        for (int16_t x = 0; x < imageW; x++) {
            uint16_t sourceX = (uint32_t)x * FRAME_WIDTH / imageW;

            lineBuffer[x] = getPixel(bitmap, sourceX, sourceY)
                ? TFT_WHITE : TFT_BLACK;
        }

        tft.pushImage(startX, startY + y, imageW, 1, lineBuffer);
    }
}

void setup() {
    Serial.begin(115200);

    // Подсветка CYD
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);

    delay(100);

    tft.begin();
    tft.setRotation(1);
    tft.invertDisplay(true);

    Serial.printf("Display: %d x %d\n",
                  tft.width(), tft.height());

    lineBuffer = (uint16_t*)malloc(tft.width() * sizeof(uint16_t));

    if (!lineBuffer) {
        Serial.println("RAM allocation failed!");
        tft.fillScreen(TFT_RED);
        while (true) delay(1000);
    }

    Serial.printf("Frames: %d\n", TOTAL_FRAMES);
}

void loop() {
    static uint16_t frame = 0;
    static uint32_t lastTime = 0;

    if (millis() - lastTime >= FRAME_DELAY) {
        lastTime = millis();

        drawFrame(frames[frame]);

        frame++;
        if (frame >= TOTAL_FRAMES) {
            frame = 0;
        }
    }
}
