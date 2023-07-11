/*
  LoopTime.h - Основной объект для обсчета времени и Управления другими объектами
  вызов, контроль расписаний
*/

#ifndef LoopTime_h
#define LoopTime_h

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

#include "Menu.h"
#include "Lamps.h"

#define SECOND_NOLED_DURATION 950

class LoopTime {

  public:
    LoopTime(TM1638My* _module, Menu* _menu, Lamps* _lamps, ControlTemp* _controlTemp, BubbleCounter* _bubbleCounter, StepMotor* _stepMotor, BubbleControl* _bubbleControl, Feeding* _feeding, MicroDS3231* _rtc, global::CurrSettings* _currSettings);
    void readKeyboard();
    void loop();
    void minuteControl();

  private:
    TM1638My* module;
    Menu* menu;
    Lamps* lamps;
    ControlTemp* controlTemp;
    BubbleCounter* bubbleCounter;
    StepMotor* stepMotor;
    BubbleControl* bubbleControl;
    Feeding* feeding;
    MicroDS3231* rtc;
    global::CurrSettings* currSettings;
    unsigned long nextKeyboardTime, nextSecondTime;
    byte activeLedMotor = 0;
    bool itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes);
};

#endif
