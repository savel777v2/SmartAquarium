/*
  Menu.h - Объект реализации меню в модуле дисплея TM1638My.h
  Также этот объект выполняет любые дейсвтия по нажатию клавиатуры модуля
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

#include "Lamps.h"
extern Lamps globLamps;

enum submenu
{
  timeMenu, timer, morning, evening, alarm, lampInterval,
  curTemp, logTemp, dayTemp, nightTemp, deltaTemp,
  bubblesInSecond, bubbleControlSettings, sensorValue, bubbleSettings, bubbleDaySpeed, bubbleNightSpeed, beforeMorningStart, bubbleControlSound,
  feedingLoop, morningFeeding, eveningFeeding, dayFeedingSettings, nightFeeding,
  motorPosition, durations,
  anon
};

class Menu {

  public:
    Menu();
    // отображает содержимое меню в модуле дисплея
    void display();
    // читает клавиатуру и выполняет движение по меню, или другие вспомогательные дейсвтия,
    // например управление светом. вызвать как можно чаще
    bool readKeyboardNeedControl();
    // возвращает true, если какое либо меню сейчас редактирууется
    bool editingMenu();
    // меняет флаг мигания текущего редактируемого меню, вызывать с интервалом времени мигания
    void changeBlink();
    submenu getSubmenu();

  private:
    MenuItem* subMenu[6];
    byte gorInd, verInd;
    unsigned long nextKeyboardTime;
    byte numEditItem;

    void initSubmenu(submenu _submenu);
    submenu submenuName(byte _gorInd, byte _verInd);
    byte getDots(submenu _submenu);    
};

#endif
