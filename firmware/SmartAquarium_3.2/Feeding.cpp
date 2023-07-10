/*
  Feeding.cpp - Программная реализация управления мельницей кормления
*/

#include "Feeding.h"

Feeding::Feeding() {
  pinMode(FEEDING_PIN, OUTPUT);
  digitalWrite(FEEDING_PIN, LOW); // off
  pinMode(FEEDING_BUTTON_PIN, INPUT);
  feedingLoop = 0;
  lastEatingTime = lastButtonTime = 0;
  feedingOn = false;
  lastButton = 0;
};

int Feeding::getFeedingLoop() {
  return feedingLoop;
};

void Feeding::setFeedingLoop(int _feedingLoop) {
  feedingLoop = _feedingLoop;
};

void Feeding::controlFeeding(int _nowInMinutes, int _feedingLoop, byte _needHour, byte _needMinute) {  
  if (_feedingLoop == 0 || _feedingLoop == 255 || feedingLoop > 0) return;
  if (global::timeInMinutes(_needHour,_needMinute) != _nowInMinutes) return;
  feedingLoop = _feedingLoop;
};

void Feeding::scheduler(int _nowInMinutes) {  
  controlFeeding(_nowInMinutes, EEPROM.read(EEPROM_MORNING_FEEDING_LOOP), EEPROM.read(EEPROM_MORNING_FEEDING_HOUR), EEPROM.read(EEPROM_MORNING_FEEDING_MINUTE));
  controlFeeding(_nowInMinutes, EEPROM.read(EEPROM_EVENING_FEEDING_LOOP), EEPROM.read(EEPROM_EVENING_FEEDING_HOUR), EEPROM.read(EEPROM_EVENING_FEEDING_MINUTE));
  controlFeeding(_nowInMinutes, EEPROM.read(EEPROM_NIGHT_FEEDING_DURATION)*-10, EEPROM.read(EEPROM_NIGHT_FEEDING_HOUR), EEPROM.read(EEPROM_NIGHT_FEEDING_MINUTE));
}

bool Feeding::loopNeedDisplay() {
  
  bool _needDisplay = false;

  // Eating loop
  if (feedingLoop != 0 || feedingOn) {
    if ((millis() - lastEatingTime) > (unsigned long) EEPROM.read(29) * 10 + (unsigned long) EEPROM.read(30) * 10000) {
      
      digitalWrite(FEEDING_PIN, HIGH);
      //Module.setLED(1, 5);
      feedingOn = true;
      lastEatingTime = millis();
    }
    else if (feedingOn && (millis() - lastEatingTime) > (feedingLoop >= 0 ? (unsigned long) EEPROM.read(29) * 10 : -feedingLoop)) {

      digitalWrite(FEEDING_PIN, LOW);
      //Module.setLED(0, 5);
      feedingOn = false;
      if (feedingLoop == 0) lastEatingTime = 0;
      else if (feedingLoop < 0) {
        feedingLoop = 0;
        lastEatingTime = 0;
      }
      else if (--feedingLoop == 0) lastEatingTime = 0;
      _needDisplay = true;
    }
  }

  // on button pressed
  if ((millis() - lastButtonTime) > KEYBOARD_INTERVAL) {
    lastButtonTime = millis();
    byte _pressButton = digitalRead(FEEDING_BUTTON_PIN);
    if (_pressButton != lastButton && _pressButton == 1) {
      
      digitalWrite(FEEDING_PIN, _pressButton);
      feedingOn = true;
      lastEatingTime = lastButtonTime;
    }
    lastButton = _pressButton;
  }

  return _needDisplay;

};
