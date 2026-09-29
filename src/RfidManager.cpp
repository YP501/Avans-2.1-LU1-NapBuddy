#include <SPI.h>
#include <MFRC522.h>
#include "RfidManager.h"
#include "MenuState.h"

static MFRC522 mfrc522;

static bool tagPresent = false;
static bool tagSaved = false;
static byte nuidPICC[4];
bool correctCardPresent = false;

static unsigned long lastSeen = 0;
constexpr uint32_t TAG_TIMEOUT = 250; // ms

static void handleTagDetected();
static void handleTagRemoved();
static void saveUID();
static bool uidMatches();

void initRFID(uint8_t ssPin, uint8_t rstPin) {
    SPI.begin();
    mfrc522.PCD_Init(ssPin, rstPin);
    Serial.println("RFID Ready");
}

void updateRFID() {
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);

    MFRC522::StatusCode result = mfrc522.PICC_WakeupA(bufferATQA, &bufferSize);

    if (result == MFRC522::STATUS_OK) {
        lastSeen = millis();

        if (!tagPresent) {
            tagPresent = true;
            handleTagDetected();
        }

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    }

    if (tagPresent && (millis() - lastSeen > TAG_TIMEOUT)) {
        handleTagRemoved();
    }
}

static bool uidMatches() {
    for (byte i = 0; i < 4; i++) {
        if (mfrc522.uid.uidByte[i] != nuidPICC[i]) {
            return false;
        }
    }
    return true;
}

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

    // Als er nog geen tag is opgeslagen OF als het systeem momenteel op slot is:
    // Sla deze kaart op als de actieve (geautoriseerde) kaart!
    if (!tagSaved || currentState == MenuState::LOCKED) {
        saveUID();
        correctCardPresent = true;
        Serial.println("Authorized card set/detected");
        return;
    }

    correctCardPresent = uidMatches();

    if (correctCardPresent) {
        Serial.println("Authorized card detected");
    } else {
        Serial.println("Unauthorized card detected");
    }
}

static void handleTagRemoved() {
    if (correctCardPresent) {
        Serial.println("Authorized card removed");
    } else {
        Serial.println("Unauthorized card removed");
    }

    tagPresent = false;
    correctCardPresent = false;
}