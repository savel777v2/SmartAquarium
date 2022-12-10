/*
  Скетч к проекту "Умный аквариум"
  Исходники на GitHub: https://github.com/savel777v2/SmartAquarium
  Автор: Савельев Евгений, 2022
*/
// Версия 2.0

#define DEBUG_MODE 0 // Отладка по COM порту

#include <TM1638.h>
#include <EEPROM.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 Rtc; // A4 - SDA, A% - SCL

#define PIEZO_PIN 3 // + ground
#define DS18B20_PIN 7
#define HEATER_PIN A0
#define EATING_PIN 12
#define EATING_BUTTON 2 // + 5V

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
  // 0 - not begin, 1 - read temp, 2 - normal
  byte aquaTempStatus = 0;
  bool heaterOn = false;
  byte eatingLoop = 0;
  byte waitingBubble = 0;
};

struct {
  byte main = 0;
  byte secondary = 0;
} currMode;

struct {
  bool blinkOff;
  unsigned long lastBlinkTime = 0;
} settMode;

struct {
  byte max1 = 0;
  byte max2 = 0;
  byte max3 = 0;
  byte max4 = 0;
  byte printMax1 = 0;
  byte printMax2 = 0;
  byte printMax3 = 0;
  byte printMax4 = 0;
} durations;

CurrSettings currSettings;

// логи температуры и работы нагревателя. температуры -100.0 до + 100.0 ложится в интервал 0 - 2000.
// датчик нагревателя включен: + 10000. Температура -100.0 - это ошибка датчика температуры или отсутствие значения
word heaterTempLog[24];
