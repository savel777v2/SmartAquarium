/*
  Menu.h - Объект реализации меню в модуле дисплея TM1638My.h
*/

#ifndef Menu_h
#define Menu_h

#include <EEPROM.h>
#include <Arduino.h>
#include <microDS3231.h>
extern MicroDS3231 globDS3231;

#include "Global.h"
extern global::CurrSettings globCurrSettings;

#include "TM1638My.h"
extern TM1638My globModule1638;

#include "ControlTemp.h"
extern ControlTemp globControlTemp;
#include "MenuItem.h"

// интервал мигаия при редатировании детали меню
#define BLINK_INTERVAL 500

enum submenu
{
  timeMenu, timer, morning, evening, alarm, lampInterval,
  curTemp, logTemp, dayTemp, nightTemp, deltaTemp,
  bubblesInSecond, bubbleControlSettings, sensorValue, bubbleSettings, bubbleDaySpeed, bubbleNightSpeed, beforeMorningStart, bubbleControlSound,
  feedingLoop, morningFeeding, eveningFeeding, dayFeedingSettings, nightFeeding, durations,
  motorPosition, motorSpeed, bubbleDurations, bubbleCount, bubblesInMinute, sensorInSecond, errorsInSecond,
  anon
};

class Menu {

  public:
    Menu();
    void display();
    bool loopNeedControl();
    submenu getSubmenu();

  private:
    MenuItem* subMenu[6];
    byte gorInd, verInd;
    unsigned long nextKeyboardTime, lastBlinkTime;
    byte numEditItem;

    void initSubmenu(submenu _submenu);
    submenu submenuName(byte _gorInd, byte _verInd);
    byte getDots(submenu _submenu);
    bool readKeyboardNeedControl();
    void blinkDisplay();
};

#endif
