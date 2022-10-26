// universal class for bubble counting

class BubbleCounter {
  public:

    BubbleCounter(int laserPin, int analogPin, void (*function)());
    void set_minBubbleLevel(int minBubbleLevel);
    int get_minBubbleLevel();
    void set_bubbleVibration(int bubbleVibration);
    int get_bubbleVibration();
    int get_sensorInSecond();
    long get_bubbleCounter();
    bool get_itsBubble();
    void tick();

  private:
    bool _checkError();

    int _laserPin, _analogPin;
    void (*_onTheBubble)();
    int _minBubbleLevel = 100; // мин уровень срабатывания пузырька
    int _bubbleVibration = 5; // дребезг срабатывания пузырька в мс
    int _sensorInSecond = 0; // считываний сенсора в секунду
    long _bubbleCounter = 0; // счетчик пузырьков
    unsigned long _durationNoBubble = 0; // продолжительность интервала простоя
    unsigned long _durationBubble = 0; // продолжительность интервала пузыря
    // мин. и макс. уровни сигналов
    int _MinLevelBubble, _MaxLevelBubble, _MinLevelNoBubble, _MaxLevelNoBubble;    
    bool _itsBubble = false; // флаг пролета пузырька
    

};

BubbleCounter::BubbleCounter(int laserPin, int analogPin, void (*function)()) {
  _laserPin = laserPin;
  _analogPin = analogPin;
  _onTheBubble = *function;
  pinMode(_laserPin, OUTPUT);
  digitalWrite(_laserPin, HIGH);
}

void BubbleCounter::set_minBubbleLevel(int minBubbleLevel) {
  _minBubbleLevel = minBubbleLevel;
}

int BubbleCounter::get_minBubbleLevel() {
  return _minBubbleLevel;
}

void BubbleCounter::set_bubbleVibration(int bubbleVibration) {
  _bubbleVibration = bubbleVibration;
}

int BubbleCounter::get_bubbleVibration() {
  return _bubbleVibration;
}

int BubbleCounter::get_sensorInSecond() {
  return _sensorInSecond;
}

long BubbleCounter::get_bubbleCounter() {
  return _bubbleCounter;
}

bool BubbleCounter::get_itsBubble() {
  return _itsBubble;
}

bool BubbleCounter::_checkError() {
  // выход показателй за предельные значения
  if ((_durationNoBubble > 10000) || (_durationNoBubble < 10) || (_durationBubble > 100) || (_durationBubble < 10)) return true;
  else return false;
}

// функция подсчета пузырьков
void BubbleCounter::tick() {
  int _newLevel, _countSensor;
  unsigned long _currentTime; // текущее время считывания
  static unsigned long _beginCountSensorInSecond; // начало обсчета сенсор в секунду
  static unsigned long _lastTimeBubbleLevel; // последнее время уровня пузырька
  static long _beginBubble; // начало регистрации пузырька
  static long _beginNoBubble; // начало регистрации без пузырька
  // обсчет мин. и макс. уровней сигналов
  static int _tempMinLevelBubble, _tempMaxLevelBubble, _tempMinLevelNoBubble, _tempMaxLevelNoBubble;
  bool _externalFunction = false;

  _newLevel = analogRead(_analogPin);
  _currentTime = millis();

  // сколько считываний в секунду
  if ((_currentTime - _beginCountSensorInSecond) > 1000) {
    _sensorInSecond = _countSensor;
    _externalFunction = true;
    _beginCountSensorInSecond = _currentTime;
    _countSensor = 0;    
  }
  else _countSensor++;

  // анализ сигнала
  if (_newLevel > _minBubbleLevel) {
    _lastTimeBubbleLevel = _currentTime;
    if (_beginBubble == -1) {
      // 1.1 это новый пузырек
      _beginBubble = _currentTime;
      _itsBubble = true;
      _externalFunction = true;
      if (!_checkError()) _bubbleCounter++;

      // обсчет уровней сигналов
      _tempMinLevelBubble = _newLevel;
      _tempMaxLevelBubble = _newLevel;
      // завершаем интервал без пузырька
      if (_beginNoBubble != -1) {
        _durationNoBubble = _currentTime - _beginNoBubble;
        _beginNoBubble = -1;
        _MinLevelNoBubble = _tempMinLevelNoBubble;
        _MaxLevelNoBubble = _tempMaxLevelNoBubble;
      }
    }
    else {
      // 1.2 идет старый пузырек обсчитываем его
      _tempMinLevelBubble = min(_newLevel, _tempMinLevelBubble);
      _tempMaxLevelBubble = max(_newLevel, _tempMaxLevelBubble);
    }
  }
  else {
    if (_beginBubble == -1) {
      // интервал без пузырька _beginBubble ==-1
      if (_beginNoBubble == -1) {
        // 2.1 начало интервала без пузырька - только при старте
        _beginNoBubble = _currentTime;
        _tempMinLevelNoBubble = _newLevel;
        _tempMaxLevelNoBubble = _newLevel;
      }
      else {
        // 2.2 идет интервал без пузырька
        _tempMinLevelNoBubble = min(_newLevel, _tempMinLevelNoBubble);
        _tempMaxLevelNoBubble = max(_newLevel, _tempMaxLevelNoBubble);
      }
    }
    else {
      if ((_currentTime - _lastTimeBubbleLevel) > _bubbleVibration) {
        // 2.3 время вибрации кончилось - пузырек завершен в мин. BubbleVibration.
        _durationBubble = _currentTime - _beginBubble;
        _beginBubble = -1;
        _itsBubble = false;
        _externalFunction = true;
                
        _MinLevelBubble = _tempMinLevelBubble;
        _MaxLevelBubble = _tempMaxLevelBubble;
        // начало интервала без пузырька - при завершении пузырька
        _beginNoBubble = _currentTime;
        _tempMinLevelNoBubble = _newLevel;
        _tempMaxLevelNoBubble = _newLevel;
      }
      else {
        // 2.4 интервал пузырька
        _tempMinLevelBubble = min(_newLevel, _tempMinLevelBubble);
        _tempMaxLevelBubble = max(_newLevel, _tempMaxLevelBubble);
      }
    }
  }

  // вызов внешней функции
  if (_externalFunction) (*_onTheBubble)();  
  
}
