#ifndef NAPBUDDY_RFIDTASK_H
#define NAPBUDDY_RFIDTASK_H
#include <freertos/queue.h>

enum class RfidEvent {
    AUTHORIZED_CARD_PRESENT,
    AUTHORIZED_CARD_REMOVED
};

extern QueueHandle_t rfidQueue;

void initRFID(uint8_t ssPin, uint8_t rstPin);
void vRfidTask(void* pvParameters);

#endif //NAPBUDDY_RFIDTASK_H
