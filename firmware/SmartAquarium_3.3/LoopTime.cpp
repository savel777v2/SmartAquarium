/*
  LoopTime.cpp - Программная реализация обсчета времени и Управления другими объектами
  вызов, контроль расписаний
*/

#include "LoopTime.h"

LoopTime::LoopTime () {  
  nextSecondTime = 0;
  activeLedMotor = 0;
};

void LoopTime::loop() {

  // Loop once First time
  if (nextSecondTime == 0) {
    nextSecondTime = millis() + 1000;
    globCurrSettings.nowSecond = globDS3231.getSeconds();
    globCurrSettings.nowMinute = globDS3231.getMinutes();
    globCurrSettings.nowHour = globDS3231.getHours();
    minuteControl();
    globMenu.display();
  };

  // blink isplay if editing
  if (globMenu.editingMenu() && millis() > (globCurrSettings.lastBlinkTime + BLINK_INTERVAL)) {
    globCurrSettings.lastBlinkTime = millis();
    globMenu.changeBlink();
  };

  // temp reader and display
  if (globControlTemp.loopNeedDisplay() && globMenu.getSubmenu() == curTemp) globMenu.display();

  // Feeding and display
  if (globFeeding.loopNeedDisplay() && globMenu.getSubmenu() == feedingLoop) globMenu.display();

  // loop BubbleCounter and display
  byte needDisplayCounter = globBubbleCounter.loopNeedDisplay();
  if ((needDisplayCounter & 0b00000001) == 0b00000001 && globMenu.getSubmenu() == sensorValue) globMenu.display();
  if ((needDisplayCounter & 0b00000100) == 0b00000100) globModule1638.setLED(1, 7); // начало пузырька
  if ((needDisplayCounter & 0b00001000) == 0b00001000) globModule1638.setLED(0, 7); // конец пузырька
  if ((needDisplayCounter & 0b00010000) == 0b00010000) {
    // контроль пузырьков - по ошибке или пузырьку
    if (globBubbleControl.controlWaiting()) globModule1638.setLED(1, 6);
    else globModule1638.setLED(0, 6);
    if (globMenu.getSubmenu() == bubblesInSecond) globMenu.display();
  }

  // loop StepMotor and display
  int _direction = globStepMotor.loopDirection();
  if (_direction != 255) {
    globModule1638.setLED(0, activeLedMotor);
    if (_direction < 0) activeLedMotor = activeLedMotor == 0 ? 3 : activeLedMotor - 1;
    else if (_direction > 0) activeLedMotor = activeLedMotor == 3 ? 0 : activeLedMotor + 1;
    if (_direction != 0) globModule1638.setLED(1, activeLedMotor);
  }

  // loop timer
  if (globCurrSettings.timer != nullptr) {
    int needDisplay = globCurrSettings.timer->loopNeedDisplay();
    if (needDisplay == -1) {
      delete globCurrSettings.timer;
      globCurrSettings.timer = nullptr;
      if (globCurrSettings.alarmMelody == nullptr) globCurrSettings.alarmMelody = new Melody();
      else (globCurrSettings.alarmMelody->restart());
      if (globMenu.getSubmenu() == timeMenu || globMenu.getSubmenu() == timer) {
        globMenu.display();
      }
    }
    else if (needDisplay == 1 && globMenu.getSubmenu() == timer) globMenu.display();
  }

  // turn off alarm Melody
  if (globCurrSettings.alarmMelody != nullptr) {
    int needLoop = globCurrSettings.alarmMelody->loopNeedLoop();
    if (!needLoop) {
      delete globCurrSettings.alarmMelody;
      globCurrSettings.alarmMelody = nullptr;
    }
  }

  // turn off second Led
  if (globCurrSettings.secondLed && millis() > (nextSecondTime - SECOND_NOLED_DURATION)) {
    globCurrSettings.secondLed = false;
    globMenu.display();
  }

  // Loop increment local time
  if (millis() > nextSecondTime) {

    // секунда оттикала
    nextSecondTime += 1000;
    if (globMenu.getSubmenu() == timeMenu) {
      globCurrSettings.secondLed = true;
      globMenu.display();
    }

    if (globMenu.getSubmenu() == durations) {
      // print durations
      for (int i = 0; i < DURATIONS_SIZE; i++) {
        globCurrSettings.printDurations[i] = globCurrSettings.curDurations[i];
        globCurrSettings.curDurations[i] = 0;
      }
      globMenu.display();
    }

    globCurrSettings.nowSecond++;
    if (globCurrSettings.nowSecond == 60) {
      globCurrSettings.nowSecond = 0;
      globCurrSettings.nowMinute++;
      if (globCurrSettings.nowMinute == 60) {
        globCurrSettings.nowMinute = 0;
        globCurrSettings.nowHour++;
        // синхронизация времени раз в час
        globCurrSettings.nowSecond = globDS3231.getSeconds();
        globCurrSettings.nowMinute = globDS3231.getMinutes();
        globCurrSettings.nowHour = globDS3231.getHours();
      }
      minuteControl();
    }
  }
};

// return it's day or nigth based on Morning and Evening
bool LoopTime::itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes) {
  if (_eveningInMinutes > _morningInMinutes) return (_nowInMinutes >= _morningInMinutes && _nowInMinutes < _eveningInMinutes);
  else return (_nowInMinutes >= _morningInMinutes || _nowInMinutes < _eveningInMinutes);
};

void LoopTime::minuteControl() {

  // local values
  int _nowInMinutes = global::timeInMinutes(globCurrSettings.nowHour, globCurrSettings.nowMinute);
  int _morningInMinutes = global::timeInMinutes(EEPROM.read(EEPROM_MORNING_HOUR), EEPROM.read(EEPROM_MORNING_MINUTE));
  int _eveningInMinutes = global::timeInMinutes(EEPROM.read(EEPROM_EVENING_HOUR), EEPROM.read(EEPROM_EVENING_MINUTE));


  // it's day or nigth
  globCurrSettings.nowDay = itsDay(_nowInMinutes, _morningInMinutes, _eveningInMinutes);

  // control alarm
  if (EEPROM.read(EEPROM_ALARM) == 1) {
    int _alarmInMinutes = global::timeInMinutes(EEPROM.read(EEPROM_ALARM_HOUR), EEPROM.read(EEPROM_ALARM_MINUTE));
    if (_nowInMinutes == _alarmInMinutes) {
      if (globCurrSettings.alarmMelody == nullptr) globCurrSettings.alarmMelody = new Melody();
      else globCurrSettings.alarmMelody->restart();
    }
  }

  // change control bubble settings
  int _morningBubbles = _morningInMinutes - EEPROM.read(EEPROM_BEFORE_MORNING_BUBBLE_START);
  if (_morningBubbles < 0) _morningBubbles += 1440;
  int _eveningBubbles = _eveningInMinutes - EEPROM.read(EEPROM_LAMP_INTERVAL) * 2;
  if (_eveningBubbles < 0) _eveningBubbles += 1440;
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
  globBubbleControl.set_currStatus(_needingStatus);
  globBubbleControl.set_bubblesIn100Second(_needingBubbleSpeed);

  // control lamps
  globLamps.scheduler();

  // control heater
  globControlTemp.scheduler();

  // control feeding
  globFeeding.scheduler(_nowInMinutes);

};
