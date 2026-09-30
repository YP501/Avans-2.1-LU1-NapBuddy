#include "SystemTask.h"
#include "DisplayManager.h"
#include "EncoderTask.h"
#include "MenuState.h"
#include "BuzzerTask.h"
#include "RfidTask.h"

constexpr int MAX_LOCK_COUNTDOWN = 5;

// RFID specific variables
static int lockCountdown = 0;
bool isCardCurrentlyPresent = false;

// Define global timer variables. From MenuState.h
int32_t totalSecondsRemaining = 0;
bool isTimerActive = false;
bool isAlarmRinging = false;

static void handleRfidEvent(const RfidEvent event) {
    switch (event) {
    case RfidEvent::AUTHORIZED_CARD_PRESENT:
        isCardCurrentlyPresent = true;

        if (currentState == MenuState::LOCKED) {
            currentState = MenuState::MAIN_MENU;
            lockCountdown = MAX_LOCK_COUNTDOWN;
            updateDisplay();
        }
        else if (currentState == MenuState::TIMER_RUNNING) {
            isTimerActive = false;
            if (isAlarmRinging) {
                isAlarmRinging = false;
                triggerBuzzer(BuzzerCommand::OFF);
            }
            currentState = MenuState::MAIN_MENU;
            lockCountdown = MAX_LOCK_COUNTDOWN;
            updateDisplay();
        }
        break;

    case RfidEvent::AUTHORIZED_CARD_REMOVED:
        isCardCurrentlyPresent = false;

        if (currentState == MenuState::WAITING_FOR_RFID_REMOVE) {
            isTimerActive = true;
            currentState = MenuState::TIMER_RUNNING;
            updateDisplay();
        }
        break;
    }
}

static void handleOneSecondTick() {
    // Grace period in the menu's if you accidentally drop the RFID or something like that
    if (currentState != MenuState::LOCKED &&
        currentState != MenuState::TIMER_RUNNING &&
        currentState != MenuState::WAITING_FOR_RFID_REMOVE) {
        if (isCardCurrentlyPresent) {
            lockCountdown = MAX_LOCK_COUNTDOWN;
        }
        else {
            if (lockCountdown > 0) {
                lockCountdown--;
            }
            else {
                resetSystemToDefaults();
                updateDisplay();
            }
        }
    }

    // Count down timer
    if (isTimerActive && !isAlarmRinging) {
        if (totalSecondsRemaining > 0) {
            totalSecondsRemaining--;

            if (currentState == MenuState::TIMER_RUNNING) {
                updateDisplay();
            }
        }
        else {
            isAlarmRinging = true;
            triggerBuzzer(BuzzerCommand::START_ALARM);

            if (currentState == MenuState::TIMER_RUNNING) {
                updateDisplay();
            }
        }
    }
}

void vSystemTask(void* pvParameters) {
    RfidEvent event;
    TickType_t lastTimerTick = xTaskGetTickCount();
    constexpr TickType_t oneSecondInTicks = pdMS_TO_TICKS(1000);

    for (;;) {
        // How much time is there until the next one second tick?
        const TickType_t now = xTaskGetTickCount();
        const TickType_t elapsed = now - lastTimerTick;

        TickType_t waitTime = 0;
        if (elapsed < oneSecondInTicks) {
            waitTime = oneSecondInTicks - elapsed;
        }

        // Wait for an RFID event from RfidTask or if 1 second has passed
        if (xQueueReceive(rfidQueue, &event, waitTime) == pdTRUE) {
            handleRfidEvent(event);
        }
        else {
            // waitTime was reached, 1 second has passed!
            lastTimerTick += oneSecondInTicks;
            handleOneSecondTick();
        }
    }
}
