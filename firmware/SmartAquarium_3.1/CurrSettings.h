/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

#include "Melody.h"
#include "Timer.h"

#define DEBUG_MODE 0 // Отладка по COM порту
#define KEYBOARD_INTERVAL 100
#define DURATIONS_SIZE 4
#define SECOND_NOLED_DURATION 950

#define EEPROM_TIMER_MINUTE 7
#define EEPROM_TIMER_SECOND 8
#define EEPROM_MORNING_HOUR 0
#define EEPROM_MORNING_MINUTE 1
#define EEPROM_EVENING_HOUR 2
#define EEPROM_EVENING_MINUTE 3
#define EEPROM_ALARM_HOUR 4
#define EEPROM_ALARM_MINUTE 5
#define EEPROM_ALARM 6
#define EEPROM_LAMP_INTERVAL 9
#define EEPROM_DAY_TEMP 11
#define EEPROM_DAY_TEMP_ON 12
#define EEPROM_NIGHT_TEMP 13
#define EEPROM_NIGHT_TEMP_ON 14
#define EEPROM_DELTA_TEMP 15
#define EEPROM_MAX_DURATION_BUBBLE 20
#define EEPROM_MIN_LEVEL_BUBBLE 21
#define EEPROM_MOTOR_POSITION 22
#define EEPROM_DAY_BUBBLE_SPEED 23
#define EEPROM_DAY_BUBBLE_ON 24
#define EEPROM_NIGHT_BUBBLE_SPEED 25
#define EEPROM_NIGHT_BUBBLE_ON 26
#define EEPROM_BEFORE_MORNING_BUBBLE_START 27
#define EEPROM_CONTROL_BUBBLE_SOUND_ON 28
#define EEPROM_DAY_FEEDING_DURATION 29 // продолжительность цикла еды *10 в миллисекундах
#define EEPROM_DAY_FEEDING_PAUSE 30 // продолжительность простоя для цикла еды *10 в секундах
#define EEPROM_MORNING_FEEDING_HOUR 31
#define EEPROM_MORNING_FEEDING_MINUTE 32
#define EEPROM_MORNING_FEEDING_LOOP 33 // количество циклов еды через паузу
#define EEPROM_EVENING_FEEDING_HOUR 34
#define EEPROM_EVENING_FEEDING_MINUTE 35
#define EEPROM_EVENING_FEEDING_LOOP 36 // количество циклов еды через паузу
#define EEPROM_NIGHT_FEEDING_HOUR 37
#define EEPROM_NIGHT_FEEDING_MINUTE 38
#define EEPROM_NIGHT_FEEDING_DURATION 39 // продолжительность ночной еды *10 в миллисекундах (один цикл)

int timeInMinutes(const int _hour, const int _minutes) {
  return _hour * 60 + _minutes;
}

struct CurrSettings {
  byte nowSecond;
  byte nowMinute;
  byte nowHour;

  bool nowDay: 1;
  bool heaterOn: 1;
  bool secondLed: 1;

  Melody* alarmMelody; // проигрыш мелодии
  Timer* timer; // timer

  byte curDurations[DURATIONS_SIZE];
  byte printDurations[DURATIONS_SIZE];

  // if you want to get statistic of duration executing of the code
  //  add this in the begin of the code: startEndDurations(0)
  //  and this in the end of the code: startEndDurations(numberOfIndex)
  // there are four index off the durations (get max duration in second)
  //  which is display on the menu 'Y'
  void startEndDurations(byte toDo) {
    static unsigned long _beginTime;

    if (toDo) curDurations[toDo - 1] = max(curDurations[toDo - 1], millis() - _beginTime);
    _beginTime = millis();

  }

};
