#include <EEPROM.h>

#include "TM1638My.h"
TM1638My gModule1638(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 rtc; // A4 - SDA, A% - SCL

#include "Global.h"
global::CurrSettings currSettings;

#include "ControlTemp.h"
ControlTemp controlTemp;

#include "BubbleCounter.h"
BubbleCounter bubbleCounter;

#include "StepMotor.h"
StepMotor stepMotor;

#include "BubbleControl.h"
BubbleControl bubbleControl(&bubbleCounter, &stepMotor);

#include "Feeding.h"
Feeding feeding;

#include "Menu.h"
Menu menu(&gModule1638, &controlTemp, &bubbleCounter, &stepMotor, &bubbleControl, &feeding, &rtc, &currSettings);

#include "Lamps.h"
Lamps lamps;

#include "LoopTime.h"
LoopTime loopTime(&gModule1638, &menu, &lamps, &controlTemp, &bubbleCounter, &stepMotor, &bubbleControl, &feeding, &rtc, &currSettings);

void setup() {
  pinMode(PIEZO_PIN, OUTPUT);
  currSettings.alarmMelody = nullptr;
  currSettings.timer = nullptr;  
#if (DEBUG_MODE == 1)
  Serial.begin(9600);
  Serial.println("debugging");
#endif
}

void loop() {
  if (menu.loopNeedControl()) loopTime.minuteControl();
  loopTime.readKeyboard();
  loopTime.loop();
}
