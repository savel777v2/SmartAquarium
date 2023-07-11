/*
  Menu.h - Объект реализации меню в модуле дисплея TM1638My.h
*/

#ifndef Menu_h
#define Menu_h

#include <EEPROM.h>
#include <Arduino.h>
#include <microDS3231.h>
#include "Global.h"
#include "TM1638My.h"
#include "Feeding.h"
#include "BubbleControl.h"
#include "BubbleCounter.h"
#include "ControlTemp.h"
#include "StepMotor.h"
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
    Menu (TM1638My* _module, ControlTemp* _controlTemp, BubbleCounter* _bubbleCounter, StepMotor* _stepMotor, BubbleControl* _bubbleControl, Feeding* _feeding, MicroDS3231* _rtc, global::CurrSettings* _currSettings);
    void display();
    bool loopNeedControl();
    submenu getSubmenu();

  private:
    TM1638My* module;
    MenuItem* subMenu[6];
    ControlTemp* controlTemp;
    BubbleCounter* bubbleCounter;
    StepMotor* stepMotor;
    BubbleControl* bubbleControl;
    Feeding* feeding;
    MicroDS3231* rtc;
    global::CurrSettings* currSettings;
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
