/*
  ControlTemp.cpp - Программная реализация управления температурой в аквариуме  
*/

#include "ControlTemp.h"

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
