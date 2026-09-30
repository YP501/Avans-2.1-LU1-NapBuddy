#include "BuzzerTask.h"
#include "MenuState.h"

constexpr uint8_t PWM_CHANNEL = 0;
constexpr uint8_t PWM_RESOLUTION = 8; // Bytes
constexpr uint16_t ALARM_FREQ = 2000; // TODO: Maybe make it so you can choose from melody in settings?

static QueueHandle_t buzzerQueue = nullptr;

static void playTone(const bool enable) {
    if (enable) {
        // Convert 0-100% volume to a PWM duty cycle
        // 8-bit resolution maxes out at 255, but a 50% duty cycle (~127) yields the maximum
        // volume because it provides equal high/low time, allowing the buzzer membrane
        // to achieve its maximum physical movement/amplitude. SOURCE: Google Gemini
        // Note: Human hearing is logarithmic, but a linear mapping suffices for this proof of concept.
        const int dutyCycle = map(alarmVolume, 0, 100, 0, 127);
        ledcWrite(PWM_CHANNEL, dutyCycle);
    } else {
        ledcWrite(PWM_CHANNEL, 0); // Silence!
    }
}

void initBuzzer(const uint8_t pin) {
    ledcSetup(PWM_CHANNEL, ALARM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(pin, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 0);

    // Initialize buzzer queue
    buzzerQueue = xQueueCreate(10, sizeof(BuzzerCommand));
}

void triggerBuzzer(const BuzzerCommand cmd) {
    if (buzzerQueue != nullptr) {
        xQueueSend(buzzerQueue, &cmd, 0);
    }
}

void vBuzzerTask(void* pvParameter) {
    BuzzerCommand cmd;
    bool isAlarmActive = false;
    bool buzzerState = false;

    for (;;) {
        // If alarm is going off, we wait 500ms for each beep
        // Else, we wait for command in the queue
        const TickType_t xTicksToWait = isAlarmActive ?  pdMS_TO_TICKS(500) : portMAX_DELAY;

        if (xQueueReceive(buzzerQueue, &cmd, xTicksToWait) == pdTRUE) {
            switch (cmd) {
            case BuzzerCommand::SHORT_BEEP:
                playTone(true);
                vTaskDelay(pdMS_TO_TICKS(60));
                playTone(false);
                break;

            case BuzzerCommand::START_ALARM:
                isAlarmActive = true;
                buzzerState = true;
                playTone(buzzerState);
                break;

            case BuzzerCommand::OFF:
                isAlarmActive = false;
                buzzerState = false;
                playTone(false);
                break;
            }
        } else {
            // Do alarm beeping toggling
            if (isAlarmActive) {
                buzzerState = !buzzerState;
                playTone(buzzerState);
            }
        }
    }
}
