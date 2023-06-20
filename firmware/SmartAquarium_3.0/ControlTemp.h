/*
  ControlTemp.h -


*/
#pragma once

enum tempStatus {notBegin, readTemp, normal};

#define HEATER_PIN A0
#define MAX_ATTEMPTS A0
#define READ_TEMP_INTERVAL 600000 // 1 minute
#define ATTEMPT_INTERVAL 1000 // 1 second

#include "TM1638My.h"
#include <OneWire.h>
#include <DS18B20.h>
#include <EEPROM.h>

class ControlTemp : public DS18B20 {

  public:
    ControlTemp(OneWire* _oneWire, TM1638My* _module);
    tempStatus getAquaTempStatus();
    float getAquaTemp();
    void heaterOnOff(bool _nowDay, byte _nowMinute, byte _nowHour);
    bool readTemperatureNeedDisplay();

  private:
    TM1638My* module;
    bool heaterOn;
    tempStatus aquaTempStatus;
    float aquaTemp;
    // логи температуры и работы нагревателя. температуры -100.0 до + 100.0 ложится в интервал 0 - 2000.
    // датчик нагревателя включен: + 10000. Температура -100.0 - это ошибка датчика температуры или отсутствие значения
    word heaterTempLog[24];
    unsigned long lastLoopTime;
    byte attemptCounter;
};

ControlTemp::ControlTemp(OneWire* _oneWire, TM1638My* _module) : DS18B20(_oneWire) {
  module = _module;
  pinMode(HEATER_PIN, OUTPUT);
  module->setLED(0, 4);
  digitalWrite(HEATER_PIN, HIGH); // false - off
  heaterOn = false;
  aquaTempStatus = notBegin;
  aquaTemp = 0;
  for (auto& valueLog : heaterTempLog) valueLog = 0;
  lastLoopTime = 0;
  attemptCounter = 0;
};

tempStatus ControlTemp::getAquaTempStatus() {
  return aquaTempStatus;
}

float ControlTemp::getAquaTemp() {
  return aquaTemp;
}

void ControlTemp::heaterOnOff(bool _nowDay, byte _nowMinute, byte _nowHour) {

  byte needingTemp;
  bool heaterAlwaysOff = false;
  if (_nowDay) {
    needingTemp = EEPROM.read(EEPROM_DAY_TEMP);
    if (EEPROM.read(EEPROM_DAY_TEMP_ON) == 0) heaterAlwaysOff = true;
  }
  else {
    needingTemp = EEPROM.read(EEPROM_NIGHT_TEMP);
    if (EEPROM.read(EEPROM_NIGHT_TEMP_ON) == 0) heaterAlwaysOff = true;
  }

  byte deltaTemp = EEPROM.read(EEPROM_DELTA_TEMP);
  float minTemp = needingTemp - (float)deltaTemp / 10;
  float maxTemp = needingTemp + (float)deltaTemp / 10;
  if (heaterOn && (heaterAlwaysOff || aquaTempStatus != normal || aquaTemp >= maxTemp)) {
    heaterOn = false;
    module->setLED(0, 4);
    digitalWrite(HEATER_PIN, HIGH);
  }
  else if (!heaterOn && !heaterAlwaysOff && aquaTempStatus == normal && aquaTemp <= minTemp) {
    heaterOn = true;
    module->setLED(1, 4);
    digitalWrite(HEATER_PIN, LOW);
  }

  // heaterTempLog each our
  if (_nowMinute == 0) {
    byte _indexOfLog = _nowHour;
    word _logValue;
    if (aquaTempStatus != normal) _logValue = 0;
    else _logValue = (float)aquaTemp * 10 + 1000;
    if (heaterOn) _logValue = _logValue + 10000;
    heaterTempLog[_indexOfLog] = _logValue;
  }

};

// read temperature from sensor in asynchronous mode
// ATTENTION read once getTempC() is longer then 8 ms
bool ControlTemp::readTemperatureNeedDisplay() {

  unsigned long _currentTime = millis();

  switch (attemptCounter) {
    case 0:
      // request temperature once in minute
      if ((_currentTime - lastLoopTime) > READ_TEMP_INTERVAL) {
        lastLoopTime = _currentTime;
        requestTemperatures();
        attemptCounter = 1;
      }
      break;
    case MAX_ATTEMPTS:
      // waiting minute after error
      if (lastLoopTime == 0 || (_currentTime - lastLoopTime) > READ_TEMP_INTERVAL) {
        if (aquaTempStatus == notBegin) {
          if (begin()) aquaTempStatus = readTemp;
        }
        if (aquaTempStatus == readTemp) {
          requestTemperatures();
          attemptCounter = 1;
        }
        lastLoopTime = _currentTime;
      }
      break;
    default:
      // try read Temperature each ATTEMPT_INTERVAL
      if ((_currentTime - lastLoopTime) > ATTEMPT_INTERVAL) {
        lastLoopTime = _currentTime;
        if (isConversionComplete()) {
          aquaTempStatus = normal;
          aquaTemp = getTempC();
          attemptCounter = 0;
          return true;
        }
        else if (++attemptCounter == MAX_ATTEMPTS) {
          aquaTempStatus = readTemp; // read temp
          return true;
        }
      }
      break;
  }

  return false;
};
