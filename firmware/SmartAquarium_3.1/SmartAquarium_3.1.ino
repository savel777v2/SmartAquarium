#include <EEPROM.h>

#include "TM1638My.h"
TM1638My module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 rtc; // A4 - SDA, A% - SCL

#include "CurrSettings.h"
CurrSettings currSettings;

#include "ControlTemp.h"
ControlTemp controlTemp(&module);

#include "BubbleCounter.h"
BubbleCounter bubbleCounter;

#include "StepMotor.h"
StepMotor stepMotor;

#include "BubbleControl.h"
BubbleControl bubbleControl(&bubbleCounter, &stepMotor);

#include "Feeding.h"
Feeding feeding;

#include "Lamps.h"
Lamps lamps(&currSettings);

#include "Menu.h"
Menu menu(&module, &controlTemp, &bubbleCounter, &stepMotor, &bubbleControl, &feeding, &rtc, &currSettings);

void setup() {
  currSettings.alarmMelody = nullptr;
  currSettings.timer = nullptr;
#if (DEBUG_MODE == 1)
  Serial.begin(9600);
  Serial.println("debugging");
#endif
  module.setDisplayToString("AQUA  30", B00000010, false);
  delay(2000);
  pinMode(PIEZO_PIN, OUTPUT); // настраиваем вывод 2 на выход
  tone(PIEZO_PIN, 2500, 100);
}

void loop() {
  bool res = menu.loopNeedControl();  

  if (res) {
    minuteControl();
  }
  readKeyboard();
  controlTempLoop();
  feedingLoop();  
  stepMotorLoop();
  timerLoop();
  melodyLoop();
  currSettings.startEndDurations(2);  
  
  loopTime();
  currSettings.startEndDurations(3);

  bubbleCounterControlLoop();
  currSettings.startEndDurations(4);

}
