/*
  BubbleCounter.cpp - Программная реализация подсчета расхода CO2 через счетчик пузырьков  
*/

#include "BubbleCounter.h"

BubbleCounter::BubbleCounter() {  
  bubbleCounter = 0;
  working = true;
  lastSensorLevel = 0;
  lastTime = nextTimeMinMaxLevel = nextTimeError = nextTimeSecond = 0;
  pinMode(LASER_PIN, OUTPUT);
  setWorking(working);  
  for (auto& delta : deltaLevel) delta = 125;
  for (auto& delta : deltaTime) delta = 0;
  intervalTime = intervalLevel = 0;
  beginInterval = endInterval = 0;
  bubblePart = 0;
  tempDurationBubble = tempDurationNoBubble = durationBubble = durationNoBubble = 0;
  tempSensorInSecond = tempError0InSecond = tempError1InSecond = sensorInSecond = error0InSecond = error1InSecond = 0;  
  for (auto& duration : lastDurations) duration = -1;
  indexDuration = 0;
}

void BubbleCounter::setBubbleCounter(unsigned long _bubbleCounter) {
  bubbleCounter = _bubbleCounter;
}

unsigned long BubbleCounter::getBubbleCounter() {
  return bubbleCounter;
}

void BubbleCounter::setWorking(bool _working) {
  working = _working;
  if (working) digitalWrite(LASER_PIN, HIGH);
  else digitalWrite(LASER_PIN, LOW);
}

bool BubbleCounter::getWorking() {
  return working;
}

int BubbleCounter::getMinLevel() {
  return MinLevel;
}

int BubbleCounter::getMaxLevel() {
  return MaxLevel;
}

byte BubbleCounter::getDeltaLevel(byte index) {
  return deltaLevel[index];
}

byte BubbleCounter::getDeltaTime(byte index) {
  return deltaTime[index];
}

byte BubbleCounter::getEndInterval() {
  return endInterval;
}

int BubbleCounter::getDurationBubble() {
  return durationBubble;
}

int BubbleCounter::getDurationNoBubble() {
  return durationNoBubble;
}

int BubbleCounter::getSensorInSecond() {
  return sensorInSecond;
}

int BubbleCounter::getError0InSecond() {
  return error0InSecond;
}

int BubbleCounter::getError1InSecond() {
  return error1InSecond;
}

int BubbleCounter::getBubbleIn100Second() {
  if (lastDurations[indexDuration] <= 0) return lastDurations[indexDuration];
  else return 100000 / lastDurations[indexDuration];
}

int BubbleCounter::getBubbleInMinute() {
  if (lastDurations[indexDuration] <= 0) return lastDurations[indexDuration];
  else return 60000 / lastDurations[indexDuration];
}

long BubbleCounter::getDuration() {
  return lastDurations[indexDuration];
}

bool BubbleCounter::itsRegularBubbles() {
  long _minDuration = lastDurations[0];
  long _maxDuration = lastDurations[0];

  for (auto& duration : lastDurations) {
    _maxDuration = max(_maxDuration, duration);
    _minDuration = min(_minDuration, duration);
  }
  if (_minDuration == -3 && _maxDuration == -3) return true;
  if (_minDuration < 0) return false;
  byte _delta;
  if (_minDuration > 5000) _delta = 20;
  else _delta = 10;
  if (_minDuration + _delta <= _maxDuration) return false;
  return true;
}

void BubbleCounter::writeLastDuration(long _duration, byte& ans) {
  // ошибка или пузырек
  ans = ans | 0b00010000;
  if (++indexDuration == BUFFER_LAST_DURATIONS) indexDuration = 0;
  lastDurations[indexDuration] = _duration;
}

void BubbleCounter::endingBubbleInterval(byte& ans) {
  tempDurationBubble = tempLevelBubble = levelBubble = bubblePart = 0;
  // погасить инидикатор
  ans = ans | 0b00001000;
};

// функция подсчета пузырьков
byte BubbleCounter::loopNeedDisplay() {

  // поразрядно: 0 - обсчет Min\Max, 1 - обсчет _InSecond, 2 - начало индикации, 3 - конец индикации, 4 - пузырек или ошибка
  byte ans = 0;

  // needing to refactoring
  if (!working) return ans;

  // it's a first loop
  unsigned long _currentTime = millis();
  if (lastTime == 0) {
    lastTime = _currentTime;
    lastSensorLevel = analogRead(ANALOG_PIN);
    return ans;
  }

  // read sensor once in millisecond
  if (_currentTime == lastTime) return ans;

  // local values
  int _newLevel = analogRead(ANALOG_PIN);

  // min max level four times in second
  tempMinLevel = min(_newLevel, tempMinLevel);
  tempMaxLevel = max(_newLevel, tempMaxLevel);
  if (_currentTime > nextTimeMinMaxLevel) {
    nextTimeMinMaxLevel = _currentTime + MIN_MAX_LEVEL_DURATION;
    MinLevel = tempMinLevel;
    MaxLevel = tempMaxLevel;
    tempMinLevel = 20000;
    tempMaxLevel = 0;
    ans = ans | 0b00000001;
  }

  // write changes of a values
  int _changeTime = _currentTime - lastTime;
  deltaTime[endInterval] = _changeTime > 255 ? 255 : _changeTime;
  int _changeLevel = _newLevel - lastSensorLevel + 125;
  deltaLevel[endInterval] = _changeLevel < 0 ? 0 : (_changeLevel > 255 ? 255 : _changeLevel);

  // COPY ALGORITM TO DEBUGING IN debugCounterTick()
  // increament interval
  intervalTime += deltaTime[endInterval];
  intervalLevel += deltaLevel[endInterval] - 125;
  tempDurationNoBubble += deltaTime[endInterval];

  // changing the interval while it is longer that CHANGE_TIME_BUBBLE
  while (intervalTime > CHANGE_TIME_BUBBLE && beginInterval != endInterval) {
    intervalTime -= deltaTime[beginInterval];
    intervalLevel -= deltaLevel[beginInterval] - 125;
    if (++beginInterval == BUFFER_SENSOR_SIZE) beginInterval = 0;
  }

  // checking Errors
  if (intervalTime > CHANGE_TIME_BUBBLE) ++tempError0InSecond;

  if (bubblePart == 0) {
    if (intervalLevel > CHANGE_LEVEL_BUBBLE) {
      // maybe Bubble interval
      bubblePart = 1;
      tempDurationBubble = intervalTime;
      tempLevelBubble = levelBubble = intervalLevel;
      // зажечь инидикатор      
      ans = ans | 0b00000100;
    }
  }
  else {
    // increament maybe Bubble interval
    tempDurationBubble += deltaTime[endInterval];
    tempLevelBubble += deltaLevel[endInterval] - 125;
    levelBubble = max(levelBubble, tempLevelBubble);
    if (tempDurationBubble > EEPROM.read(EEPROM_MAX_DURATION_BUBBLE)) {
      // Bubble too long
      writeLastDuration(-1, ans);
      endingBubbleInterval(ans);
    }
    else if (bubblePart == 1 && intervalLevel < CHANGE_LEVEL_BUBBLE * (-1)) bubblePart = 2;
    else if (bubblePart == 2 && intervalLevel >= CHANGE_LEVEL_BUBBLE * (-1) && intervalLevel <= CHANGE_LEVEL_BUBBLE) {
      // maybe end of s Bubble interval
      if (levelBubble > EEPROM.read(EEPROM_MIN_LEVEL_BUBBLE)) {
        durationBubble = tempDurationBubble;
        durationNoBubble = tempDurationNoBubble - tempDurationBubble;
        tempDurationNoBubble = 0;
        // counting
        bubbleCounter++;
        nextTimeError = _currentTime + 10000;
        writeLastDuration(durationNoBubble > 10000 ? -3 : (long) durationBubble + durationNoBubble , ans);
      }
      else {
        // Bubble too small
        writeLastDuration(-2, ans);
      }
      endingBubbleInterval(ans);
    }
  }

  // increament current index
  if (++endInterval == BUFFER_SENSOR_SIZE) endInterval = 0;
  if (endInterval == beginInterval) {
    ++tempError1InSecond;
    intervalTime -= deltaTime[beginInterval];
    intervalLevel -= deltaLevel[beginInterval] - 125;
    if (++beginInterval == BUFFER_SENSOR_SIZE) beginInterval = 0;
  }
  // END COPY ALGORITM

  // loop longer durations errors
  if (_currentTime > nextTimeError) {
    nextTimeError = _currentTime + 10000;
    if (tempDurationNoBubble > 10000) writeLastDuration(-3, ans);
  }

  // counting values for the next loop
  lastTime = _currentTime;
  lastSensorLevel = _newLevel;

  // one second loop
  if (_currentTime > nextTimeSecond) {
    nextTimeSecond += 1000;
    sensorInSecond = tempSensorInSecond;
    error0InSecond = tempError0InSecond;
    error1InSecond = tempError1InSecond;
    ans = ans | 0b00000010;
    tempSensorInSecond = tempError0InSecond = tempError1InSecond = 0;
  }
  else tempSensorInSecond++;

  return ans;
};
