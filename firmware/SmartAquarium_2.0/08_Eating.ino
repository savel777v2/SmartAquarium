
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
    _value = 20;
    EEPROM.update(30, _value);
  }
}

// turn on or turn off eating motor depending on the schedule or the button pressed
bool eatingLoopNeedDisplay() {
  static unsigned long _lastEatingTime = 0;
  static bool _eatingOn = false;
  static byte _lastButton = 0;

  bool _needDisplay = false;

  // Eating loop
  if (currSettings.eatingLoop != 0) {
    if (_lastEatingTime == 0 || (millis() - _lastEatingTime) > (unsigned long) (EEPROM.read(29) + EEPROM.read(30)) * 1000) {

      digitalWrite(EATING_PIN, HIGH);
      Module.setLED(1, 5);
      _eatingOn = true;
      _lastEatingTime = millis();
    }
    else if (_eatingOn && (millis() - _lastEatingTime) > (unsigned long) EEPROM.read(29) * 1000) {

      digitalWrite(EATING_PIN, LOW);
      Module.setLED(0, 5);
      _eatingOn = false;
      if (--currSettings.eatingLoop == 0) _lastEatingTime = 0;
      _needDisplay = true;
    }
  }

  /*byte _pressButton = digitalRead(EATING_BUTTON);
  if (_pressButton != _lastButton) {
    digitalWrite(EATING_PIN, _pressButton);
    Module.setLED(_pressButton, 5);
  }
  _lastButton = _pressButton;*/

  return _needDisplay;

}
