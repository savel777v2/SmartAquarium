/*
  Lamps.h - Объект для управления светом прожекторов в аквариуме
  Все настройки управления хранятся в EEPROM, адреса заданы в Global.h
  На данный момент управляет 3 прожекторами - влючая их по утрам слева направо (рассвет),
  и выключая по вечерам также слева направо (закат). Также возможно ручное управление по кнопке.
*/

#ifndef Lamps_h
#define Lamps_h

#include <EEPROM.h>
#include <Arduino.h>

#include "Global.h"
extern global::CurrSettings globCurrSettings;

// Прожектора слева направо
#define LAMP1_PIN A1
#define LAMP2_PIN A2
#define LAMP3_PIN A3
// Продолжительность воздействия ручного управления по кнопке (после поять сваливается в расписание)
#define MANUAL_LAMP_DURATION 600000 // 10 минут

class Lamps {

  public:
    Lamps();
    // проверка режима света по расписанию
    // вызывать при изменении текущих минут
    void scheduler();
    // изменение ручного режима ламп - вызывать по нажатию кнопки
    void changeManualLamp();

  private:
    // ручной режим ламп: 0 - отсутствует, действует автомат, 1 - лампы включены, 0 - отключены
    byte manualLamp;
    unsigned long manualLampTimeOff;
};

#endif