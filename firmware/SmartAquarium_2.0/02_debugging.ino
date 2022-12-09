
// if you want to get statistic of duration executing of the code
//  add this in the begin of the code: startEndDurations(0)
//  and this in the end of the code: startEndDurations(numberOfIndex) 
// there are four index off the durations (get max duration in second)
//  which is display on the menu 'Y'
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

#if (DEBUG_MODE == 1)

/*
void addSensorValue(byte& _index, byte _time, byte _level) {
  _changeTime[_index] = _time;
  _changeLevel[_index] = _level + 125;
  _index++;
}

void endSensorValues(byte& _index, byte _time, byte _level) {
  while (_index < BUFFER_SENSOR_SIZE) {
    _changeTime[_index] = _time;
    _changeLevel[_index] = _level + 125;
    _index++;
  }
}

void addTest1() {
  byte _addIndex = 0;
  // first bubble
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 3, 3);
  addSensorValue(_addIndex, 1, 2);
  addSensorValue(_addIndex, 1, 3);
  addSensorValue(_addIndex, 1, 4);
  addSensorValue(_addIndex, 1, 7);
  addSensorValue(_addIndex, 1, 11);
  addSensorValue(_addIndex, 1, 12);
  addSensorValue(_addIndex, 1, 16);
  addSensorValue(_addIndex, 1, 7);
  addSensorValue(_addIndex, 1, -3);
  addSensorValue(_addIndex, 1, -18);
  addSensorValue(_addIndex, 1, -15);
  addSensorValue(_addIndex, 1, -2);
  addSensorValue(_addIndex, 1, -10);
  addSensorValue(_addIndex, 1, -6);
  addSensorValue(_addIndex, 1, -5);
  addSensorValue(_addIndex, 1, -1);
  addSensorValue(_addIndex, 1, -2);
  addSensorValue(_addIndex, 1, 1);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  // second bubble
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 10, 62);
  addSensorValue(_addIndex, 1, 12);
  addSensorValue(_addIndex, 1, -16);
  addSensorValue(_addIndex, 1, -16);
  addSensorValue(_addIndex, 1, -17);
  addSensorValue(_addIndex, 1, -10);
  addSensorValue(_addIndex, 1, -5);
  addSensorValue(_addIndex, 1, -3);
  addSensorValue(_addIndex, 1, -2);
  addSensorValue(_addIndex, 1, -2);
  addSensorValue(_addIndex, 1, -1);
  addSensorValue(_addIndex, 1, -2);
  addSensorValue(_addIndex, 1, 1);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  // third bubble
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 3);
  addSensorValue(_addIndex, 1, 3);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 5);
  addSensorValue(_addIndex, 1, 2);
  addSensorValue(_addIndex, 1, 6);
  addSensorValue(_addIndex, 1, 9);
  addSensorValue(_addIndex, 2, 8);
  addSensorValue(_addIndex, 1, 3);
  addSensorValue(_addIndex, 1, 15);
  addSensorValue(_addIndex, 1, 3);
  addSensorValue(_addIndex, 1, 5);
  addSensorValue(_addIndex, 1, 5);
  addSensorValue(_addIndex, 1, -32);
  addSensorValue(_addIndex, 2, -11);
  addSensorValue(_addIndex, 1, -5);
  addSensorValue(_addIndex, 1, -3);
  addSensorValue(_addIndex, 1, -2);
  addSensorValue(_addIndex, 1, -1);
  addSensorValue(_addIndex, 1, -1);
  addSensorValue(_addIndex, 1, -1);
  addSensorValue(_addIndex, 1, 0);
  addSensorValue(_addIndex, 1, 0);
  //ending
  endSensorValues(_addIndex, 1, 0);
}

void endingBubbleInterval(word& __durationBubble, int& __levelBubble, int& __maxLevelBubble, byte& __countSmoothLevel, byte& _events) {
  __durationBubble = 0;
  __levelBubble = 0;
  __maxLevelBubble = 0;
  __countSmoothLevel = 0;
  // погасить инидикатор
  _events = _events | 0b00000100;
}

void debugCounterTick() {

  static int _intervalTime = 0;   // время оцениваемого интервала
  static int _intervalLevel = 0;  // изменение уровня оцениваемого интервала
  static int _countError0 = 0;  // счетчик ошибок "провалов" считывания
  static int _countError1 = 0;  // счетчик ошибок нехватки буфера обсчета
  static int _countError2 = 0;  // счетчик ошибок нулевого времени

  // параметры диагностики пузырька
  byte _changeTimeBubble = 5; // подъем\спуск пузырька (длительность в мс.)
  byte _changeLevelBubble = 10; // подъем\спуск пузырька (изменение уровня)
  byte _maxDurationBubble = 30; // макс. длительность пузырька
  byte _minLevelBubble = 30; // мин. уровень пузырька

  // диагностика пузырька
  word __durationBubble = 0; // продолжительность интервала пузыря
  word __durationNoBubble = 0; // продолжительность интервала простоя
  int __levelBubble = 0; // уровень пузыря
  int __maxLevelBubble = 0; // уровень пузыря
  byte __countSmoothLevel = 0; // счетчик ровного уровня

  unsigned long _beginBubble = 0; // начало регистрации пузырька
  unsigned long _beginNoBubble = 0; // начало регистрации без пузырька

  unsigned long _bubbleCounter = 0; // счетчик пузырьков
  word _durationBubble = 0; // продолжительность интервала пузыря
  word _durationNoBubble = 0; // продолжительность интервала простоя


  addTest1();
  byte _curIndex = 0;

  // local values
  unsigned long _currentTime = 100;
  // поразрядно: 0 - обсчет Min\Max, 1 - обсчет _sensorInSecond, 2 - начало пузырька, 3 - конец пузырька, 4 - ошибка сигнала
  byte _events = 0;

  Serial.print("_curIndex: "); Serial.print("_cI: ");
  Serial.print(",__changeTime: "); Serial.print("_cT: ");
  Serial.print(",__changeLevel: "); Serial.print("_cL: ");
  Serial.print(",_intervalIndex: "); Serial.print("_iI: ");
  Serial.print(",_intervalTime: "); Serial.print("_iT: ");
  Serial.print(",_intervalLevel: "); Serial.println("_iL: ");
  Serial.print(",__durationBubble: "); Serial.print("__dB: ");
  Serial.print(",__durationNoBubble: "); Serial.print("__dNB: ");
  Serial.print(",__levelBubble: "); Serial.print("__lB: ");
  Serial.print(",__maxLevelBubble: "); Serial.print("__mLB: ");
  Serial.print(",__countSmoothLevel: "); Serial.print("__cSL: ");
  Serial.print(",_bubbleCounter: "); Serial.print("_bC: ");
  Serial.print(",_durationBubble: "); Serial.print("_dB: ");
  Serial.print(",_durationNoBubble: "); Serial.println("_dNB: ");

  while (_curIndex < BUFFER_SENSOR_SIZE) {

    // read changes of a values for test
    int __changeTime = _changeTime[_curIndex];
    int __changeLevel = _changeLevel[_curIndex] - 125;

    // COPY ALGORITM TO DEBUGING IN debugCounterTick()
    // increament interval
    _intervalTime = _intervalTime + _changeTime[_curIndex];
    _intervalLevel = _intervalLevel + _changeLevel[_curIndex] - 125;
    __durationNoBubble = __durationNoBubble + _changeTime[_curIndex];

    // changing the interval while it is longer that _changeTimeBubble
    while (_intervalTime > _changeTimeBubble && _intervalIndex != _curIndex) {
      _intervalTime = _intervalTime - _changeTime[_intervalIndex];
      _intervalLevel = _intervalLevel - _changeLevel[_intervalIndex] + 125;
      if (++_intervalIndex == BUFFER_SENSOR_SIZE) _intervalIndex = 0;
    }

    // checking Errors
    if (_intervalTime > _changeTimeBubble) ++_countError0;

    if (__durationBubble == 0) {
      if (_intervalLevel > _changeLevelBubble) {
        // maybe Bubble interval
        __durationBubble = _intervalTime;
        __levelBubble = _intervalLevel;
        __maxLevelBubble = _intervalLevel;
        // зажечь инидикатор
        _events = _events | 0b00000100;
      }
    }
    else {
      // increament maybe Bubble interval
      __durationBubble = __durationBubble + _changeTime[_curIndex];
      __levelBubble = __levelBubble + _changeLevel[_curIndex] - 125;
      if (__levelBubble > __maxLevelBubble) __maxLevelBubble = __levelBubble;
      if (__durationBubble > _maxDurationBubble) {
        // Bubble too long
        endingBubbleInterval(__durationBubble, __levelBubble, __maxLevelBubble, __countSmoothLevel, _events);
      }
      else if (_intervalLevel >= _changeLevelBubble * (-1) && _intervalLevel <= _changeLevelBubble) {
        // maybe end of s Bubble interval
        if (++__countSmoothLevel > 1) {
          if (__maxLevelBubble > _minLevelBubble) {
            _durationBubble = __durationBubble;
            _durationNoBubble = __durationNoBubble - __durationBubble;
            __durationNoBubble = 0;
            // counting
            _events = _events | 0b00000100;
            ++_bubbleCounter;
            //_lastTimeError = _currentTime;
            //_onTheBubble();
          }
          endingBubbleInterval(__durationBubble, __levelBubble, __maxLevelBubble, __countSmoothLevel, _events);
        }
      }
      else __countSmoothLevel = 0;
    }

    Serial.print("_cI: "); Serial.print(_curIndex);
    Serial.print(",_cT: "); Serial.print(__changeTime);
    Serial.print(",_cL: "); Serial.print(__changeLevel);
    Serial.print(",_iI: "); Serial.print(_intervalIndex);
    Serial.print(",_iT: "); Serial.print(_intervalTime);
    Serial.print(",_iL: "); Serial.print(_intervalLevel);
    Serial.print(",__dB: "); Serial.print(__durationBubble);
    Serial.print(",__dNB: "); Serial.print(__durationNoBubble);
    Serial.print(",__lB: "); Serial.print(__levelBubble);
    Serial.print(",__mLB: "); Serial.print(__maxLevelBubble);
    Serial.print(",__cSL: "); Serial.print(__countSmoothLevel);
    Serial.print(",_bC: "); Serial.print(_bubbleCounter);
    Serial.print(",_dB: "); Serial.print(_durationBubble);
    Serial.print(",_dNB: "); Serial.println(_durationNoBubble);

    // increament current index
    //if (++_curIndex == BUFFER_SENSOR_SIZE) _curIndex = 0;
    ++_curIndex;
    if (_curIndex == _intervalIndex) {
      ++_countError1;
      _intervalTime = _intervalTime - _changeTime[_intervalIndex];
      _intervalLevel = _intervalLevel - _changeLevel[_intervalIndex] + 125;
      if (++_intervalIndex == BUFFER_SENSOR_SIZE) _intervalIndex = 0;
    }
    // END COPY ALGORITM

  }
}*/

void debugTemp() {
  Serial.print("aquaTemp: ");
  Serial.print(currSettings.aquaTemp);
  Serial.print(",aquaTempStatus: ");
  Serial.println(currSettings.aquaTempStatus);
}

void debugEditingMenuItemPart() {
  Serial.print(EditingMenuItemPart.get_typeOfPart());
  Serial.print(",isNull: ");
  Serial.print(EditingMenuItemPart.get_isNull());
  Serial.print(",value: ");
  Serial.println(EditingMenuItemPart.get_value());
}

void debugManualLamp() {
  Serial.print("manualLamp: ");
  Serial.print(currSettings.manualLamp);
  Serial.print(",nowDay: ");
  Serial.println(currSettings.nowDay);
}

void debugOnOffLamps(char _char[10], int _i, int _nowInMinutes, int _minutesLamp) {
  Serial.print(_char);
  Serial.print(" _i: ");
  Serial.print(_i);
  Serial.print(" _nowInMinutes: ");
  Serial.print(_nowInMinutes);
  Serial.print(" _minutesLamp: ");
  Serial.println(_minutesLamp);
}

void debugCounterForBubbles() {
  Serial.print("bubbleCounter: ");
  Serial.print(CounterForBubbles.get_bubbleCounter());
  Serial.print("bubbleIn100Second: ");
  Serial.print(CounterForBubbles.get_bubbleIn100Second());
  Serial.print("bubbleInMinute: ");
  Serial.print(CounterForBubbles.get_bubbleInMinute());
  Serial.print(",sensorInSecond: ");
  Serial.println(CounterForBubbles.get_sensorInSecond());
}

#endif
