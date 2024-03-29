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

  heaterInitialize();
  
  eatingInitialize();

  initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);
  EditingMenuItemPart.set_isNull(1);

  // initialization settinggs BubbleCounter from EEPROM
  unsigned long _value4 = EEPROM.read(16);
  if (_value4 != 4294967295) CounterForBubbles.set_bubbleCounter(_value4);
  byte _value1;
  _value1 = EEPROM.read(20);
  // _maxDurationBubble
  if (_value1 == 255) {
    _value1 = CounterForBubbles.get_maxDurationBubble();
    EEPROM.update(20, _value1);
  }
  else CounterForBubbles.set_maxDurationBubble(_value1);
  // _maxDurationBubble
  _value1 = EEPROM.read(21);
  if (_value1 == 255) {
    _value1 = CounterForBubbles.get_minLevelBubble();
    EEPROM.update(21, _value1);
  }
  else CounterForBubbles.set_minLevelBubble(_value1);

}

void startEndDurations(byte toDo) {
  static unsigned long _beginTime;

  if (toDo == 0) {
    _beginTime = millis();
    return;
  }

  unsigned long _endTime = millis();
  byte _duration = _endTime - _beginTime;

  switch (toDo) {
    case 1: if (_duration > durations.max1) durations.max1 = _duration; break;
    case 2: if (_duration > durations.max2) durations.max2 = _duration; break;
    case 3: if (_duration > durations.max3) durations.max3 = _duration; break;
    case 4: if (_duration > durations.max4) durations.max4 = _duration; break;
  }
}

void loop() {
  bool _needDisplay = false;

  _needDisplay = _needDisplay || loopTimeNeedDisplay();
  CounterForBubbles.tick();
  
  _needDisplay = _needDisplay || readTemperatureNeedDisplay();
  CounterForBubbles.tick();

  _needDisplay = _needDisplay || eatingLoopNeedDisplay();
  CounterForBubbles.tick();
  
  if (_needDisplay) {
    printDisplay();
    CounterForBubbles.tick();
  }
  
  readKeyboard();
  CounterForBubbles.tick();

  StepMotorBubbles.tick();
  CounterForBubbles.tick();
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
  heaterOnOff();

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

// main procedure for control time
bool loopTimeNeedDisplay() {
  static unsigned long _lastLoopTime = 0;
  static unsigned long _lastTimerTime = 0;
  static unsigned long _nextNoteTime = 0;  
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

    // print duration
    durations.printMax1 = durations.max1;
    durations.printMax2 = durations.max2;
    durations.printMax3 = durations.max3;
    durations.printMax4 = durations.max4;
    durations.max1 = 0;
    durations.max2 = 0;
    durations.max3 = 0;
    durations.max4 = 0;
    if ((currMode.main == 3) && (currMode.secondary == 4)) _needDisplay = true;

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

  return _needDisplay;

}
