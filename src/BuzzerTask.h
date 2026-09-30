#ifndef NAPBUDDY_BUZZERTASK_H
#define NAPBUDDY_BUZZERTASK_H

#include <Arduino.h>

enum class BuzzerCommand {
    SHORT_BEEP,
    START_ALARM,
    OFF
};

void initBuzzer(uint8_t pin);
void triggerBuzzer(BuzzerCommand cmd);
void vBuzzerTask(void* pvParameter);

#endif //NAPBUDDY_BUZZERTASK_H
