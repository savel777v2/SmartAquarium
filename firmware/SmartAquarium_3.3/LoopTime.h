/*
  LoopTime.h - Основной объект для обсчета времени и Управления другими объектами
  вызов, контроль расписаний
*/

#ifndef LoopTime_h
#define LoopTime_h

#include <EEPROM.h>
#include <Arduino.h>
#include <microDS3231.h>
extern MicroDS3231 globDS3231;

#include "Global.h"
extern global::CurrSettings globCurrSettings;
#include "TM1638My.h"
extern TM1638My globModule1638;
#include "Feeding.h"
extern Feeding globFeeding;
#include "BubbleControl.h"
extern BubbleControl globBubbleControl;
#include "BubbleCounter.h"
extern BubbleCounter globBubbleCounter;
#include "ControlTemp.h"
extern ControlTemp globControlTemp;
#include "StepMotor.h"
extern StepMotor globStepMotor;

#include "MenuItem.h"

#include "Menu.h"
extern Menu globMenu;
#include "Lamps.h"
extern Lamps globLamps;

#define SECOND_NOLED_DURATION 950

class LoopTime {

  public:
    LoopTime();
    void readKeyboard();
    void loop();
    void minuteControl();

  private:
    unsigned long nextKeyboardTime, nextSecondTime;
    byte activeLedMotor = 0;
    bool itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes);
};

#endif