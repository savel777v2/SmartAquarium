#include "TM1638My.h"
TM1638My Module(4, 5, 6); // DIO, CLK, STB

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
#if (DEBUG_MODE == 1)
  Serial.begin(9600);
  Serial.println("debugging");
#endif
}

void loop() {
  loopTime.loop();
  menu.loop();
}
