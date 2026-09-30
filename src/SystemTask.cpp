#include "SystemTask.h"
#include "DisplayManager.h"
#include "EncoderTask.h"
#include "MenuState.h"
#include "RfidManager.h"
#include "BuzzerTask.h"

constexpr int MAX_LOCK_COUNTDOWN = 5;

static int lockCountdown = 0;

// Define global timer variables. From MenuState.h
int32_t totalSecondsRemaining = 0;
bool isTimerActive = false;
bool isAlarmRinging = false;

void vSystemTask(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t secondCounter = 0;

    for (;;) {
        // Small 50 ms delay so we do not spam the RFID reader
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));

        // Process possibly new RFID sensor state and update all variables that depend on it
        updateRFID();

        // React accordingly based on updated variables
        if (currentState == MenuState::LOCKED) {
            if (correctCardPresent) {
                currentState = MenuState::MAIN_MENU;
                lockCountdown = MAX_LOCK_COUNTDOWN;
                updateDisplay();
            }
        }
        else if (currentState == MenuState::TIMER_RUNNING) {
            if (correctCardPresent) {
                isTimerActive = false;
                if (isAlarmRinging) {
                    isAlarmRinging = false;
                    triggerBuzzer(BuzzerCommand::OFF);
                }
                currentState = MenuState::MAIN_MENU;
                lockCountdown = MAX_LOCK_COUNTDOWN;
                updateDisplay();
            }
        }

        // Wait for removal of RFID card
        if (currentState == MenuState::WAITING_FOR_RFID_REMOVE) {
            if (!correctCardPresent) {
                isTimerActive = true;
                currentState = MenuState::TIMER_RUNNING;
                updateDisplay();
            }
        }

        // Stuff that should happen once per second
        secondCounter++;
        if (secondCounter >= 20) {
            // 20 * 50ms (from vTaskDelayUntil) = 1000ms
            secondCounter = 0;

            // Small grace period in main menu if you accidentally drop the RFID tag
            // Do NOT give the grace period when state is LOCKED, TIMER_RUNNING or WAITING_FOR_RFID_REMOVE
            if (currentState != MenuState::LOCKED &&
                currentState != MenuState::TIMER_RUNNING &&
                currentState != MenuState::WAITING_FOR_RFID_REMOVE) {
                if (correctCardPresent) {
                    lockCountdown = MAX_LOCK_COUNTDOWN; // Reset countdown if card is present within the time
                }
                else {
                    if (lockCountdown > 0) {
                        lockCountdown--;
                    }
                    else {
                        // Grace period over, lock the system back up
                        if (isAlarmRinging) {
                            isAlarmRinging = false;
                            triggerBuzzer(BuzzerCommand::OFF);
                        }
                        resetSystemToDefaults();
                        currentState = MenuState::LOCKED;
                        updateDisplay();
                    }
                }
            }

            // Update timer values if timer is active and not ringing
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
    }
}
