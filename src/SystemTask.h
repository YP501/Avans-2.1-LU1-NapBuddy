#ifndef NAPBUDDY_SYSTEMTASK_H
#define NAPBUDDY_SYSTEMTASK_H

#include <Arduino.h>

void initBuzzer(uint8_t buzzerPin);
void playBuzzerTone(bool enable);
void vSystemTask(void* pvParameters);

#endif //NAPBUDDY_SYSTEMTASK_H
