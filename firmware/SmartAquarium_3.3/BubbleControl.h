/*
  BubbleControl.h - Объект для управления расходом CO2 BubbleCounter.cpp
  через шаговый двигатель игольчатого редуктора StepMotor.h
  настройки расхода CO2 хранятся в EEPROM, адреса заданы в Global.h
*/

#ifndef BubbleControl_h
#define BubbleControl_h

#include <EEPROM.h>
#include <Arduino.h>
#include "Global.h"
#include "BubbleCounter.h"
extern BubbleCounter globBubbleCounter;
#include "StepMotor.h"
extern StepMotor globStepMotor;

class BubbleControl {
  public:

    void set_currStatus(byte currStatus);
    byte get_currStatus();
    String get_condition();
    void set_bubblesIn100Second(byte bubblesIn100Second);
    byte get_bubblesIn100Second();
    int get_minBubbleDuration();
    int get_maxBubbleDuration();
    word get_maxBubblesIn100Second();
    word get_minBubblesIn100Second();
    void clearError();
    bool controlWaiting();

  private:

    void _checkReturnPosition(); // возвращает на место после ошибки

    // 0 - отключен, 1 - включен, 2 - в процессе, 3 - результат достигнут, 4 - ошибка 1 (ничего не меняется), 5 - ошибка 2 (долго крутим), 6 - ошибка 3 (не попали в длительность)
    byte _currStatus = 0;
    byte _countError3 = 0; // счетчик повторов для ошибки 3
    int _lastPositionMove = 0;
    int _lastBubbleDuration = 0;
    int _moveNoResult = 0;
    int _moveOneWay = 0;
    int _minBubbleDuration = 10000;
    int _maxBubbleDuration = 10000;
    byte _bubblesIn100Second = 0;
    byte _bubblesAfterMotor = 0;

};

#endif
