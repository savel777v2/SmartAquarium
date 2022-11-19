// local class for bubble control

StepMotor StepMotorBubbles(11, 10, 9, 8, onTheStepMotorBubbles);
BubbleCounter CounterForBubbles(2, A7, onTheBubble); //10

class BubbleControl {
  public:

    void set_currStatus(byte currStatus);
    byte get_currStatus();
    void get_condition(char* _strValue);
    void set_bubblesIn100Second(byte bubblesIn100Second);
    byte get_bubblesIn100Second();    
    int get_minBubbleDuration();
    int get_maxBubbleDuration();
    word get_maxBubblesIn100Second();
    word get_minBubblesIn100Second();
    void clearError();
    void control(int bubbleDuration);

  private:

    void _checkReturnPosition(); // возвращает на место после ошибки
    void _addSybstring(char* _str1, char* _str2);

    // 0 - отключен, 1 - включен, 2 - в процессе, 3 - результат достигнут, 4 - ошибка 1 (ничего не меняется), 5 - ошибка 2 (долго крутим), 6 - ошибка 3 (не попали в длительность)
    byte _currStatus = 0;
    int _lastPositionMove = 0;
    int _lastBubbleDuration = 0;
    int _moveNoResult = 0;
    int _moveOneWay = 0;
    int _minBubbleDuration = 0;
    int _maxBubbleDuration = 0;
    byte _bubblesIn100Second = 0;

};

// local functions
void BubbleControl::_checkReturnPosition() {
  // return to position before NoResult
  if (_currStatus == 4 && _moveNoResult != 0) StepMotorBubbles.set_positionMove(_moveNoResult * -1);
}

void BubbleControl::_addSybstring(char* _str1, char* _str2) {
  int i = -1;
  do {
    i++;
    _str1[i] = _str2[i];
  } while (_str2[i] != '\0');
}

// global functions
void BubbleControl::set_currStatus(byte currStatus) {
  if (_currStatus == 0 && currStatus == 1) _currStatus = 1;
  if (_currStatus != 0 && currStatus == 0) {
    _checkReturnPosition();
    _currStatus = 0;
    _minBubbleDuration = 0;
    _maxBubbleDuration = 0;
  }
}

byte BubbleControl::get_currStatus() {
  return _currStatus;
}

void BubbleControl::get_condition(char* _strValue) {
  switch (_currStatus) {
    case 0: _addSybstring(_strValue, " OFF"); break;
    case 1: _addSybstring(_strValue, "  ON"); break;
    case 2:
      sprintf(_strValue, "%4d", _lastPositionMove);
      break;
    case 3: _addSybstring(_strValue, "Good"); break;
    case 4: _addSybstring(_strValue, "Err1"); break;
    case 5: _addSybstring(_strValue, "Err2"); break;
    case 6: _addSybstring(_strValue, "Err3"); break;
  }
}

void BubbleControl::set_bubblesIn100Second(byte bubblesIn100Second) {
  byte _delta;
  if (_currStatus != 0 && _bubblesIn100Second != bubblesIn100Second) {
    if (bubblesIn100Second < 30) _delta = 0;
    else if (bubblesIn100Second < 60) _delta = 1;
    else if (bubblesIn100Second < 100) _delta = 2;
    else if (bubblesIn100Second < 150) _delta = 5;
    else _delta = 10;
    _checkReturnPosition();
    _currStatus = 1;
    _bubblesIn100Second = bubblesIn100Second;
    _minBubbleDuration = 100000 / ((int)_bubblesIn100Second + 1 + _delta) + 1;
    _maxBubbleDuration = 100000 / ((int)_bubblesIn100Second - _delta);
  }
}

byte BubbleControl::get_bubblesIn100Second() {
  return _bubblesIn100Second;
}

int BubbleControl::get_minBubbleDuration() {
  return _minBubbleDuration;
}

int BubbleControl::get_maxBubbleDuration() {
  return _maxBubbleDuration;
}

word BubbleControl::get_maxBubblesIn100Second() {
  return 100000 / _minBubbleDuration;
}

word BubbleControl::get_minBubblesIn100Second() {
  return 100000 / _maxBubbleDuration;
}

void BubbleControl::clearError() {
  if (_currStatus >= 4) {
    _currStatus = 1;
  }
}

void BubbleControl::control(int bubbleDuration) {

  // пока не нужно контролировать
  if (StepMotorBubbles.get_isActive() || !CounterForBubbles.get_itsRegularBubbles()) return;

  // если не работаем или ошибка - ничего не нужно
  if (_currStatus == 0 || _currStatus == 6) return;

  // проверим может и так все хорошо
  if (bubbleDuration <= _maxBubbleDuration && bubbleDuration >= _minBubbleDuration) {
    // ура все срослось
    _currStatus = 3;
    return;
  }

  // критерий продолжения ошибки 4, 5
  if (_currStatus == 4 || _currStatus == 5) {
    if ((bubbleDuration > _maxBubbleDuration && _lastPositionMove > 0) || (bubbleDuration < _minBubbleDuration && _lastPositionMove < 0)) return;
  }

  if (_currStatus == 2) {
    // если ранее двигались
    // есть ли результат прошлого действия
    if ((bubbleDuration <= _lastBubbleDuration + 10 && _lastPositionMove < 0) || (bubbleDuration >= _lastBubbleDuration - 10 && _lastPositionMove > 0)) {
      _moveNoResult = _moveNoResult + _lastPositionMove;
      if (_moveNoResult >= 200 || _moveNoResult <= -200) {
        tone(PIEZO_PIN, 2500, 3000);
        _currStatus = 4;
        return;
      }
    }
    else {
      _lastBubbleDuration = bubbleDuration;
      _moveNoResult = 0;
    }
    // перелетели нужный результат
    if ((bubbleDuration < _minBubbleDuration && _lastPositionMove > 0) || (bubbleDuration > _maxBubbleDuration && _lastPositionMove < 0)) {
      switch (_lastPositionMove) {
        case 100: _lastPositionMove = -10; break;
        case -100: _lastPositionMove = 10; break;
        case 10: _lastPositionMove = -5; break;
        case -10: _lastPositionMove = 5; break;
        default:
          tone(PIEZO_PIN, 2500, 3000);
          _currStatus = 6;
          return;
          break;
      }
      _moveOneWay = 0;
    }
    else {
      _moveOneWay = _moveOneWay + _lastPositionMove;
      if (_moveOneWay >= 2000 || _moveNoResult <= -2000) {
        tone(PIEZO_PIN, 2500, 3000);
        _currStatus = 5;
        return;
      }
    }
  }
  else {
    // новое движение если показатель сам ушел за нужные пределы
    if (bubbleDuration < _minBubbleDuration) {
      if (_currStatus == 1) _lastPositionMove = -100;
      else _lastPositionMove = -5;
    }
    else {
      if (_currStatus == 1) _lastPositionMove = 100;
      else _lastPositionMove = 5;
    }
    _currStatus = 2;
    _lastBubbleDuration = bubbleDuration;
    _moveNoResult = 0;
    _moveOneWay = 0;
  }

  // движение мотора собственно
  tone(PIEZO_PIN, 2500, 500);
  StepMotorBubbles.set_positionMove(_lastPositionMove);

}
