/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

#define BLINK_INTERVAL 500

#include "MenuItem.h"

enum submenu
{
  timeMenu, timer, morning, evening, alarm, lampInterval,
  curTemp, logTemp, dayTemp, nightTemp, deltaTemp,
  bubblesInSecond, bubbleControlSettings, sensorValue, bubbleSettings, bubbleDaySpeed, bubbleNightSpeed, beforeMorningStart, bubbleControlSound,
  feedingMenu, morningFeeding, eveningFeeding, dayFeedingSettings, nightFeeding, durations,
  motorPosition, motorSpeed, bubbleDurations, bubbleCount, bubblesInMinute, sensorInSecond, errorsInSecond,
  anon
};

struct MenuSettings {

  byte gorInd, verInd;
  byte numEditItem;
  MenuItem* subMenu[6];
  unsigned long lastBlinkTime;

};
