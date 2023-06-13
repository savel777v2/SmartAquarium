/*
  LoopTime.h -


*/
#pragma once

#include "CurrSettings.h"
#include "MenuItem.h"

class LoopTime {

  public:
    LoopTime(Menu* _menu, MicroDS3231* _rtc, CurrSettings* _currSettings) {
      menu = _menu;
      rtc = _rtc;
      currSettings = _currSettings;
      lastLoopTime = 0;
    };
    void loop();

  private:
    Menu* menu;
    MicroDS3231* rtc;
    CurrSettings* currSettings;
    unsigned long lastLoopTime;
    void minuteControl();
};

void LoopTime::loop() {

  // Loop once First time
  if (lastLoopTime == 0) {
    //currSettings->now = rtc->getTime();
    currSettings->now.hour = 11;
    currSettings->now.minute = 42;
    currSettings->now.second = 40;
    //minuteControl();
    lastLoopTime = millis();
    menu->display();
  }

  // turn off alarm Melody
  if (currSettings->alarmMelody != nullptr) {
    currSettings->alarmMelody->loop();
    if ((millis() - currSettings->alarmMelody->getMelodyStartTime()) >= ALARM_DURATION) {
      delete currSettings->alarmMelody;
      currSettings->alarmMelody = nullptr;
    }
  }

  // turn off second Led
  if (currSettings->secondLed && (millis() - lastLoopTime) > SECOND_LED_DURATION) {
    currSettings->secondLed = false;
    menu->display();
  }

  // Loop increment local time
  if ((millis() - lastLoopTime) > 1000) {

    // секунда оттикала
    lastLoopTime  = millis();
    if (menu->getSubmenu() == time) {
      currSettings->secondLed = true;
      menu->display();
    }

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
      minuteControl();
    }
  }
}

void LoopTime::minuteControl() {

  int _nowInMinutes = (int)currSettings->now.hour * 60 + currSettings->now.minute;

  if (currSettings->alarmMelody == nullptr && EEPROM.read(EEPROM_ALARM) == 1) {
    int _alarmInMinutes = (int)EEPROM.read(EEPROM_ALARM_HOUR) * 60 + EEPROM.read(EEPROM_ALARM_MINUTE);
    if (_nowInMinutes == _alarmInMinutes) currSettings->alarmMelody = new Melody(ALARM_PIN);
  }

}
