/*
  LoopTime.h -


*/
#pragma once

#include "CurrSettings.h"
#include "MenuItem.h"
#include "BubbleCounter.h"

class LoopTime {

  public:
    LoopTime(TM1638My* _module, Menu* _menu, Lamps* _lamps, ControlTemp* _controlTemp, BubbleCounter* _bubbleCounter, StepMotor* _stepMotor, BubbleControl* _bubbleControl, MicroDS3231* _rtc, CurrSettings* _currSettings);
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
    MicroDS3231* rtc;
    CurrSettings* currSettings;
    unsigned long nextKeyboardTime, lastLoopTime;
    byte activeLedMotor = 0;
    bool itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes);
};

LoopTime::LoopTime (TM1638My* _module, Menu* _menu, Lamps* _lamps, ControlTemp* _controlTemp, BubbleCounter* _bubbleCounter, StepMotor* _stepMotor, BubbleControl* _bubbleControl, MicroDS3231* _rtc, CurrSettings* _currSettings) {
  module = _module;
  menu = _menu;
  lamps = _lamps;
  controlTemp = _controlTemp;
  bubbleCounter = _bubbleCounter;
  stepMotor = _stepMotor;
  bubbleControl = _bubbleControl;
  rtc = _rtc;
  currSettings = _currSettings;
  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  lastLoopTime = 0;
  activeLedMotor = 0;
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
    currSettings->now = rtc->getTime();
    /*currSettings->now.hour = 11;
    currSettings->now.minute = 42;
    currSettings->now.second = 40;*/

    minuteControl();
    menu->display();
  }

  // temp reader and display
  if (controlTemp->readTemperatureNeedDisplay() && menu->getSubmenu() == curTemp) menu->display();

  // loop BubbleCounter and display
  byte loopResult = bubbleCounter->loop();
  if ((loopResult & 0b00000001) == 0b00000001 && menu->getSubmenu() == sensorValue) menu->display();
  if ((loopResult & 0b00000100) == 0b00000100) module->setLED(1, 7); // начало пузырька
  if ((loopResult & 0b00001000) == 0b00001000) module->setLED(0, 7); // конец пузырька
  if ((loopResult & 0b00010000) == 0b00010000) {
    // контроль пузырьков - по ошибке или пузырьку
    if (bubbleControl->controlWaiting()) module->setLED(1, 6);
    else module->setLED(0, 6);
    if (menu->getSubmenu() == bubblesInSecond) menu->display();
  }

  // loop StepMotor and display
  int _direction = stepMotor->loop();
  if (_direction != 255) {
    module->setLED(0, activeLedMotor);
    if (_direction < 0) activeLedMotor = activeLedMotor == 0 ? 3 : activeLedMotor - 1;
    else if (_direction > 0) activeLedMotor = activeLedMotor == 3 ? 0 : activeLedMotor + 1;
    if (_direction != 0) module->setLED(1, activeLedMotor);
  }

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
        currSettings->now = rtc->getTime();
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

  // control alarm
  if (EEPROM.read(EEPROM_ALARM) == 1) {
    int _alarmInMinutes = (int)EEPROM.read(EEPROM_ALARM_HOUR) * 60 + EEPROM.read(EEPROM_ALARM_MINUTE);
    if (_nowInMinutes == _alarmInMinutes) {
      if (currSettings->alarmMelody == nullptr) currSettings->alarmMelody = new Melody(PIEZO_PIN);
      else currSettings->alarmMelody->restart();
    }
  }

  // change control bubble settings  
  int _morningBubbles = _morningInMinutes - EEPROM.read(EEPROM_BEFORE_MORNING_BUBBLE_START);
  if (_morningBubbles < 0) _morningBubbles = _morningBubbles + 1440;
  int _eveningBubbles = _eveningInMinutes - EEPROM.read(EEPROM_LAMP_INTERVAL) * 2;
  if (_eveningBubbles < 0) _eveningBubbles = _eveningBubbles + 1440;
  byte _needingBubbleSpeed;
  byte _needingStatus;
  if (itsDay(_nowInMinutes, _morningBubbles, _eveningBubbles)) {
    _needingBubbleSpeed = EEPROM.read(EEPROM_DAY_BUBBLE_SPEED);
    _needingStatus = EEPROM.read(EEPROM_DAY_BUBBLE_ON);
  }
  else {
    _needingBubbleSpeed = EEPROM.read(EEPROM_NIGHT_BUBBLE_SPEED);
    _needingStatus = EEPROM.read(EEPROM_NIGHT_BUBBLE_ON);
  }
  bubbleControl->set_currStatus(_needingStatus);
  bubbleControl->set_bubblesIn100Second(_needingBubbleSpeed);

  // control lamps
  lamps->controlLamps();

  // control heater
  controlTemp->heaterOnOff(currSettings->nowDay, currSettings->now.minute, currSettings->now.hour);

}
