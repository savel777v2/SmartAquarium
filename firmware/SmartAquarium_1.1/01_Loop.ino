MenuItemPart EditingMenuItemPart;

void setup() {

#if (DEBUG_MODE == 1)
  Serial.begin(9600);
  Serial.println("debugging");
#endif

  pinMode(PIEZO_PIN, OUTPUT); // настраиваем вывод 2 на выход
  tone(PIEZO_PIN, 2500, 100);
  for (int i = 0; i < 3; i++) {
    pinMode(lampPinsLevel[i][0], OUTPUT);
  }

  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, HIGH); // false - off

  pinMode(EATING_PIN, OUTPUT);
  digitalWrite(EATING_PIN, LOW); // off

  currSettings.aquaTempErr = !sensor.begin();
  if (!currSettings.aquaTempErr) sensor.setResolution(12);

  initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);
  EditingMenuItemPart.set_isNull(1);

  // initialization settinggs BubbleCounter from EEPROM
  unsigned long _value4 = EEPROM.read(16);
  if (_value4 != 4294967295) CounterForBubbles.set_bubbleCounter(_value4);
  byte _value1;
  _value1 = EEPROM.read(20);
  // bubbleVibration
  if (_value1 == 255) {
    _value1 = CounterForBubbles.get_bubbleVibration();
    EEPROM.update(20, _value1);
  }
  else CounterForBubbles.set_bubbleVibration(_value1);
  // minBubbleLevel
  _value1 = EEPROM.read(21);
  if (_value1 == 255) {
    _value1 = CounterForBubbles.get_minBubbleLevel();
    EEPROM.update(21, _value1 - 50);
  }
  else CounterForBubbles.set_minBubbleLevel(_value1 + 50);

}

void loop() {
  loopTime();
  readKeyboard();
  CounterForBubbles.tick();
  StepMotorBubbles.tick();
}

// return it's day or nigth based on Morning and Evening
bool itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes) {
  if (_eveningInMinutes > _morningInMinutes) return (_nowInMinutes >= _morningInMinutes && _nowInMinutes < _eveningInMinutes);
  else return (_nowInMinutes >= _morningInMinutes || _nowInMinutes < _eveningInMinutes);
}

// return it's day or nigth based on Morning and Evening
void checkingEatingLoop(byte needEatingLoop, int nowInMinutes, byte needHour, byte needMinute) {
  if (needEatingLoop == 0 || needEatingLoop == 255 || currSettings.eatingLoop > 0) return;
  if (((int)needHour * 60 + needMinute) != nowInMinutes) return;
  currSettings.eatingLoop = needEatingLoop;
}

// all options control
void conditionControl() {
  static unsigned long _manualLampTime = 0;
  
  // general values
  int _nowInMinutes = (int)currSettings.now.hour * 60 + currSettings.now.minute;
  int _morningInMinutes = (int)EEPROM.read(0) * 60 + EEPROM.read(1);
  int _eveningInMinutes = (int)EEPROM.read(2) * 60 + EEPROM.read(3);
  int _minutesBetweenLamps = EEPROM.read(9);

  // it's day or nigth
  currSettings.nowDay = itsDay(_nowInMinutes, _morningInMinutes, _eveningInMinutes);

  // starting eating loop
  checkingEatingLoop(EEPROM.read(33), _nowInMinutes, EEPROM.read(31), EEPROM.read(32));
  checkingEatingLoop(EEPROM.read(36), _nowInMinutes, EEPROM.read(34), EEPROM.read(35));

  // settings of a bubble speed  
  int _morningBubbles = _morningInMinutes - EEPROM.read(27);
  if (_morningBubbles < 0) _morningBubbles = _morningBubbles + 1440;
  int _eveningBubbles = _eveningInMinutes - _minutesBetweenLamps * 2;
  if (_eveningBubbles < 0) _eveningBubbles = _eveningBubbles + 1440;
  byte _needingBubbleSpeed;
  byte _needingStatus;
  if (itsDay(_nowInMinutes, _morningBubbles, _eveningBubbles)) {
    _needingBubbleSpeed = EEPROM.read(23);
    _needingStatus = EEPROM.read(24);
  }
  else {
    _needingBubbleSpeed = EEPROM.read(25);
    _needingStatus = EEPROM.read(26);
  }
  BubbleSpeedControl.set_currStatus(_needingStatus);
  BubbleSpeedControl.set_bubblesIn100Second(_needingBubbleSpeed);

  // starting alarm
  if (currSettings.alarmStartSound == 0) {
    if (EEPROM.read(6) == 1) {
      int _alarmInMinutes = (int)EEPROM.read(4) * 60 + EEPROM.read(5);
      if (_nowInMinutes == _alarmInMinutes) currSettings.alarmStartSound = millis();
    }
  }

  // control off heater
  byte needingTemp;
  bool heaterAlwaysOff = false;
  if (currSettings.nowDay) {
    needingTemp = EEPROM.read(11);
    if (EEPROM.read(12) == 0) heaterAlwaysOff = true;
  }
  else {
    needingTemp = EEPROM.read(13);
    if (EEPROM.read(14) == 0) heaterAlwaysOff = true;
  }
  byte deltaTemp = EEPROM.read(15);
  float minTemp = needingTemp - (float)deltaTemp / 10;
  float maxTemp = needingTemp + (float)deltaTemp / 10;
  if (currSettings.heaterOn && (heaterAlwaysOff || currSettings.aquaTempErr || currSettings.aquaTemp >= maxTemp)) {
    currSettings.heaterOn = false;
    Module.setLED(0, 4);
    digitalWrite(HEATER_PIN, HIGH);
  }
  else if (!currSettings.heaterOn && !heaterAlwaysOff && currSettings.aquaTemp <= minTemp) {
    currSettings.heaterOn = true;
    Module.setLED(1, 4);
    digitalWrite(HEATER_PIN, LOW);
  }

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

  // heaterTempLog each 15 minutes
  if (currSettings.now.minute % 15 == 0) {

    byte _indexOfLog = currSettings.now.hour * 4;
    if (currSettings.now.minute >= 45) _indexOfLog = _indexOfLog + 3;
    else if (currSettings.now.minute >= 30) _indexOfLog = _indexOfLog + 2;
    else if (currSettings.now.minute >= 15) _indexOfLog = _indexOfLog + 1;

    word _logValue;
    if (currSettings.aquaTempErr) _logValue = 0;
    else _logValue = (float)currSettings.aquaTemp * 10 + 1000;
    if (currSettings.heaterOn) _logValue = _logValue + 10000;

    heaterTempLog[_indexOfLog] = _logValue;

  }

}

// main procedure for control time
void loopTime() {
  static unsigned long _lastLoopTime = 0;
  static unsigned long _lastTempTime = 0;
  static bool _waitingTemp = false;
  static unsigned long _intWaitingTemp = 0;
  static unsigned long _lastTimerTime = 0;
  static unsigned long _nextNoteTime = 0;
  static unsigned long _lastEatingTime = 0;  
  static byte _iNote = 0;
  bool _needDisplay = false;

  // Loop once First time
  if (_lastLoopTime == 0) {
    currSettings.now = Rtc.getTime();
    conditionControl();
    _lastLoopTime  = millis();
    _needDisplay = true;
  }

  // Alarm sound loop
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

  // Eating loop
  if (currSettings.eatingLoop != 0) {
    bool holdOnEating = false;
    unsigned long _endTime;
    if (_lastEatingTime == 0) holdOnEating = true;
    else if (currSettings.eatingOn) {
      _endTime = EEPROM.read(29) * 1000;
      if ((millis() - _lastEatingTime) > _endTime) {
        digitalWrite(EATING_PIN, LOW);
        Module.setLED(0, 5);
        currSettings.eatingOn = false;
        if (--currSettings.eatingLoop == 0) _lastEatingTime = 0;
        else _lastEatingTime = millis();
        _needDisplay = true;
      }
    }
    else {
      _endTime = EEPROM.read(30) * 1000;
      if ((millis() - _lastEatingTime) > _endTime) holdOnEating = true;
    }
    if (holdOnEating) {
      digitalWrite(EATING_PIN, HIGH);
      Module.setLED(1, 5);
      currSettings.eatingOn = true;
      _lastEatingTime = millis();
    }
  }

  // request temperature
  if (!currSettings.aquaTempErr && !_waitingTemp && (millis() - _lastTempTime) > TEMP_RENEW_INTERVAL) {
    _lastTempTime = millis();
    sensor.requestTemperatures();
    _intWaitingTemp = 0;
    _waitingTemp = true;
  }

  // Renew temperature
  if (_waitingTemp && (millis() - _lastTempTime) > 10) {
    _lastTempTime = millis();
    if (sensor.isConversionComplete()) {
      currSettings.aquaTempErr = false;
      currSettings.aquaTemp = sensor.getTempC();
      _waitingTemp = false;
      if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
    }
    else if (_intWaitingTemp++ > 10) {
      currSettings.aquaTempErr = true;
      if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
    }
  }

  // Loop decrement timer
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

  // Loop blink Settings
  if (currSettings.setting > 0) {
    if ((millis() - settMode.lastBlinkTime) > 500) {
      settMode.lastBlinkTime  = millis();
      settMode.blinkOff = !settMode.blinkOff;
      _needDisplay = true;
    }
  }

  // Loop increment local time
  if ((millis() - _lastLoopTime) > 1000) {
    // секунда оттикала
    _lastLoopTime  = millis();
    currSettings.now.second++;
    if (currSettings.now.second == 60) {
      currSettings.now.second = 0;
      currSettings.now.minute++;
      if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      if (currSettings.now.minute == 60) {
        // синхронизация раз в час
        currSettings.now = Rtc.getTime();
      }
      conditionControl();
    }
  }

  if (_needDisplay) printDisplay();

}
