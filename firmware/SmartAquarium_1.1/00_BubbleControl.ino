// local class for bubble control

StepMotor StepMotorBubbles(11, 10, 9, 8, onTheStepMotorBubbles);
BubbleCounter CounterForBubbles(2, A7, onTheBubble); //10

class BubbleControl {
  public:

    void set_currStatus(byte currStatus);
    byte get_currStatus();
    void get_condition(char* _strValue);
    void set_bubblesIn10Second(byte bubbleIn10Second);
    byte get_bubblesIn10Second();
    int get_minBubbleDuration();
    int get_maxBubbleDuration();
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
    byte _bubblesIn10Second = 0;

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

void BubbleControl::set_bubblesIn10Second(byte bubblesIn10Second) {
  byte _delta;
  if (_currStatus != 0 && _bubblesIn10Second != bubblesIn10Second) {
    if (bubblesIn10Second < 3) _delta = 0;
    else if (bubblesIn10Second < 6) _delta = 1;
    else if (bubblesIn10Second < 10) _delta = 2;
    else if (bubblesIn10Second < 15) _delta = 5;
    else _delta = 10;
    _checkReturnPosition();
    _currStatus = 1;
    _bubblesIn10Second = bubblesIn10Second;
    _minBubbleDuration = 100000 / ((int)_bubblesIn10Second * 10 + 1 + _delta) + 1;
    _maxBubbleDuration = 100000 / ((int)_bubblesIn10Second * 10 - _delta);
  }
}

byte BubbleControl::get_bubblesIn10Second() {
  return _bubblesIn10Second;
}

int BubbleControl::get_minBubbleDuration() {
  return _minBubbleDuration;
}

int BubbleControl::get_maxBubbleDuration() {
  return _maxBubbleDuration;
}

void BubbleControl::clearError() {
  if (_currStatus >= 4) {
    _currStatus = 1;
  }
}

void BubbleControl::control(int bubbleDuration) {

  // если не работаем или ошибка - ничего не нужно
  if (_currStatus == 0 || _currStatus >= 4) return;

  // проверим может и так все хорошо
  if (bubbleDuration <= _maxBubbleDuration && bubbleDuration >= _minBubbleDuration) {
    // ура все срослось
    _currStatus = 3;
    return;
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
    // новое движение или показатель сам ушел
    if (bubbleDuration < _minBubbleDuration) {
      switch (_currStatus) {
        case 1: _lastPositionMove = -100; break;
        case 3: _lastPositionMove = -5; break;
      }
    }
    if (bubbleDuration > _maxBubbleDuration) {
      switch (_currStatus) {
        case 1: _lastPositionMove = 100; break;
        case 3: _lastPositionMove = 5; break;
      }
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
