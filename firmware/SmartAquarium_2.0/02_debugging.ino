#if (DEBUG_MODE == 1)


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
  endSensorValues(_addIndex, 1, 0);
}

void debugCounterTick() {

  static int _intervalTime = 0;   // время оцениваемого интервала
  static int _intervalLevel = 0;  // изменение уровня оцениваемого интервала
  static int _countError0 = 0;  // счетчик ошибок "провалов" считывания
  static int _countError1 = 0;  // счетчик ошибок нехватки буфера обсчета
  static int _countError2 = 0;  // счетчик ошибок нулевого времени

  byte _changeTimeBubble = 5; // мин. время изменений для диагностики пузырька
  byte _changeLevelBubble = 10; // изменение уровня для диагностики пузырька
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

  while (_curIndex < BUFFER_SENSOR_SIZE) {

    // read changes of a values for test
    int __changeTime = _changeTime[_curIndex];
    int __changeLevel = _changeLevel[_curIndex] - 125;

    // COPY ALGORITM TO DEBUGING IN debugCounterTick()
    // increament interval
    _intervalTime = _intervalTime + _changeTime[_curIndex];
    _intervalLevel = _intervalLevel + _changeLevel[_curIndex] - 125;

    // changing the interval while it is longer that _changeTimeBubble
    while (_intervalTime > _changeTimeBubble && _intervalIndex != _curIndex) {
      _intervalTime = _intervalTime - _changeTime[_intervalIndex];
      _intervalLevel = _intervalLevel - _changeLevel[_intervalIndex] + 125;
      if (++_intervalIndex == BUFFER_SENSOR_SIZE) _intervalIndex = 0;
    }

    // checking Errors
    if (_intervalTime > _changeTimeBubble) ++_countError0;
    if (_changeTime[_curIndex] == 0) _countError2++;

    // checking level up to bubble
    if (_intervalLevel > _changeLevelBubble) {
      // starting Bubble interval
      _beginBubble = _currentTime;
      _events = _events | 0b00000100;
      // ending NoBubble interval
      if (_beginNoBubble != 0) {
        _durationNoBubble = _currentTime - _beginNoBubble;
        _beginNoBubble = 0;
      }
    }

    // checking level down from the bubble
    if (_intervalLevel < _changeLevelBubble * (-1)) {
      // starting NoBubble interval
      _beginNoBubble = _currentTime;
      _events = _events | 0b00001000;
      // ending Bubble interval
      if (_beginBubble != 0) {
        _durationBubble = _currentTime - _beginBubble;
        _beginBubble = 0;
        // counting
        //_lastTimeError = _currentTime;
        //_onTheBubble();
      }
    }

    Serial.print("_curIndex: "); Serial.print(_curIndex);
    Serial.print(",__changeTime: "); Serial.print(__changeTime);
    Serial.print(",__changeLevel: "); Serial.print(__changeLevel);
    Serial.print(",_intervalIndex: "); Serial.print(_intervalIndex);
    Serial.print(",_intervalTime: "); Serial.print(_intervalTime);
    Serial.print(",_intervalLevel: "); Serial.print(_intervalLevel);
    Serial.print(",_beginBubble: "); Serial.print(_beginBubble);
    Serial.print(",_beginNoBubble: "); Serial.println(_beginNoBubble);

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
}

void debugTemp() {
  Serial.print("aquaTemp: ");
  Serial.print(currSettings.aquaTemp);
  Serial.print(",aquaTempErr: ");
  Serial.println(currSettings.aquaTempErr);
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
