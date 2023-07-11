#include <EEPROM.h>

#include "TM1638My.h"
TM1638My globModule1638(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 globDS3231; // A4 - SDA, A% - SCL

#include "Global.h"
global::CurrSettings globCurrSettings;

#include "ControlTemp.h"
ControlTemp globControlTemp;

#include "BubbleCounter.h"
BubbleCounter globBubbleCounter;

#include "StepMotor.h"
StepMotor globStepMotor;

#include "BubbleControl.h"
BubbleControl globBubbleControl;

#include "Feeding.h"
Feeding globFeeding;

#include "Menu.h"
Menu globMenu(&globBubbleCounter, &globStepMotor, &globBubbleControl, &globFeeding, &globCurrSettings);

#include "Lamps.h"
Lamps globLamps;

#include "LoopTime.h"
LoopTime globLoopTime;

void setup() {
  pinMode(PIEZO_PIN, OUTPUT);
  globCurrSettings.alarmMelody = nullptr;
  globCurrSettings.timer = nullptr;  
#if (DEBUG_MODE == 1)
  Serial.begin(9600);
  Serial.println("debugging");
#endif
}

void loop() {
  if (globMenu.loopNeedControl()) globLoopTime.minuteControl();
  globLoopTime.readKeyboard();
  globLoopTime.loop();
}
