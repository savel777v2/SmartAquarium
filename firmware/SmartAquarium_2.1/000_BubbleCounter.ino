// universal class for bubble counting

#define BUFFER_SENSOR_SIZE 50
#define CHANGE_TIME_BUBBLE 5 // подъем\спуск пузырька (макс. длительность в мс.)
#define CHANGE_LEVEL_BUBBLE 10 // подъем\спуск пузырька (мин. изменение уровня)

class BubbleCounter {
  public:

    BubbleCounter(int laserPin, int analogPin, void (*function)(byte _events));
    void set_working(bool working);
    bool get_working();    
    void set_maxDurationBubble(byte maxDurationBubble);
    byte get_maxDurationBubble();
    void set_minLevelBubble(byte minLevelBubble);
    byte get_minLevelBubble();
    byte get_curIndex();
    byte get_changeLevel(byte index);
    byte get_changeTime(byte index);
    int get_sensorInSecond();
    int get_error0InSecond();
    int get_error1InSecond();
    void set_bubbleCounter(unsigned long bubbleCounter);
    unsigned long get_bubbleCounter();
    int get_bubbleIn100Second();
    int get_bubbleInMinute();
    int get_MinLevel();
    int get_MaxLevel();
    int get_durationBubble();
    int get_durationNoBubble();
    long get_lastDuration();
    bool get_itsRegularBubbles();
    
    void tick();

  private:
    void writeLastDuration(long lastDuration, byte& _events);
    void endingBubbleInterval(word& __durationBubble, int& __levelBubble, int& __maxLevelBubble, byte& __countSmoothLevel, byte& _events);

    int _laserPin, _analogPin;
    void (*_externalOnTheBubble)(byte _events); // значения _events:

    // изменение уровня с последнего измерения 0 - мин. уровень 255 - макс. уровень. 125 - уровень 0
    byte _changeLevel[BUFFER_SENSOR_SIZE];
    byte _changeTime[BUFFER_SENSOR_SIZE];  // прирост мс. с последнего измерения, 255 - макс. уровень
    byte _curIndex = 0;    // тек. индекс записи
    byte _intervalIndex = 0; // индекс начала интервала

    bool _working;

    // параметры диагностики пузырька
    byte _maxDurationBubble = 30; // макс. длительность пузырька
    byte _minLevelBubble = 30; // мин. уровень пузырька

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
  _working = true;
  pinMode(_laserPin, OUTPUT);
  digitalWrite(_laserPin, HIGH);
  for (int _i = 0; _i < BUFFER_SENSOR_SIZE; _i++) _changeLevel[_i] = 125;
}

void BubbleCounter::set_working(bool working) {
  _working = working;
  if (_working) digitalWrite(_laserPin, HIGH);
  else digitalWrite(_laserPin, LOW);
}

bool BubbleCounter::get_working() {
  return _working;
}

void BubbleCounter::set_maxDurationBubble(byte maxDurationBubble) {
  _maxDurationBubble = maxDurationBubble;
}

byte BubbleCounter::get_maxDurationBubble() {
  return _maxDurationBubble;
}

void BubbleCounter::set_minLevelBubble(byte minLevelBubble) {
  _minLevelBubble = minLevelBubble;
}

byte BubbleCounter::get_minLevelBubble() {
  return _minLevelBubble;
}

byte BubbleCounter::get_curIndex() {
  return _curIndex;
}

byte BubbleCounter::get_changeLevel(byte index) {
  return _changeLevel[index];
}

byte BubbleCounter::get_changeTime(byte index) {
  return _changeTime[index];
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

int BubbleCounter::get_bubbleIn100Second() {
  if (_lastDurations[_iLastDuration] <= 0) return _lastDurations[_iLastDuration];
  else return 100000 / _lastDurations[_iLastDuration];
}

int BubbleCounter::get_bubbleInMinute() {
  if (_lastDurations[_iLastDuration] <= 0) return _lastDurations[_iLastDuration];
  else return 60000 / _lastDurations[_iLastDuration];
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

void BubbleCounter::writeLastDuration(long lastDuration, byte& _events) {
  // ошибка или пузырек
  _events = _events | 0b00010000;
  if (++_iLastDuration == 5) _iLastDuration = 0;
  _lastDurations[_iLastDuration] = lastDuration;
}

void BubbleCounter::endingBubbleInterval(word& __durationBubble, int& __levelBubble, int& __maxLevelBubble, byte& __countSmoothLevel, byte& _events) {
  __durationBubble = 0;
  __levelBubble = 0;
  __maxLevelBubble = 0;
  __countSmoothLevel = 0;
  // погасить инидикатор
  _events = _events | 0b00001000;
}

// функция подсчета пузырьков
void BubbleCounter::tick() {

  static unsigned long _lastTime = 0;     // последнее время датчика
  static unsigned long _lastTimeMinMaxLevel; // начало обсчета сенсор в секунду
  static unsigned long _lastTimeError; // последнее время цикла ошибки
  static unsigned long _lastTimeSecond; // последнее время цикла секунды

  // диагностика пузырька
  static word __durationBubble = 0; // продолжительность интервала пузыря
  static word __durationNoBubble = 0; // продолжительность интервала простоя
  static int __levelBubble = 0; // уровень пузыря
  static int __maxLevelBubble = 0; // уровень пузыря
  static byte __countSmoothLevel = 0; // счетчик ровного уровня

  static int _lastLevel = 0;    // последний уровень датчика
  static int _intervalTime = 0;   // время оцениваемого интервала
  static int _intervalLevel = 0;  // изменение уровня оцениваемого интервала
  static int _countLoop = 0;    // счетчик циклов
  static int _countError0 = 0;  // счетчик ошибок "провалов" считывания
  static int _countError1 = 0;  // счетчик ошибок нехватки буфера обсчета
  static int _tempMinLevel, _tempMaxLevel; // обсчет мин. и макс. уровней сигналов

  // needing to refactoring
  if (!_working) return;  

  // it's a first loop
  unsigned long _currentTime = millis();
  if (_lastTime == 0) {
    _lastTime = _currentTime;
    _lastLevel = analogRead(_analogPin);
    return;
  }

  // read sensor once in millisecond
  if (_currentTime == _lastTime) return;

  // local values
  int _newLevel = analogRead(_analogPin);
  // поразрядно: 0 - обсчет Min\Max, 1 - обсчет _InSecond, 2 - начало индикации, 3 - конец индикации, 4 - пузырек или ошибка
  byte _events = 0;

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
  int __changeLevel = _newLevel - _lastLevel + 125;
  if (__changeLevel < 0) _changeLevel[_curIndex] = 0;
  else if (__changeLevel > 255) _changeLevel[_curIndex] = 255;
  else _changeLevel[_curIndex] = __changeLevel;

  // COPY ALGORITM TO DEBUGING IN debugCounterTick()
  // increament interval
  _intervalTime = _intervalTime + _changeTime[_curIndex];
  _intervalLevel = _intervalLevel + _changeLevel[_curIndex] - 125;
  __durationNoBubble = __durationNoBubble + _changeTime[_curIndex];

  // changing the interval while it is longer that CHANGE_TIME_BUBBLE
  while (_intervalTime > CHANGE_TIME_BUBBLE && _intervalIndex != _curIndex) {
    _intervalTime = _intervalTime - _changeTime[_intervalIndex];
    _intervalLevel = _intervalLevel - _changeLevel[_intervalIndex] + 125;
    if (++_intervalIndex == BUFFER_SENSOR_SIZE) _intervalIndex = 0;
  }

  // checking Errors
  if (_intervalTime > CHANGE_TIME_BUBBLE) ++_countError0;

  if (__durationBubble == 0) {
    if (_intervalLevel > CHANGE_LEVEL_BUBBLE) {
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
      writeLastDuration(-1, _events);
      endingBubbleInterval(__durationBubble, __levelBubble, __maxLevelBubble, __countSmoothLevel, _events);
    }
    else if (_intervalLevel >= CHANGE_LEVEL_BUBBLE * (-1) && _intervalLevel <= CHANGE_LEVEL_BUBBLE) {
      // maybe end of s Bubble interval
      if (++__countSmoothLevel > 1) {
        if (__maxLevelBubble > _minLevelBubble) {
          _durationBubble = __durationBubble;
          _durationNoBubble = __durationNoBubble - __durationBubble;
          __durationNoBubble = 0;
          // counting          
          _bubbleCounter++;
          _lastTimeError = _currentTime; 
          long __writeDuration;
          if (_durationNoBubble > 10000) __writeDuration = -3;
          else __writeDuration = _durationBubble + _durationNoBubble;
          writeLastDuration(__writeDuration, _events);
        }
        else {
          // Bubble too small
          writeLastDuration(-2, _events);
        }        
        endingBubbleInterval(__durationBubble, __levelBubble, __maxLevelBubble, __countSmoothLevel, _events);
      }
    }
    else __countSmoothLevel = 0;
  }

  // increament current index
  if (++_curIndex == BUFFER_SENSOR_SIZE) _curIndex = 0;
  if (_curIndex == _intervalIndex) {
    ++_countError1;
    _intervalTime = _intervalTime - _changeTime[_intervalIndex];
    _intervalLevel = _intervalLevel - _changeLevel[_intervalIndex] + 125;
    if (++_intervalIndex == BUFFER_SENSOR_SIZE) _intervalIndex = 0;
  }
  // END COPY ALGORITM

  // loop longer durations errors
  if ((_currentTime - _lastTimeError) > 10000) {
    _lastTimeError = _currentTime;
    if (__durationNoBubble > 10000) writeLastDuration(-3, _events);
  }

  // counting values for the next loop
  _lastTime = _currentTime;
  _lastLevel = _newLevel;

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
