/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

#include <microDS3231.h>

#define DEBUG_MODE 0 // Отладка по COM порту
#define EEPROM_ALARM 6
#define EEPROM_MORNING_HOUR 7
#define EEPROM_MORNING_MINUTE 8
#define EEPROM_EVENING_HOUR 9
#define EEPROM_EVENING_MINUTE 10

enum tempStatus {notBegin, readTemp, normal};

struct CurrSettings {
  DateTime now;

  bool nowDay: 1;
  bool timerOn: 1;
  bool heaterOn: 1;

  byte max1 = 0;
  byte max2 = 0;
  byte max3 = 0;
  byte max4 = 0;
  byte printMax1 = 0;
  byte printMax2 = 0;
  byte printMax3 = 0;
  byte printMax4 = 0;
};
