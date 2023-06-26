/*
  Feeding.h -


*/
#pragma once

#define FEEDING_PIN 12
#define FEEDING_BUTTON_PIN 2 // + 5V

class Feeding {
  public:
    Feeding();
  private:
    int feedingLoop;    
    
}


// Initialize eating pins for motor and button
Feeding::Feeding() {
  pinMode(FEEDING_PIN, OUTPUT);
  digitalWrite(FEEDING_PIN, LOW); // off
  pinMode(FEEDING_BUTTON_PIN, INPUT);
  feedingLoop = 0;
}

// start Eating Loop on the schedule if necessary
void controlEatingLoop(int needEatingLoop, int nowInMinutes, byte needHour, byte needMinute) {
  if (needEatingLoop == 0 || needEatingLoop == 255 || currSettings.eatingLoop > 0) return;
  if (((int)needHour * 60 + needMinute) != nowInMinutes) return;
  currSettings.eatingLoop = needEatingLoop;
}

// turn on or turn off eating motor depending on the schedule or the button pressed
bool eatingLoopNeedDisplay() {
  static unsigned long _lastEatingTime = 0;
  static unsigned long _lastButtonTime = 0;
  static bool _eatingOn = false;
  static byte _lastButton = 0;

  bool _needDisplay = false;

  // Eating loop
  if (currSettings.eatingLoop != 0 || _eatingOn) {
    if ((millis() - _lastEatingTime) > (unsigned long) EEPROM.read(29) * 10 + (unsigned long) EEPROM.read(30) * 10000) {
      
      digitalWrite(FEEDING_PIN, HIGH);
      Module.setLED(1, 5);
      _eatingOn = true;
      _lastEatingTime = millis();
    }
    else if (_eatingOn && (millis() - _lastEatingTime) > (currSettings.eatingLoop >= 0 ? (unsigned long) EEPROM.read(29) * 10 : -currSettings.eatingLoop)) {

      digitalWrite(FEEDING_PIN, LOW);
      Module.setLED(0, 5);
      _eatingOn = false;
      if (currSettings.eatingLoop == 0) _lastEatingTime = 0;
      else if (currSettings.eatingLoop < 0) {
        currSettings.eatingLoop = 0;
        _lastEatingTime = 0;
      }
      else if (--currSettings.eatingLoop == 0) _lastEatingTime = 0;
      _needDisplay = true;
    }
  }

  // on button pressed
  if ((millis() - _lastButtonTime) > 100) {
    _lastButtonTime = millis();
    byte _pressButton = digitalRead(FEEDING_BUTTON_PIN);
    if (_pressButton != _lastButton && _pressButton == 1) {
      
      digitalWrite(FEEDING_PIN, _pressButton);
      Module.setLED(_pressButton, 5);
      _eatingOn = true;
      _lastEatingTime = _lastButtonTime;
    }
    _lastButton = _pressButton;
  }

  return _needDisplay;

}
