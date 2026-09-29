#include <optional>
#include <LiquidCrystal_I2C.h>
#include "DisplayManager.h"

// We use optional here since we want lcd to be static globally available throughout the program
// however we want to be able to set the I2C address from within main.cpp
// and it cannot be initialized without the default constructor, which needs the I2C address
static std::optional<LiquidCrystal_I2C> lcd;

void initDisplay(uint8_t i2cAddr, uint8_t cols,  uint8_t rows) {
    // Populate lcd with default constructor using parameters fetched from main.cpp
    lcd.emplace(i2cAddr, cols, rows);

    lcd->init();
    lcd->backlight();
    updateDisplay();
}

void updateDisplay() {
    // 17 byte character buffer for using snprintf to format our lcd text.
    // Made buffer a little bigger thank the 16 lcd characters
    // to account for null terminator byte.
    // This makes sure the string doesn't terminate early on the LCD screen
    // causing leftover text to remain on the LCD screen.
    char displayBuffer[17];

    switch (currentState) {
    case MenuState::LOCKED:
        lcd->setCursor(0, 0);
        lcd->print(" [ OP SLOT ]    ");
        lcd->setCursor(0, 1);
        lcd->print(" Scan RFID AUB  ");
        break;

    case MenuState::MAIN_MENU:
        lcd->setCursor(0, 0);
        lcd->print("=== HOOFDMENU ==");
        lcd->setCursor(0, 1);
        if (currentSelection == 0) lcd->print("> Timer         ");
        else lcd->print("> Instellingen  ");
        break;

    case MenuState::SETTINGS_MENU:
        lcd->setCursor(0, 0);
        lcd->print("== INSTELLINGEN ");
        lcd->setCursor(0, 1);
        if (currentSelection == 0) lcd->print("> Alarm Volume  ");
        else lcd->print("> Terug         ");
        break;

    case MenuState::SET_VOLUME:
        lcd->setCursor(0, 0);
        lcd->print("Set Volume:     ");
        lcd->setCursor(0, 1);
        snprintf(displayBuffer, sizeof(displayBuffer), "[ %3d%% ]        ", alarmVolume);
        lcd->print(displayBuffer);
        break;

    case MenuState::SET_TIMER_HOURS:
        lcd->setCursor(0, 0);
        lcd->print("Instellen Uren: ");
        lcd->setCursor(0, 1);
        snprintf(displayBuffer, sizeof(displayBuffer), "[ %02d uur ]      ", timerHours);
        lcd->print(displayBuffer);
        break;

    case MenuState::SET_TIMER_MINUTES:
        lcd->setCursor(0, 0);
        lcd->print("Instellen Min:  ");
        lcd->setCursor(0, 1);
        snprintf(displayBuffer, sizeof(displayBuffer), "[ %02d min ]      ", timerMinutes);
        lcd->print(displayBuffer);
        break;

    case MenuState::WAITING_FOR_RFID_REMOVE:
        lcd->setCursor(0, 0);
        lcd->print("Timer ingesteld ");
        lcd->setCursor(0, 1);
        lcd->print("Verwijder RFID..");
        break;

    case MenuState::TIMER_RUNNING:
        lcd->setCursor(0, 0);

        if (isAlarmRinging) {
            lcd->print("!! WAKE UP !!   ");
            lcd->setCursor(0, 1);
            lcd->print("> Toon RFID <   ");
        }
        else {
            lcd->print("Timer Loopt...  ");
            lcd->setCursor(0, 1);

            const int hrs = totalSecondsRemaining / 3600;
            const int mins = (totalSecondsRemaining % 3600) / 60;
            const int secs = totalSecondsRemaining % 60;

            snprintf(displayBuffer, sizeof(displayBuffer), "    %02d:%02d:%02d    ", hrs, mins, secs);
            lcd->print(displayBuffer);
        }
        break;

    default:
        break;
    }
}
