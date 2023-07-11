/*
  ControlTemp.h - Объект для управления температурой в аквариуме
  Днем поддерживает одну температуру, ночью другую.
  Все настройки температуры хранятся в EEPROM, адреса заданы в Global.h
  Считывает цифровой датчик температуры DS18B20
*/

#ifndef ControlTemp_h
#define ControlTemp_h

#include <EEPROM.h>
#include <Arduino.h>
#include <microDS18B20.h>
#include "TM1638My.h"
extern TM1638My gModule1638;

#include "Global.h"

// пин цифрового датчика температуры
#define DS18B20_PIN 7
// пин реле нагревателя
#define HEATER_PIN A0
// период считывания датчика
#define READ_TEMP_INTERVAL 600000 // 1 minute
// настрока попыток считать температуру датчика
#define ATTEMPT_INTERVAL 1000 // 1 second
#define MAX_ATTEMPTS 10

class ControlTemp {

  public:
    ControlTemp();
    bool getAquaTempConnected();
    float getAquaTemp();
    word getHeaterTempLog(byte _index);
    // проверка текущего состояния нагревателя от расписания и текущей температуры (берет из кэша)
    // вызывать при изменении текущих минут
    void scheduler(bool _nowDay, byte _nowMinute, byte _nowHour);
    // вызывать как можно чаще, функция считывания температуры
    // возвращает true если она изменилась, иначе false
    bool loopNeedDisplay();

  private:    
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

#endif
