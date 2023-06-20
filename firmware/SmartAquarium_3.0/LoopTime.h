/*
  LoopTime.h -


*/
#pragma once

#include "CurrSettings.h"
#include "MenuItem.h"

class LoopTime {

  public:
    LoopTime(TM1638My* _module, Menu* _menu, Lamps* _lamps, ControlTemp* _controlTemp, MicroDS3231* _rtc, CurrSettings* _currSettings);
    void readKeyboard();
    void loop();    
    void minuteControl();

  private:
    TM1638My* module;
    Menu* menu;
    Lamps* lamps;
    ControlTemp* controlTemp;
    MicroDS3231* rtc;
    CurrSettings* currSettings;
    unsigned long nextKeyboardTime, lastLoopTime;
    bool itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes);
};

LoopTime::LoopTime (TM1638My* _module, Menu* _menu, Lamps* _lamps, ControlTemp* _controlTemp, MicroDS3231* _rtc, CurrSettings* _currSettings) {
  module = _module;
  menu = _menu;
  lamps = _lamps;
  controlTemp = _controlTemp;
  rtc = _rtc;
  currSettings = _currSettings;
  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  lastLoopTime = 0;
};

void LoopTime::readKeyboard() {

  if (millis() <= nextKeyboardTime) return false;

  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  byte keys = module->keysPressed(B10000000, B00000000);
  if (module->keyPressed(7, keys)) {
    // manualLamp
    lamps->changeManualLamp();
  }
}

void LoopTime::loop() {

  // Loop once First time
  if (lastLoopTime == 0) {
    lastLoopTime = millis();

    //currSettings->now = rtc->getTime();
    currSettings->now.hour = 11;
    currSettings->now.minute = 42;
    currSettings->now.second = 40;

    minuteControl();
    menu->display();
  }

  // temp reader and display
  if (controlTemp->readTemperatureNeedDisplay() && menu->getSubmenu() == curTemp) menu->display();

  // loop timer
  if (currSettings->timer != nullptr) {
    status curStatus = currSettings->timer->loop();
    if (curStatus == off) {
      delete currSettings->timer;
      currSettings->timer = nullptr;
      if (currSettings->alarmMelody == nullptr) currSettings->alarmMelody = new Melody(PIEZO_PIN);
      else (currSettings->alarmMelody->restart());
      if (menu->getSubmenu() == time || menu->getSubmenu() == timer) {
        menu->display();
      }
    }
    else if (curStatus == display && menu->getSubmenu() == timer) menu->display();
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
        currSettings->now.minute = 0;
        currSettings->now.hour++;
        // синхронизация времени раз в час
        //currSettings->now = rtc->getTime();
      }
      minuteControl();
    }
  }
}

// return it's day or nigth based on Morning and Evening
bool LoopTime::itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes) {
  if (_eveningInMinutes > _morningInMinutes) return (_nowInMinutes >= _morningInMinutes && _nowInMinutes < _eveningInMinutes);
  else return (_nowInMinutes >= _morningInMinutes || _nowInMinutes < _eveningInMinutes);
}

void LoopTime::minuteControl() {

  // local values
  int _nowInMinutes = (int)currSettings->now.hour * 60 + currSettings->now.minute;
  int _morningInMinutes = (int)EEPROM.read(EEPROM_MORNING_HOUR) * 60 + EEPROM.read(EEPROM_MORNING_MINUTE);
  int _eveningInMinutes = (int)EEPROM.read(EEPROM_EVENING_HOUR) * 60 + EEPROM.read(EEPROM_EVENING_MINUTE);

  // it's day or nigth
  currSettings->nowDay = itsDay(_nowInMinutes, _morningInMinutes, _eveningInMinutes);

  if (EEPROM.read(EEPROM_ALARM) == 1) {
    int _alarmInMinutes = (int)EEPROM.read(EEPROM_ALARM_HOUR) * 60 + EEPROM.read(EEPROM_ALARM_MINUTE);
    if (_nowInMinutes == _alarmInMinutes) {
      if (currSettings->alarmMelody == nullptr) currSettings->alarmMelody = new Melody(PIEZO_PIN);
      else currSettings->alarmMelody->restart();
    }
  }

  // control lamps
  lamps->controlLamps();

  // control heater
  controlTemp->heaterOnOff(currSettings->nowDay, currSettings->now.minute, currSettings->now.hour);

}
