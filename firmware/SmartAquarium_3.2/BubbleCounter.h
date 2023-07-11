/*
  BubbleCounter.h - Объект для подсчета расхода CO2 через счетчик пузырьков
  настройки анализа сигнала пузырька хранятся в EEPROM, адреса заданы в Global.h
*/

#ifndef BubbleCounter_h
#define BubbleCounter_h

#include <EEPROM.h>
#include <Arduino.h>
#include "Global.h"

// пины лазера и фотоэлемента - пузырьки капают через "просвет"
#define LASER_PIN 2
#define ANALOG_PIN A7

#define BUFFER_SENSOR_SIZE 50
#define CHANGE_TIME_BUBBLE 5 // подъем\спуск пузырька (макс. длительность в мс.)
#define CHANGE_LEVEL_BUBBLE 20 // подъем\спуск пузырька (мин. изменение уровня)
#define BUFFER_LAST_DURATIONS 5
#define MIN_MAX_LEVEL_DURATION 250

class BubbleCounter {
  public:

    BubbleCounter();
    void setBubbleCounter(unsigned long _bubbleCounter);
    unsigned long getBubbleCounter();
    void setWorking(bool _working);
    bool getWorking();
    int getMinLevel();
    int getMaxLevel();
    byte getDeltaLevel(byte index);
    byte getDeltaTime(byte index);
    byte getEndInterval();
    int getDurationBubble();
    int getDurationNoBubble();
    int getSensorInSecond();
    int getError0InSecond();
    int getError1InSecond();
    
    int getBubbleIn100Second();
    int getBubbleInMinute();    
    long getDuration();
    bool itsRegularBubbles();    
    byte loopNeedDisplay();

  private:

    unsigned long bubbleCounter; // счетчик пузырьков
    bool working;
    int lastSensorLevel;
    // последнее время датчика, начало обсчета сенсор в секунду, последнее время цикла ошибки, последнее время цикла секунды
    unsigned long lastTime, nextTimeMinMaxLevel, nextTimeError, nextTimeSecond;
    // обсчет мин. и макс. уровней сигналов
    int tempMinLevel, tempMaxLevel, MinLevel, MaxLevel;    

    byte deltaLevel[BUFFER_SENSOR_SIZE]; // изменение уровня с последнего измерения 0 - мин. уровень 255 - макс. уровень. 125 - уровень 0
    byte deltaTime[BUFFER_SENSOR_SIZE];  // прирост мс. с последнего измерения, 255 - макс. уровень

    // оцениваемый интервал
    int intervalTime, intervalLevel;
    byte beginInterval, endInterval;

    // диагностика пузырька
    byte bubblePart; // индикатор части пузыря: 0 -нет, 1 - подъем был, 2 - спуск был
    word tempDurationBubble, tempDurationNoBubble, durationBubble, durationNoBubble; // продолжительность интервала пузыря, простоя    
    word tempLevelBubble, levelBubble; // уровень пузыря
    
    // счетчик считываний в секунду
    // счетчик ошибок "провалов" считывания
    // счетчик ошибок нехватки буфера обсчета
    int tempSensorInSecond, tempError0InSecond, tempError1InSecond, sensorInSecond, error0InSecond, error1InSecond;

    long lastDurations[BUFFER_LAST_DURATIONS];
    byte indexDuration;    
    
    void endingBubbleInterval(byte& ans);    
    void writeLastDuration(long _duration, byte& ans);
};

#endif
