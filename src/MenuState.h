#ifndef NAPBUDDY_MENUSTATE_H
#define NAPBUDDY_MENUSTATE_H

#include <Arduino.h>

enum class MenuState {
    LOCKED,
    MAIN_MENU,
    SETTINGS_MENU,
    SET_VOLUME,
    SET_TIMER_HOURS,
    SET_TIMER_MINUTES,
    WAITING_FOR_RFID_REMOVE,
    TIMER_RUNNING
};

extern MenuState currentState;
extern int currentSelection;
extern int alarmVolume;
extern int timerHours;
extern int timerMinutes;
extern int32_t totalSecondsRemaining;
extern bool isTimerActive;
extern bool isAlarmRinging;
extern bool isCardCurrentlyPresent;

#endif //NAPBUDDY_MENUSTATE_H
