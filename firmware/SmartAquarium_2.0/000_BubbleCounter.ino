// universal class for bubble counting

class BubbleCounter {
  public:

    BubbleCounter(int laserPin, int analogPin, void (*function)(byte _events));
    void set_changeLevelBubble(int changeLevelBubble);
    int get_changeLevelBubble();
    void set_changeTimeBubble(byte changeTimeBubble);
    byte get_changeTimeBubble();
    int get_sensorInSecond();
    int get_error0InSecond();
    int get_error1InSecond();
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

    byte _changeTimeBubble = 10; // мин. время изменений для диагностики пузырька
    byte _changeLevelBubble = 50; // изменение уровня для диагностики пузырька
    long _beginBubble = 0; // начало регистрации пузырька
    long _beginNoBubble = 0; // начало регистрации без пузырька
    bool _itsBubble = false; // флаг пролета пузырька
    
    int _sensorInSecond = 0, _error0InSecond = 0, _error1InSecond = 0;    
    int _MinLevel, _MaxLevel; // мин. и макс. уровни сигналов

    unsigned long _bubbleCounter = 0; // счетчик пузырьков
    word _durationBubble = 0; // продолжительность интервала пузыря
    word _durationNoBubble = 0; // продолжительность интервала простоя

    long _lastDurations[5] = { -1, -1, -1, -1, -1};
    byte _iLastDuration = 0;

};

BubbleCounter::BubbleCounter(int laserPin, int analogPin, void (*function)(byte _events)) {
  _laserPin = laserPin;
  _analogPin = analogPin;
  _externalOnTheBubble = *function;
  pinMode(_laserPin, OUTPUT);
  digitalWrite(_laserPin, HIGH);
}

void BubbleCounter::set_changeLevelBubble(int changeLevelBubble) {
  _changeLevelBubble = changeLevelBubble;
}

int BubbleCounter::get_changeLevelBubble() {
  return _changeLevelBubble;
}

void BubbleCounter::set_changeTimeBubble(byte changeTimeBubble) {
  _changeTimeBubble = changeTimeBubble;
}

byte BubbleCounter::get_changeTimeBubble() {
  return _changeTimeBubble;
}

int BubbleCounter::get_sensorInSecond() {
  return _sensorInSecond;
}

int BubbleCounter::get_error0InSecond() {
  return _error0InSecond;
}

int BubbleCounter::get_error1InSecond() {
  return _error1InSecond;
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

  static unsigned long _lastTime = 0;     // последнее время датчика
  static unsigned long _lastTimeMinMaxLevel; // начало обсчета сенсор в секунду
  static unsigned long _lastTimeError; // последнее время цикла ошибки
  static unsigned long _lastTimeSecond; // последнее время цикла секунды
  
  static int _lastLevel = 0;    // последний уровень датчика
  static byte _changeLevel[10]; // изменение уровня с последнего измерения 0 - мин. уровень 255 - макс. уровень. х-125 изменение уровня
  static byte _changeTime[10];  // прирост мс. с последнего измерения, 255 - макс. уровень
  static byte _curIndex = 0;    // тек. индекс записи  
  static byte _intervalIndex = 0;   // индекс начала интервала
  static int _intervalTime;     // время оцениваемого интервала
  static int _intervalLevel;    // изменение уровня оцениваемого интервала  
  static int _countLoop = 0;    // счетчик циклов
  static int _countError0 = 0;  // счетчик ошибок "провалов" считывания
  static int _countError1 = 0;  // счетчик ошибок нехватки буфера обсчета
  static int _tempMinLevel, _tempMaxLevel; // обсчет мин. и макс. уровней сигналов  

  // local values
  int _newLevel = analogRead(_analogPin);
  unsigned long _currentTime = millis();
  // поразрядно: 0 - обсчет Min\Max, 1 - обсчет _sensorInSecond, 2 - начало пузырька, 3 - конец пузырька, 4 - ошибка сигнала
  byte _events = 0;  

  // it's a first loop
  if (_lastTime == 0) {
    _lastTime = _currentTime;
    _lastLevel = _newLevel;
    return;
  }

  // min max level four times in second
  _tempMinLevel = min(_newLevel, _tempMinLevel);
  _tempMaxLevel = max(_newLevel, _tempMaxLevel);
  if ((_currentTime - _lastTimeMinMaxLevel) > 250) {
    _lastTimeMinMaxLevel = _currentTime;    
    _MinLevel = _tempMinLevel;
    _MaxLevel = _tempMaxLevel;
    _tempMinLevel = 20000;
    _tempMaxLevel = 0;    
    _events = _events | 0b00000001;    
  }

  // write changes of a values
  int __changeTime = _currentTime - _lastTime;
  if (__changeTime > 255) _changeTime[_curIndex] = 255;
  else _changeTime[_curIndex] = __changeTime;
  _intervalTime = _intervalTime + __changeTime;

  int __changeLevel = _newLevel - _lastLevel + 125;
  if (__changeLevel < 0) _changeLevel[_curIndex] = 0;
  else if (__changeLevel > 255) _changeLevel[_curIndex] = 255;
  else _changeLevel[_curIndex] = __changeLevel;
  _intervalLevel = _intervalLevel + __changeLevel - 125;

  // changing the interval while it is longer that _changeTimeBubble
  while (_intervalTime > _changeTimeBubble && _intervalIndex != _curIndex) {
    _intervalTime = _intervalTime - _changeTime[_intervalIndex];
    _intervalLevel = _intervalLevel - _changeLevel[_intervalIndex] + 125;
    if (++_intervalIndex == 10) _intervalIndex = 0;
  }

  // checking Errors 0
  if (_intervalTime > _changeTimeBubble) ++_countError0;

  // checking level up to bubble
  if (_intervalLevel > _changeLevelBubble && _beginBubble == -1) {
    // starting Bubble interval
    _beginBubble = _currentTime;
    _itsBubble = true;    
    _events = _events | 0b00000100;
    // ending NoBubble interval
    if (_beginNoBubble != -1) {
      _durationNoBubble = _currentTime - _beginNoBubble;
      _beginNoBubble = -1;
    }
  }

  // checking level down from the bubble
  if (_intervalLevel < _changeLevelBubble * (-1) && _beginBubble != -1) {
    // ending Bubble interval
    _durationBubble = _currentTime - _beginBubble;
    _beginBubble = -1;
    _itsBubble = false;    
    _events = _events | 0b00001000;
    // starting NoBubble interval
    _beginNoBubble = _currentTime;
    _lastTimeError = _currentTime;
    _onTheBubble();
  }

  // loop longer durations errors
  if ((_currentTime - _lastTimeError) > 10000) {
    _lastTimeError = _currentTime;
    if (_checkErrorBubble() < 0 || _checkErrorNoBubble() < 0) {      
      _events = _events | 0b00010000;
      _onTheBubble();
    }
  }

  // counting values for the next loop
  _lastTime = _currentTime;
  _lastLevel = _newLevel;
  if (++_curIndex == 10) _curIndex = 0;  
  if (_curIndex == _intervalIndex) {
    ++_countError1;
    _intervalTime = _intervalTime - _changeTime[_intervalIndex];
    _intervalLevel = _intervalLevel - _changeLevel[_intervalIndex] + 125;
    if (++_intervalIndex == 10) _intervalIndex = 0;    
  }

  // one second loop
  if ((_currentTime - _lastTimeSecond) > 1000) {
    _lastTimeSecond = _currentTime;    
    _sensorInSecond = _countLoop;
    _error0InSecond = _countError0;
    _error1InSecond = _countError1;
    _events = _events | 0b00000010;
    _countLoop = 0;
    _countError0 = 0;
    _countError1 = 0;
  }
  else _countLoop++;

  // external Function
  if (_events > 0) (*_externalOnTheBubble)(_events);

}
