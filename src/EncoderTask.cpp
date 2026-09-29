#include "EncoderTask.h"
#include "DisplayManager.h"
#include "SystemTask.h"

TWIST twist;
SemaphoreHandle_t encoderSemaphore = nullptr;

MenuState currentState = MenuState::LOCKED; // Start with system locked and waiting for RFID
int currentSelection = 0;

// Alarm config values
int alarmVolume = 50;
int timerHours = 0;
int timerMinutes = 0;

// Interrupt which wakes up the encoder task when int pin fires from encoder
static void IRAM_ATTR encoderISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(encoderSemaphore, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void initEncoder(const uint8_t i2cAddr, const uint8_t intPin) {
    pinMode(intPin, INPUT_PULLUP);
    twist.begin(Wire, i2cAddr);
    twist.clearInterrupts();

    encoderSemaphore = xSemaphoreCreateBinary();

    attachInterrupt(digitalPinToInterrupt(intPin), encoderISR, FALLING);
}

void resetSystemToDefaults() {
    currentSelection = 0;
    alarmVolume = 50;
    timerHours = 0;
    timerMinutes = 0;
    totalSecondsRemaining = 0;
    isTimerActive = false;
    isAlarmRinging = false;
}

// Task which reads out the encoder and acts accordingly
void vEncoderTask(void* pvParameters) {
    bool alreadyPressed = false;
    int lastCount = twist.getCount();

    for (;;) {
        // Small tick amount when butten was pressed to keep polling for release
        const TickType_t xTicksToWait = alreadyPressed ? pdMS_TO_TICKS(15) : portMAX_DELAY;

        // Waits for signal from interrupt
        if (xSemaphoreTake(encoderSemaphore, xTicksToWait) == pdTRUE || alreadyPressed) {
            // If system locked, ignore encoder input, however DO update count and clear the interrupt
            if (currentState == MenuState::LOCKED) {
                lastCount = twist.getCount();
                twist.clearInterrupts();
                continue;
            }

            const bool isPressed = twist.isPressed();
            const bool isMoved = twist.isMoved();

            // Knob rotation
            if (isMoved) {
                const int currentCount = twist.getCount();
                const int delta = currentCount - lastCount; // How much did we rotate and which direction?
                lastCount = currentCount;

                // Did we even change rotation? For example, forward 3 and backward 3 = nothing change
                if (delta != 0) {
                    // Modulo trickery in this entire switch-case statement to wrap around the selection range
                    switch (currentState) {
                    case MenuState::MAIN_MENU:
                        if (delta > 0) {
                            currentSelection = (currentSelection + 1) % 2; // % 2 means there's only 2 options
                        }
                        else if (delta < 0) {
                            currentSelection = (currentSelection - 1 + 2) % 2; // + 2 sine we are going backwards
                        }
                        break;

                    case MenuState::SETTINGS_MENU:
                        if (delta > 0) {
                            currentSelection = (currentSelection + 1) % 2;
                        }
                        else if (delta < 0) {
                            currentSelection = (currentSelection - 1 + 2) % 2;
                        }
                        break;

                    case MenuState::SET_VOLUME:
                        alarmVolume = constrain(alarmVolume + (delta * 5), 0, 100); // Volume between 0 and 100

                        playBuzzerTone(true);
                        vTaskDelay(pdMS_TO_TICKS(40));
                        playBuzzerTone(false);
                        break;

                    case MenuState::SET_TIMER_HOURS:
                        timerHours = constrain(timerHours + delta, 0, 23); // Hours between 0 and 23
                        break;

                    case MenuState::SET_TIMER_MINUTES:
                        timerMinutes = constrain(timerMinutes + delta, 0, 59); // Hours between 0 and 59
                        break;

                    case MenuState::LOCKED:
                    case MenuState::WAITING_FOR_RFID_REMOVE:
                    case MenuState::TIMER_RUNNING:
                        // Ignore rotations for all above states.
                        // Reason we also check for locked state above, is because we want early guard clause
                        // and prevent unnecessary I2C communication and/or calculations
                        break;
                    }

                    updateDisplay();
                }
            }

            // Button pressing
            if (isPressed && !alreadyPressed) { // Debounce to check if we already pressed button to prevent spamming
                alreadyPressed = true;

                switch (currentState) {
                case MenuState::MAIN_MENU:
                    if (currentSelection == 0) {
                        currentState = MenuState::SET_TIMER_HOURS;
                    }
                    else {
                        currentState = MenuState::SETTINGS_MENU;
                        currentSelection = 0;
                    }
                    break;

                case MenuState::SETTINGS_MENU:
                    if (currentSelection == 0) {
                        currentState = MenuState::SET_VOLUME;
                    }
                    else {
                        currentState = MenuState::MAIN_MENU;
                        currentSelection = 0;
                    }
                    break;

                case MenuState::SET_VOLUME:
                    currentState = MenuState::SETTINGS_MENU;
                    break;

                case MenuState::SET_TIMER_HOURS:
                    currentState = MenuState::SET_TIMER_MINUTES;
                    break;

                case MenuState::SET_TIMER_MINUTES:
                    totalSecondsRemaining = (timerHours * 3600) + (timerMinutes * 60);

                    if (totalSecondsRemaining > 0) {
                        isTimerActive = false;
                        isAlarmRinging = false;
                        currentState = MenuState::WAITING_FOR_RFID_REMOVE;
                    }
                    else {
                        currentState = MenuState::MAIN_MENU;
                    }
                    break;

                case MenuState::LOCKED:
                case MenuState::WAITING_FOR_RFID_REMOVE:
                case MenuState::TIMER_RUNNING:
                    break;
                }

                updateDisplay();
            }
            else if (!isPressed && alreadyPressed) { // When button is physically released, we also release debounce
                alreadyPressed = false;
            }

            twist.clearInterrupts(); // Clear any active interrupts so that we can listen for new ones
        }
    }
}
