#include "SystemTask.h"
#include "DisplayManager.h"
#include "EncoderTask.h"
#include "MenuState.h"
#include "RfidManager.h"

// PWM settings for buzzer volume
constexpr uint8_t PWM_CHANNEL = 0;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint16_t ALARM_FREQ = 2000; // TODO: Maybe make it so you can choose your timer sound?

constexpr int MAX_LOCK_COUNTDOWN = 5;

static int lockCountdown = 0;

// Define global timer variables. From MenuState.h
int32_t totalSecondsRemaining = 0;
bool isTimerActive = false;
bool isAlarmRinging = false;

void initBuzzer(const uint8_t buzzerPin) {
    ledcSetup(PWM_CHANNEL, ALARM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(buzzerPin, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 0);
}

void playBuzzerTone(const bool enable) {
    if (enable) {
        // Convert 0-100% volume to a PWM duty cycle
        // 8-bit resolution maxes out at 255, but a 50% duty cycle (~127) yields the maximum
        // volume because it provides equal high/low time, allowing the buzzer membrane
        // to achieve its maximum physical movement/amplitude. SOURCE: Google Gemini
        // Note: Human hearing is logarithmic, but a linear mapping suffices for this proof of concept.
        const int dutyCycle = map(alarmVolume, 0, 100, 0, 127);
        ledcWrite(PWM_CHANNEL, dutyCycle);
    }
    else {
        ledcWrite(PWM_CHANNEL, 0); // Silence!
    }
}

void vSystemTask(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    bool buzzerToggle = false;
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
                isAlarmRinging = false;
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

                    if (currentState == MenuState::TIMER_RUNNING) {
                        updateDisplay();
                    }
                }
            }

            // Toggle buzzer if alarm is ringing, else disable it
            if (isAlarmRinging) {
                buzzerToggle = !buzzerToggle;
                playBuzzerTone(buzzerToggle);
            }
            else {
                playBuzzerTone(false);
                buzzerToggle = false;
            }
        }
    }
}
