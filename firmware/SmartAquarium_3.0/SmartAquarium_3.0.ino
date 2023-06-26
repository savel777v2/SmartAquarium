#include "TM1638My.h"
TM1638My Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 rtc; // A4 - SDA, A% - SCL

#include "CurrSettings.h"
CurrSettings currSettings;

#include "ControlTemp.h"

#define DS18B20_PIN 7

OneWire oneWire(DS18B20_PIN);
ControlTemp controlTemp(&oneWire, &Module);

#include "BubbleCounter.h"
BubbleCounter bubbleCounter;

#include "StepMotor.h"
StepMotor stepMotor(MOTOR_PIN_1, MOTOR_PIN_2, MOTOR_PIN_3, MOTOR_PIN_4);

#include "BubbleControl.h"
BubbleControl bubbleControl(&bubbleCounter, &stepMotor);

#include "Menu.h"
Menu menu(&Module, &controlTemp, &bubbleCounter, &stepMotor, &bubbleControl, &rtc, &currSettings);

#include "Lamps.h"
Lamps lamps(&currSettings);

#include "LoopTime.h"
LoopTime loopTime(&Module, &menu, &lamps, &controlTemp, &bubbleCounter, &stepMotor, &bubbleControl, &rtc, &currSettings);

void setup() {  
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
