#ifndef NAPBUDDY_RFIDMANAGER_H
#define NAPBUDDY_RFIDMANAGER_H

extern bool correctCardPresent; // For SystemTask, updated by RfidManager

void initRFID(uint8_t ssPin, uint8_t rstPin);
void updateRFID();

#endif //NAPBUDDY_RFIDMANAGER_H
