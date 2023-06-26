/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

#include <microDS3231.h>
#include "Melody.h"
#include "Timer.h"

#define DEBUG_MODE 0 // Отладка по COM порту
#define KEYBOARD_INTERVAL 100
#define SECOND_LED_DURATION 50
#define PIEZO_PIN 3 // + ground
#define ALARM_DURATION 60000

#define MOTOR_PIN_1 11
#define MOTOR_PIN_2 10
#define MOTOR_PIN_3 9
#define MOTOR_PIN_4 8

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

struct CurrSettings {
  DateTime now;

  bool nowDay: 1;  
  bool heaterOn: 1;
  bool secondLed: 1;

  Melody* alarmMelody; // проигрыш мелодии
  Timer* timer; // timer

  byte max1 = 0;
  byte max2 = 0;
  byte max3 = 0;
  byte max4 = 0;
  byte printMax1 = 0;
  byte printMax2 = 0;
  byte printMax3 = 0;
  byte printMax4 = 0;
};
