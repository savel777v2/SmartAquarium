#define NUMBER_OF_NOTES 10
// frequency, duration
unsigned int alarmMelody[14][2] = {{2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 3000}};

// Initialize alarm
void alarmInitialize() {
  pinMode(PIEZO_PIN, OUTPUT); // настраиваем вывод 2 на выход
  tone(PIEZO_PIN, 2500, 100);
}

// play alarm melody if necessary
void alarmLoop() {
  static unsigned long _nextNoteTime = 0;
  static byte _iNote = 0;

  if (currSettings.alarmStartSound != 0) {
    if ((millis() - currSettings.alarmStartSound) >= 60000) {
      currSettings.alarmStartSound = 0;
      _nextNoteTime = 0;
      noTone(PIEZO_PIN);
    }
    else if (millis() > _nextNoteTime) {
      if (alarmMelody[_iNote][0] == 0) noTone(PIEZO_PIN);
      else tone(PIEZO_PIN, alarmMelody[_iNote][0]);
      _nextNoteTime = millis() + alarmMelody[_iNote][1];
      if (++_iNote == NUMBER_OF_NOTES) _iNote = 0;
    }
  }
  else if (_nextNoteTime != 0) {
    _nextNoteTime = 0;
    noTone(PIEZO_PIN);
  }

}

// on off Alarm on the schedule if necessary
void controlAlarm(int _nowInMinutes) {

  if (currSettings.alarmStartSound == 0) {
    if (EEPROM.read(6) == 1) {
      int _alarmInMinutes = (int)EEPROM.read(4) * 60 + EEPROM.read(5);
      if (_nowInMinutes == _alarmInMinutes) currSettings.alarmStartSound = millis();
    }
  }

}

// timer tick
bool timerLoopNeedDisplay() {
  static unsigned long _lastTimerTime = 0;

  bool _needDisplay = false;

  if (currSettings.timerOn && ((millis() - _lastTimerTime) > 1000)) {
    if (_lastTimerTime == 0) currSettings.timerSecond++; // first second compensation
    _lastTimerTime  = millis();
    if (currSettings.timerSecond == 0) {
      if (currSettings.timerMinute == 0) {
        currSettings.timerOn = false;
        currSettings.alarmStartSound = millis();
        _lastTimerTime  = 0;
        if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      }
      currSettings.timerSecond = 59;
      currSettings.timerMinute--;
    }
    else currSettings.timerSecond--;
    if ((currMode.main == 0) && (currMode.secondary == 1)) _needDisplay = true;
  }

  return _needDisplay;
}
