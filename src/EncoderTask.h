#ifndef NAPBUDDY_ENCODERTASK_H
#define NAPBUDDY_ENCODERTASK_H

#include <Arduino.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>

extern TWIST twist;
extern SemaphoreHandle_t encoderSemaphore;

void initEncoder(uint8_t i2cAddr, uint8_t intPin);
void resetSystemToDefaults();
void vEncoderTask(void* pvParameters);

#endif //NAPBUDDY_ENCODERTASK_H
