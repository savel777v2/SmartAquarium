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

  private:

    void _checkReturnPosition(); // возвращает на место после ошибки
    void _addSybstring(char* _str1, char* _str2);

    byte _currStatus = 0; // 0 - отключен, 1 - включен, 2 - в процессе, 3 - результат достигнут, 4 - ошибка 1 (ничего не меняется), 5 - ошибка 2 (не попали в длительность)
    int _lastPositionMove = 0;
    int _moveNoResult = 0;
    int _minBubbleDuration = 0;
    int _maxBubbleDuration = 0;
    byte _bubblesIn10Second = 0;

};

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
    case 3: _addSybstring(_strValue, " OK "); break;
    case 4: _addSybstring(_strValue, "Err1"); break;
    case 5: _addSybstring(_strValue, "Err2"); break;
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
