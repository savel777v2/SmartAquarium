/*
  Feeding.h - Объект для управления мельницей кормления рыб по расписанию.
  Все настройки кормления хранятся в EEPROM, адреса заданы в Global.h
*/

#ifndef Feeding_h
#define Feeding_h

#include <EEPROM.h>
#include <Arduino.h>
#include "Global.h"

// сюда подключен ключ двигателя прмого тока мельницы (один контакт)
#define FEEDING_PIN 12
// сюда подключена кнопка с мельницы для ручного кормления (подтягивать к 5 вольтам!)
#define FEEDING_BUTTON_PIN 2 // + 5V

class Feeding {
  public:
    Feeding();
    // текущий счетчик кормления (отрицательный - одно кормление в мс, 0 - нет кормежки, положительный - число кормлений с интервалом)
    int getFeedingLoop();
    void setFeedingLoop(int _feedingLoop);
    // проверка запуска кормления по всем расписаниям, использует controlFeeding для проверки конкретных расписаний
    // вызывать при изменении текущих минут
    void scheduler(int _nowInMinutes);
    // вызывать как можно чаще, кормит рыб, если установлен текущий счетчик кормления feedingLoop
    // возвращает true если он меняется и false иначе - для контроля отрисовки диалогов
    bool loopNeedDisplay();

  private:
    // текущий счетчик кормления (отрицательный - одно кормление в мс, 0 - нет кормежки, положительный - число кормлений с интервалом)
    int feedingLoop;
    unsigned long lastEatingTime, lastButtonTime;
    bool feedingOn;
    byte lastButton;
    // проверка запуска кормления по расписанию, вызывать при изменении текущих минут на различные расписания
    void controlFeeding(int _nowInMinutes, int _feedingLoop, byte _needHour, byte _needMinute);
};

#endif
