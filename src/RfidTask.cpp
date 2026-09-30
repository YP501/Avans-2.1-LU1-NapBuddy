#include <SPI.h>
#include <MFRC522.h>
#include "RfidTask.h"
#include "MenuState.h"

static MFRC522 mfrc522;
QueueHandle_t rfidQueue = nullptr;

static bool tagPresent = false;
static bool tagSaved = false;

// PICC stands for Proximity Integrated Circuit Card
// This is the unique identifier that each tag has and will be used to save a certain tag
static byte nuidPICC[4];
static bool correctCardPresent = false;

static unsigned long lastSeen = 0;
constexpr uint32_t TAG_TIMEOUT = 250; // ms

static void handleTagDetected();
static void handleTagRemoved();
static void saveUID();
static bool uidMatches();

void initRFID(const uint8_t ssPin, const uint8_t rstPin) {
    SPI.begin();
    mfrc522.PCD_Init(ssPin, rstPin);
    rfidQueue = xQueueCreate(5, sizeof(RfidEvent));
    Serial.println("RFID Ready");
}

// Send to the rfidQueue for the RfidTask to process -> different thread
static void sendRfidEvent(const RfidEvent event) {
    if (rfidQueue != nullptr) {
        xQueueSend(rfidQueue, &event, 0);
    }
}

// Compares each byte of the detected UID with the save UID and returns true if all of them match
static bool uidMatches() {
    for (byte i = 0; i < 4; i++) {
        if (mfrc522.uid.uidByte[i] != nuidPICC[i]) {
            return false;
        }
    }
    return true;
}

// Loops through each byte of the detected UID and saves it to nuidPICC, afterwards set tagSaved to true and show in serial
static void saveUID() {
    for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = mfrc522.uid.uidByte[i];
    }
    tagSaved = true;
    Serial.print("Saved UID: ");
    for (const uint8_t i : nuidPICC) {
        Serial.print(i, HEX);
        Serial.print(" ");
    }
    Serial.println();
}

static void handleTagDetected() {
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Save a new RFID tag if no tag is saved yet OR if the system is currently LOCKED
    if (!tagSaved || currentState == MenuState::LOCKED) {
        saveUID();
        correctCardPresent = true;
        Serial.println("Authorized card set/detected");
        sendRfidEvent(RfidEvent::AUTHORIZED_CARD_PRESENT);
        return;
    }

    // Is set to true if detected UID matches saved one
    correctCardPresent = uidMatches();

    if (correctCardPresent) {
        Serial.println("Authorized card detected");
        sendRfidEvent(RfidEvent::AUTHORIZED_CARD_PRESENT);
    } else {
        Serial.println("Unauthorized card detected");
    }
}

// When tag is removed
static void handleTagRemoved() {
    if (correctCardPresent) {
        Serial.println("Authorized card removed");
        sendRfidEvent(RfidEvent::AUTHORIZED_CARD_REMOVED);
    } else {
        Serial.println("Unauthorized card removed");
    }

    tagPresent = false;
    correctCardPresent = false;
}

// The first three lines of code are there to confirm if an RFID tag is present
// The ATQA (Answer To reQuest A) has to be stored as the library used requires it
static void updateRFID() {
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);

    // Sends a request to the tag and writes the result to the buffer
    // If this succeeded StatusCode wil be set to STATUS_OK
    MFRC522::StatusCode result = mfrc522.PICC_WakeupA(bufferATQA, &bufferSize);

    // Check if tag has responded
    if (result == MFRC522::STATUS_OK) {
        lastSeen = millis();

        if (!tagPresent) {
            tagPresent = true;
            handleTagDetected();
        }

        // Finish communication with the tag
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    }

    // If a tag hasn't been present since
    if (tagPresent && (millis() - lastSeen > TAG_TIMEOUT)) {
        handleTagRemoved();
    }
}

void vRfidTask(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // We couldn't get the interrupt of the RFID to work so this task
        // keeps polling the RFID sensor on its own thread every 50 seconds.
        // Allegedly the IRQ pin of the sensor should send out an interrupt signal when it detects an RFID tag,
        // However the picoscope wouldn't show anything.
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
        updateRFID();
    }
}