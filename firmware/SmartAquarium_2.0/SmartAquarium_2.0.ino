/*
  Скетч к проекту "Умный аквариум"
  Исходники на GitHub: https://github.com/savel777v2/SmartAquarium
  Автор: Савельев Евгений, 2022
*/
// Версия 2.0

#define DEBUG_MODE 1 // Отладка по COM порту

#include <TM1638.h>
#include <EEPROM.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 Rtc; // A4 - SDA, A% - SCL

#define KEYBOARD_INTERVAL 10
#define PIEZO_PIN 3 // + ground
#define TURN_OFF_MANUAL_LAMP 120000 // at least on change minute
#define DS18B20_PIN 7
#define TEMP_RENEW_INTERVAL 200
#define HEATER_PIN A0
#define EATING_PIN 12

byte lampPinsLevel[][2] = {{A1, 0}, {A2, 0}, {A3, 0}}; // Pin from left to right, Level

#include <OneWire.h>
#include <DS18B20.h>

OneWire oneWire(DS18B20_PIN);
DS18B20 sensor(&oneWire);

struct CurrSettings {
  DateTime now;
  byte alarmOn;
  bool nowDay;
  bool timerOn;
  byte timerMinute;
  byte timerSecond;
  byte setting;
  unsigned long alarmStartSound;
  byte manualLamp;
  float aquaTemp;
  bool aquaTempErr;
  bool heaterOn = false;
  byte eatingLoop;
  bool eatingOn = false;
};

struct {
  byte main = 0;
  byte secondary = 0;
} currMode;

struct {
  bool blinkOff;
  unsigned long lastBlinkTime = 0;
} settMode;

CurrSettings currSettings;

// frequency, duration
#define NUMBER_OF_NOTES 10
unsigned int alarmMelody[14][2] = {{2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 3000}};

// логи температуры и работы нагревателя. температуры -100.0 до + 100.0 ложится в интервал 0 - 2000.
// датчик нагревателя включен: + 10000. Температура -100.0 - это ошибка датчика температуры или отсутствие значения
word heaterTempLog[24];
