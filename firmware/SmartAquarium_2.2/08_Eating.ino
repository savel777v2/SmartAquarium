
// Initialize eating pins for motor and button
void eatingInitialize() {
  pinMode(EATING_PIN, OUTPUT);
  digitalWrite(EATING_PIN, LOW); // off

  pinMode(EATING_BUTTON, INPUT);

  byte _value;
  _value = EEPROM.read(29);
  if (_value == 255) {
    _value = 10;
    EEPROM.update(29, _value);
  }
  _value = EEPROM.read(30);
  if (_value == 255) {
    _value = 2;
    EEPROM.update(30, _value);
  }
}

// start Eating Loop on the schedule if necessary
void controlEatingLoop(byte needEatingLoop, int nowInMinutes, byte needHour, byte needMinute) {
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
    if (_lastEatingTime == 0 || (millis() - _lastEatingTime) > EEPROM.read(29) * 10 + EEPROM.read(30) * 60000) {
      
      digitalWrite(EATING_PIN, HIGH);
      Module.setLED(1, 5);
      _eatingOn = true;
      _lastEatingTime = millis();
    }
    else if (_eatingOn && (millis() - _lastEatingTime) > EEPROM.read(29) * 10) {

      digitalWrite(EATING_PIN, LOW);
      Module.setLED(0, 5);
      _eatingOn = false;
      if (currSettings.eatingLoop == 0) _lastEatingTime = 0;
      else if (--currSettings.eatingLoop == 0) _lastEatingTime = 0;
      _needDisplay = true;
    }
  }

  if ((millis() - _lastButtonTime) > 100) {
    _lastButtonTime = millis();
    byte _pressButton = digitalRead(EATING_BUTTON);
    if (_pressButton != _lastButton && _pressButton == 1) {
      
      digitalWrite(EATING_PIN, _pressButton);
      Module.setLED(_pressButton, 5);
      _eatingOn = true;
      _lastEatingTime = _lastButtonTime;
    }
    _lastButton = _pressButton;
  }

  return _needDisplay;

}
