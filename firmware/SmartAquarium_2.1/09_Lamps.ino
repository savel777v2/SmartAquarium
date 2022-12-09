#define TURN_OFF_MANUAL_LAMP 120000 // at least on change two minutes

// Initialize lamps
void lampsInitialize() {
  for (int i = 0; i < 3; i++) {
    pinMode(lampPinsLevel[i][0], OUTPUT);
  }
}

// on off lamps on the schedule if necessary
void controlLamps(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes) {
  static unsigned long _manualLampTime = 0;

  int _minutesBetweenLamps = 0;
  if (EEPROM.read(10) == 1) _minutesBetweenLamps = EEPROM.read(9);

  // turn off manual lamp
  if (currSettings.manualLamp > 0) {
    if (_manualLampTime == 0) _manualLampTime = millis();
    if ((millis() - _manualLampTime) >= TURN_OFF_MANUAL_LAMP) currSettings.manualLamp = 0;
  }

  // clear static values for manual lamp
  if (_manualLampTime > 0 && currSettings.manualLamp == 0) {
    _manualLampTime = 0;
    tone(PIEZO_PIN, 2500, 100);
  }

  // turn on\off flags of lamps
  if (currSettings.manualLamp == 0) {
    if (!currSettings.nowDay) {
      for (int i = 0; i < 3; i++) lampPinsLevel[i][1] = 0;
    }
    else if (EEPROM.read(10) == 0) {
      for (int i = 0; i < 3; i++) lampPinsLevel[i][1] = 1;
    }
    else {
      for (int i = 0; i < 3; i++) {
        // after morning
        int _minutesLamp = _morningInMinutes + _minutesBetweenLamps * i;
        lampPinsLevel[i][1] = 0;
        if (_nowInMinutes >= _morningInMinutes) {
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 1;
        }
        else if (_minutesLamp >= 1440) {
          _minutesLamp = _minutesLamp - 1440;
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 1;
        }
        else lampPinsLevel[i][1] = 1;
        // before evening
        _minutesLamp = _eveningInMinutes + _minutesBetweenLamps * (i - 2);
        if (_nowInMinutes < _eveningInMinutes) {
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 0;
        }
        else if (_minutesLamp < 0) {
          _minutesLamp = _minutesLamp + 1440;
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 0;
        }
      }
    }
  }

  // turn on\off lamps
  for (int i = 0; i < 3; i++) {
    if (lampPinsLevel[i][1] == 1) digitalWrite(lampPinsLevel[i][0], LOW);
    else digitalWrite(lampPinsLevel[i][0], HIGH);
  }
}
