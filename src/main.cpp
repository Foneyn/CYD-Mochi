/*
 * Dasai Mochi animation port for ESP32-2432S028R (CYD 2.8")
 * TFT: ILI9341, 320x240, landscape
 * Source: 772 monochrome frames, 128x64, from supplied archive.
 */
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "all_frames.h"

TFT_eSPI tft = TFT_eSPI();
static constexpr int16_t IMAGE_X = 0, IMAGE_Y = 40, IMAGE_W = 320, IMAGE_H = 160;
static constexpr uint32_t FRAME_DELAY_MS = 42; // 24 FPS
static uint16_t *frameBuffer = nullptr;

static inline bool sourcePixel(const uint8_t *bitmap, uint16_t x, uint16_t y) {
  const uint32_t bitIndex = (uint32_t)y * FRAME_WIDTH + x;
  const uint8_t value = pgm_read_byte(bitmap + (bitIndex >> 3));
  return (value & (0x80u >> (bitIndex & 7u))) != 0;
}

static void renderFrame(const uint8_t *bitmap) {
  for (int16_t y = 0; y < IMAGE_H; ++y) {
    const uint16_t sy = ((uint32_t)y * FRAME_HEIGHT) / IMAGE_H;
    const uint32_t rowOffset = (uint32_t)y * IMAGE_W;
    for (int16_t x = 0; x < IMAGE_W; ++x) {
      const uint16_t sx = ((uint32_t)x * FRAME_WIDTH) / IMAGE_W;
      frameBuffer[rowOffset + x] = sourcePixel(bitmap, sx, sy) ? TFT_WHITE : TFT_BLACK;
    }
  }
  tft.pushImage(IMAGE_X, IMAGE_Y, IMAGE_W, IMAGE_H, frameBuffer);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  frameBuffer = static_cast<uint16_t *>(malloc((size_t)IMAGE_W * IMAGE_H * sizeof(uint16_t)));
  if (!frameBuffer) {
    Serial.println("ERROR: cannot allocate RGB565 frame buffer");
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 100);
    tft.println("OUT OF RAM");
    while (true) delay(1000);
  }
  Serial.printf("CYD ILI9341 ready; %u frames, %ux%u source, 24 FPS target\n",
                (unsigned)TOTAL_FRAMES, (unsigned)FRAME_WIDTH, (unsigned)FRAME_HEIGHT);
}

void loop() {
  static uint16_t currentFrame = 0;
  static uint32_t lastFrameTime = 0;
  const uint32_t now = millis();
  if ((uint32_t)(now - lastFrameTime) >= FRAME_DELAY_MS) {
    lastFrameTime = now;
    renderFrame(frames[currentFrame]);
    currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
  }
}
