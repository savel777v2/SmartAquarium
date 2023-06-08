/*
  LoopTime.h -


*/
#pragma once

#include <TM1638.h>
#include "CurrSettings.h"
#include "MenuItem.h"

class LoopTime {

  public:
    LoopTime(Menu* _menu, MicroDS3231* _rtc, CurrSettings* _currSettings) {
      menu = _menu;
      rtc = _rtc;
      currSettings = _currSettings;
    };
    void loop();

  private:
    Menu* menu;
    MicroDS3231* rtc;
    CurrSettings* currSettings;    
};

void LoopTime::loop() {  
  static unsigned long _lastLoopTime = 0;  

  // Loop once First time
  if (_lastLoopTime == 0) {
    //currSettings->now = rtc->getTime();
    currSettings->now.hour = 10;
    currSettings->now.minute = 24;
    currSettings->now.second = 40;
    //minuteControl();
    _lastLoopTime = millis();    
    menu->display();    
  }

  // Loop increment local time
  if ((millis() - _lastLoopTime) > 1000) {

    // секунда оттикала
    _lastLoopTime  = millis();

    if (menu->getSubmenu() == durations) {
      // print durations
      currSettings->printMax1 = currSettings->max1;
      currSettings->printMax2 = currSettings->max2;
      currSettings->printMax3 = currSettings->max3;
      currSettings->printMax4 = currSettings->max4;
      menu->display();
      currSettings->max1 = 0;
      currSettings->max2 = 0;
      currSettings->max3 = 0;
      currSettings->max4 = 0;
    }

    currSettings->now.second++;
    if (currSettings->now.second == 60) {
      currSettings->now.second = 0;
      currSettings->now.minute++;
      if (currSettings->now.minute == 60) {
        // синхронизация времени раз в час
        //currSettings->now = rtc->getTime();
        //currSettings->now.hour = 10;
        //currSettings->now.minute = 22;
      }
      if (menu->getSubmenu() == time) menu->display();
      //minuteControl();
    }
  }

}
