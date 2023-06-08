#include <TM1638.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 rtc; // A4 - SDA, A% - SCL

#include "CurrSettings.h"
CurrSettings currSettings;

#include "Menu.h"
Menu menu(&Module, &currSettings);

#include "LoopTime.h"
LoopTime loopTime(&menu, &rtc, &currSettings);

void setup() {
  currSettings.nowDay = false;
  currSettings.timerOn = true;
}

void loop() {
  loopTime.loop();
}
