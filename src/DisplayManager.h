#ifndef NAPBUDDY_DISPLAYMANAGER_H
#define NAPBUDDY_DISPLAYMANAGER_H

#include "MenuState.h"

void initDisplay(uint8_t i2cAddr, uint8_t cols,uint8_t rows);
void updateDisplay();

#endif //NAPBUDDY_DISPLAYMANAGER_H
