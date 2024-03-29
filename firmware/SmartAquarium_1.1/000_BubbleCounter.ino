// universal class for bubble counting

class BubbleCounter {
  public:

    BubbleCounter(int laserPin, int analogPin, void (*function)(byte _events));
    void set_minBubbleLevel(int minBubbleLevel);
    int get_minBubbleLevel();
    void set_bubbleVibration(byte bubbleVibration);
    byte get_bubbleVibration();
    int get_sensorInSecond();
    void set_bubbleCounter(unsigned long bubbleCounter);
    unsigned long get_bubbleCounter();
    bool get_itsBubble();
    int get_bubbleIn100Second();
    int get_bubbleInMinute();
    int get_MinLevel();
    int get_MaxLevel();
    int get_durationBubble();
    int get_durationNoBubble();
    long get_minDurationLastBubbles();
    long get_maxDurationLastBubbles();
    long get_lastDuration();
    bool get_itsRegularBubbles();
    void tick();

  private:
    int _checkErrorBubble();
    int _checkErrorNoBubble();
    void _onTheBubble();

    int _laserPin, _analogPin;
    void (*_externalOnTheBubble)(byte _events); // значения _events:
    int _minBubbleLevel = 200; // мин уровень срабатывания пузырька
    byte _bubbleVibration = 5; // дребезг срабатывания пузырька в мс
    int _sensorInSecond = 0; // считываний сенсора в секунду
    unsigned long _bubbleCounter = 0; // счетчик пузырьков
    unsigned long _durationBubble = 0; // продолжительность интервала пузыря
    unsigned long _durationNoBubble = 0; // продолжительность интервала простоя
    long _beginBubble = 0; // начало регистрации пузырька
    long _beginNoBubble = 0; // начало регистрации без пузырька
    long _lastDurations[5] = { -1, -1, -1, -1, -1};
    byte _iLastDuration = 0;
    // мин. и макс. уровни сигналов
    int _MinLevel, _MaxLevel;
    bool _itsBubble = false; // флаг пролета пузырька


};

BubbleCounter::BubbleCounter(int laserPin, int analogPin, void (*function)(byte _events)) {
  _laserPin = laserPin;
  _analogPin = analogPin;
  _externalOnTheBubble = *function;
  pinMode(_laserPin, OUTPUT);
  digitalWrite(_laserPin, HIGH);
}

void BubbleCounter::set_minBubbleLevel(int minBubbleLevel) {
  _minBubbleLevel = minBubbleLevel;
}

int BubbleCounter::get_minBubbleLevel() {
  return _minBubbleLevel;
}

void BubbleCounter::set_bubbleVibration(byte bubbleVibration) {
  _bubbleVibration = bubbleVibration;
}

byte BubbleCounter::get_bubbleVibration() {
  return _bubbleVibration;
}

int BubbleCounter::get_sensorInSecond() {
  return _sensorInSecond;
}

void BubbleCounter::set_bubbleCounter(unsigned long bubbleCounter) {
  _bubbleCounter = bubbleCounter;
}

unsigned long BubbleCounter::get_bubbleCounter() {
  return _bubbleCounter;
}

bool BubbleCounter::get_itsBubble() {
  return _itsBubble;
}

int BubbleCounter::_checkErrorBubble() {
  // выход показателя за предельные значения
  if (_durationBubble > 50) return -1;
  else if (_itsBubble && (millis() - _beginBubble) > 50) return -1;
  else return 0;
}

int BubbleCounter::_checkErrorNoBubble() {
  // выход показателй за предельные значения
  if (_durationNoBubble < 30) return -2;
  else if (_durationNoBubble > 10000) return -3;
  else if (!_itsBubble && (millis() - _beginNoBubble) > 10000) {
    if (_durationNoBubble > 10000) return -3;
    else return -2;
  }
  else return 0;
}

int BubbleCounter::get_bubbleIn100Second() {
  int _checkError;
  _checkError = _checkErrorBubble();
  if (_checkError < 0) return _checkError;
  _checkError = _checkErrorNoBubble();
  if (_checkError < 0) return _checkError;
  if ((_durationBubble + _durationNoBubble) == 0) return 0;
  return 100000 / (_durationBubble + _durationNoBubble);
}

int BubbleCounter::get_bubbleInMinute() {
  int _checkError;
  _checkError = _checkErrorBubble();
  if (_checkError < 0) return _checkError;
  _checkError = _checkErrorNoBubble();
  if (_checkError < 0) return _checkError;
  if ((_durationBubble + _durationNoBubble) == 0) return 0;
  return 60000 / (_durationBubble + _durationNoBubble);
}

int BubbleCounter::get_MinLevel() {
  return _MinLevel;
}

int BubbleCounter::get_MaxLevel() {
  return _MaxLevel;
}

int BubbleCounter::get_durationBubble() {
  return _durationBubble;
}

int BubbleCounter::get_durationNoBubble() {
  return _durationNoBubble;
}

long BubbleCounter::get_minDurationLastBubbles() {
  long _minDuration = _lastDurations[0];
  for (int _i = 1; _i < 5; _i++) _minDuration = min(_minDuration, _lastDurations[_i]);
  return _minDuration;
}

long BubbleCounter::get_maxDurationLastBubbles() {
  long _maxDuration = _lastDurations[0];
  for (int _i = 1; _i < 5; _i++) _maxDuration = max(_maxDuration, _lastDurations[_i]);
  return _maxDuration;
}

long BubbleCounter::get_lastDuration() {
  return _lastDurations[_iLastDuration];
}

bool BubbleCounter::get_itsRegularBubbles() {
  long _minDuration = _lastDurations[0];
  long _maxDuration = _lastDurations[0];
  for (int _i = 1; _i < 5; _i++) {
    _maxDuration = max(_maxDuration, _lastDurations[_i]);
    _minDuration = min(_minDuration, _lastDurations[_i]);
  }
  if (_minDuration == -3 && _maxDuration == -3) return true;
  if (_minDuration < 0) return false;
  byte _delta;
  if (_minDuration > 5000) _delta = 20;
  else _delta = 10;
  if (_minDuration + _delta <= _maxDuration) return false;
  return true;
}

void BubbleCounter::_onTheBubble() {
  if (++_iLastDuration == 5) _iLastDuration = 0;
  int _checkError1 = _checkErrorBubble();
  int _checkError2 = _checkErrorNoBubble();
  if (_checkError1 < 0) _lastDurations[_iLastDuration] = _checkError1;
  else if (_checkError2 < 0) _lastDurations[_iLastDuration] = _checkError2;
  else {
    _lastDurations[_iLastDuration] = _durationBubble + _durationNoBubble;
    _bubbleCounter++;
  }
}

// функция подсчета пузырьков
void BubbleCounter::tick() {
  int _newLevel;
  static int _countSensor;
  unsigned long _currentTime; // текущее время считывания
  static unsigned long _beginMinMaxLevel; // начало обсчета сенсор в секунду
  static unsigned long _lastTimeSecond; // последнее время цикла секунды
  static unsigned long _lastTimeError; // последнее время цикла ошибки
  static unsigned long _lastTimeBubbleLevel; // последнее время уровня пузырька
  // обсчет мин. и макс. уровней сигналов
  static int _tempMinLevel, _tempMaxLevel;
  bool _externalFunction = false;
  byte _events = 0; // поразрядно: 0 - обсчет Min\Max, 1 - обсчет _sensorInSecond, 2 - начало пузырька, 3 - конец пузырька, 4 - ошибка сигнала

  _newLevel = analogRead(_analogPin);
  _currentTime = millis();

  // обсчет мин.\макс. уровень датчика - 2 раза в секунду
  _tempMinLevel = min(_newLevel, _tempMinLevel);
  _tempMaxLevel = max(_newLevel, _tempMaxLevel);
  if ((_currentTime - _beginMinMaxLevel) > 500) {
    _MinLevel = _tempMinLevel;
    _MaxLevel = _tempMaxLevel;
    _tempMinLevel = 20000;
    _tempMaxLevel = 0;
    _externalFunction = true;
    _events = _events | 0b00000001;
    _beginMinMaxLevel = _currentTime;
  }

  // анализ сигнала
  if (_newLevel > _minBubbleLevel) {
    _lastTimeBubbleLevel = _currentTime;
    if (_beginBubble == -1) {
      // 1.1 это новый пузырек
      _beginBubble = _currentTime;
      _itsBubble = true;
      _externalFunction = true;
      _events = _events | 0b00000100;
      // завершаем интервал без пузырька
      if (_beginNoBubble != -1) {
        _durationNoBubble = _currentTime - _beginNoBubble;
        _beginNoBubble = -1;
      }
    }
  }
  else {
    if (_beginBubble == -1) {
      // интервал без пузырька _beginBubble ==-1
      if (_beginNoBubble == -1) {
        // 2.1 начало интервала без пузырька - только при старте
        _beginNoBubble = _currentTime;
      }
    }
    else {
      if ((_currentTime - _lastTimeBubbleLevel) > _bubbleVibration) {
        // 2.3 время вибрации кончилось - пузырек завершен в мин. BubbleVibration.
        _durationBubble = _currentTime - _beginBubble;
        _beginBubble = -1;
        _itsBubble = false;
        _externalFunction = true;
        _events = _events | 0b00001000;
        // начало интервала без пузырька - при завершении пузырька
        _beginNoBubble = _currentTime;
        _lastTimeError = _currentTime;
        _onTheBubble();
      }
    }
  }

  // цикл ошибки длительности
  if ((_currentTime - _lastTimeError) > 10000) {
    _lastTimeError = _currentTime;
    if (_checkErrorBubble() < 0 || _checkErrorNoBubble() < 0) {
      _externalFunction = true;
      _events = _events | 0b00010000;
      _onTheBubble();
    }
  }

  // секундный цикл
  if ((_currentTime - _lastTimeSecond) > 1000) {
    _lastTimeSecond = _currentTime;
    // сколько считываний в секунду
    _sensorInSecond = _countSensor;
    _externalFunction = true;
    _events = _events | 0b00000010;
    _countSensor = 0;
  }
  else _countSensor++;

  // вызов внешней функции
  if (_externalFunction) (*_externalOnTheBubble)(_events);

}
