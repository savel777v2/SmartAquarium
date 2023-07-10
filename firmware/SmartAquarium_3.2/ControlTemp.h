/*
  ControlTemp.h -


*/
#pragma once

#define DS18B20_PIN 7
#define HEATER_PIN A0
#define MAX_ATTEMPTS 10
#define READ_TEMP_INTERVAL 600000 // 1 minute
#define ATTEMPT_INTERVAL 1000 // 1 second

#include <microDS18B20.h>

class ControlTemp {

  public:
    ControlTemp(TM1638My* _module);
    bool getAquaTempConnected();
    float getAquaTemp();
    word getHeaterTempLog(byte _index);
    void scheduler(bool _nowDay, byte _nowMinute, byte _nowHour);
    bool loopNeedDisplay();

  private:
    TM1638My* module;
    MicroDS18B20<DS18B20_PIN> sensor;
    bool heaterOn;
    bool aquaTempConnected;
    float aquaTemp;
    // логи температуры и работы нагревателя. температуры -100.0 до + 100.0 ложится в интервал 0 - 2000.
    // датчик нагревателя включен: + 10000. Температура -100.0 - это ошибка датчика температуры или отсутствие значения
    word heaterTempLog[24];
    unsigned long nextLoopTime;
    byte attemptCounter;
};

ControlTemp::ControlTemp(TM1638My* _module) {
  module = _module;
  pinMode(HEATER_PIN, OUTPUT);
  module->setLED(0, 4);
  digitalWrite(HEATER_PIN, HIGH); // false - off
  heaterOn = false;
  aquaTempConnected = false;
  aquaTemp = 0;
  for (auto& valueLog : heaterTempLog) valueLog = 0;
  nextLoopTime = 0;
  attemptCounter = 0;
};

bool ControlTemp::getAquaTempConnected() {
  return aquaTempConnected;
}

word ControlTemp::getHeaterTempLog(byte _index) {
  return heaterTempLog[_index];
}

float ControlTemp::getAquaTemp() {
  return aquaTemp;
}

void ControlTemp::scheduler(bool _nowDay, byte _nowMinute, byte _nowHour) {

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
  if (heaterOn && (heaterAlwaysOff || !aquaTempConnected || aquaTemp >= maxTemp)) {
    heaterOn = false;
    module->setLED(0, 4);
    digitalWrite(HEATER_PIN, HIGH);
  }
  else if (!heaterOn && !heaterAlwaysOff && aquaTempConnected && aquaTemp <= minTemp) {
    heaterOn = true;
    module->setLED(1, 4);
    digitalWrite(HEATER_PIN, LOW);
  }

  // heaterTempLog each our
  if (_nowMinute == 0) {
    byte _indexOfLog = _nowHour;
    word _logValue;
    if (!aquaTempConnected) _logValue = 0;
    else _logValue = (float)aquaTemp * 10 + 1000;
    if (heaterOn) _logValue = _logValue + 10000;
    heaterTempLog[_indexOfLog] = _logValue;
  }

};

// read temperature from sensor in asynchronous mode
// requestTemp for each READ_TEMP_INTERVAL
// attempt readTemp for each ATTEMPT_INTERVAL
bool ControlTemp::loopNeedDisplay() {

  if (millis() < nextLoopTime) return false;

  if (attemptCounter == 0) {
    sensor.requestTemp();
    attemptCounter = 1;
    nextLoopTime = millis() + ATTEMPT_INTERVAL;
    return false;
  }
  else if (sensor.readTemp()) {
    aquaTempConnected = true; // success
    aquaTemp = sensor.getTemp();
    nextLoopTime = millis() + READ_TEMP_INTERVAL;
    attemptCounter = 0;
    return true;
  }
  else if (++attemptCounter == MAX_ATTEMPTS) {
    aquaTempConnected = false; // no connection
    nextLoopTime = millis() + READ_TEMP_INTERVAL;
    attemptCounter = 0;
    return true;
  }
  else {
    nextLoopTime = millis() + ATTEMPT_INTERVAL;
    return false;
  }

  return false;
};
