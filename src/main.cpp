#include <Arduino.h>
#include "DisplayManager.h"
#include "EncoderTask.h"
#include "SystemTask.h"
#include "BuzzerTask.h"
#include "RfidTask.h"

constexpr uint8_t LCD_ADDRESS       = 0x27;
constexpr uint8_t ENCODER_ADDRESS   = 0x3F;
constexpr uint8_t ENCODER_INT_PIN   = 27;
constexpr uint8_t RFID_SS_PIN       = 32;
constexpr uint8_t RFID_RST_PIN      = 14;
constexpr uint8_t BUZZER_PIN        = 33;

void setup() {
    Serial.begin(115200);

    initBuzzer(BUZZER_PIN);
    initDisplay(LCD_ADDRESS, 16, 2);
    initEncoder(ENCODER_ADDRESS, ENCODER_INT_PIN);
    initRFID(RFID_SS_PIN, RFID_RST_PIN);

    xTaskCreate(vBuzzerTask, "BuzzerTask", 2048, nullptr, 2, nullptr);
    xTaskCreate(vRfidTask, "RfidTask", 3072, nullptr, 1, nullptr);
    xTaskCreate(vEncoderTask, "ReadEncoderTask", 4096, nullptr, 1, nullptr);
    xTaskCreate(vSystemTask, "SystemTask", 2048, nullptr, 1, nullptr);
}

void loop() {
}
