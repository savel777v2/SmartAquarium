/*
  Скетч к проекту "Умный аквариум"
  Исходники на GitHub: https://github.com/savel777v2/SmartAquarium
  Автор: Савельев Евгений, 2022
*/
// Версия 2.2

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

enum tempStatus {notBegin, readTemp, normal};

struct CurrSettings {
  DateTime now;  
  bool nowDay:1;
  bool timerOn:1;
  bool heaterOn:1;  
  tempStatus aquaTempStatus:2;
  byte setting:3;
  byte timerMinute;
  byte timerSecond;  
  unsigned long alarmStartSound;
  byte manualLamp;
  float aquaTemp;  
  int eatingLoop = 0;
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
