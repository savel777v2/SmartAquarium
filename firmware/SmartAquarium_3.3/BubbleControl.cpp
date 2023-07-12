/*
  BubbleControl.cpp - Программная реализация управления расходом CO2
*/

#include "BubbleControl.h"

// local functions
void BubbleControl::_checkReturnPosition() {
  // return to position before NoResult
  if (_currStatus == 4 && _moveNoResult != 0) globStepMotor.set_positionMove(_moveNoResult * -1);
}

// global functions
void BubbleControl::set_currStatus(byte currStatus) {
  if (_currStatus == 0 && currStatus == 1) _currStatus = 1;
  if (_currStatus != 0 && currStatus == 0) {
    _checkReturnPosition();
    _currStatus = 0;
  }
};

byte BubbleControl::get_currStatus() {
  return _currStatus;
}

String BubbleControl::get_condition() {
  switch (_currStatus) {
    case 0: return " OFF"; break;
    case 1: return "  ON"; break;
    case 2: return global::valToString(_lastPositionMove,4,3); break;
    case 3: return "Good"; break;
    case 4: return "Err1"; break;
    case 5: return "Err2"; break;
    case 6: return "Err3"; break;
  }
};

void BubbleControl::set_bubblesIn100Second(byte bubblesIn100Second) {
  // минимальное значение настройки
  if (bubblesIn100Second <= 19) bubblesIn100Second = 0;
  if (_currStatus != 0 && _bubblesIn100Second != bubblesIn100Second) {
    _checkReturnPosition();
    _currStatus = 1;
    _bubblesIn100Second = bubblesIn100Second;
    if (_bubblesIn100Second == 0) {
      _minBubbleDuration = 10000;
      _maxBubbleDuration = 10000;
    }
    else {
      byte _delta;
      if (_bubblesIn100Second < 60) _delta = 1;
      else if (_bubblesIn100Second < 100) _delta = 2;
      else if (_bubblesIn100Second < 150) _delta = 5;
      else _delta = 10;
      _minBubbleDuration = 100000 / ((int)_bubblesIn100Second + 1 + _delta) + 1;
      _maxBubbleDuration = 100000 / ((int)_bubblesIn100Second - _delta);
    }
  }
};

byte BubbleControl::get_bubblesIn100Second() {
  return _bubblesIn100Second;
};

int BubbleControl::get_minBubbleDuration() {
  return _minBubbleDuration;
};

int BubbleControl::get_maxBubbleDuration() {
  return _maxBubbleDuration;
};

word BubbleControl::get_maxBubblesIn100Second() {
  if (_minBubbleDuration == 0 || _minBubbleDuration == 10000) return 0;
  return 100000 / _minBubbleDuration;
};

word BubbleControl::get_minBubblesIn100Second() {
  if (_maxBubbleDuration == 0 || _maxBubbleDuration == 10000) return 0;
  return 100000 / _maxBubbleDuration;
};

void BubbleControl::clearError() {
  if (_currStatus >= 4) {
    _currStatus = 3;
  }
};

bool BubbleControl::controlWaiting() {

  // пока не нужно контролировать
  if (globStepMotor.get_isActive()) {
    _bubblesAfterMotor = 0;
    return true;
  }

  // после последнего движения ждем 5 пузырей на обратную связь по скорости пузырька
  if (_bubblesAfterMotor < 5) {
    _bubblesAfterMotor++;
    return true;
  }

  // теперь проверяем дабы пузырьки стабилизировались
  if (!globBubbleCounter.itsRegularBubbles()) return true;

  // если не работаем - ничего не нужно
  if (_currStatus == 0) return false;

  int bubbleDuration = globBubbleCounter.getDuration();

  // если кран перекрыт, то перекрыт
  if (bubbleDuration == -3) bubbleDuration = 10000;

  // проверим может и так все хорошо
  if (bubbleDuration <= _maxBubbleDuration && bubbleDuration >= _minBubbleDuration) {
    // ура все срослось
    _currStatus = 3;
    return false;
  }

  // критерий продолжения ошибок
  if (_currStatus == 4 || _currStatus == 5) {
    if ((bubbleDuration > _maxBubbleDuration && _lastPositionMove > 0) || (bubbleDuration < _minBubbleDuration && _lastPositionMove < 0)) return false;
  }
  else if (_currStatus == 6) return false;

  if (_currStatus == 2) {
    // если ранее двигались
    // есть ли результат прошлого действия
    if ((bubbleDuration <= _lastBubbleDuration + 10 && _lastPositionMove < 0) || (bubbleDuration >= _lastBubbleDuration - 10 && _lastPositionMove > 0)) {
      _moveNoResult = _moveNoResult + _lastPositionMove;
      if (_moveNoResult >= 300 || _moveNoResult <= -300) {
        tone(PIEZO_PIN, 2500, 3000);
        _currStatus = 4;
        return false;
      }
    }
    else {
      _lastBubbleDuration = bubbleDuration;
      _moveNoResult = 0;
    }
    // перелетели нужный результат
    if ((bubbleDuration < _minBubbleDuration && _lastPositionMove > 0) || (bubbleDuration > _maxBubbleDuration && _lastPositionMove < 0)) {
      switch (_lastPositionMove) {
        case 50: _lastPositionMove = -10; break;
        case -50: _lastPositionMove = 10; break;
        case 10: _lastPositionMove = -5; break;
        case -10: _lastPositionMove = 5; break;
        default:
          if (++_countError3 == 10) {
            tone(PIEZO_PIN, 2500, 3000);
            _currStatus = 6;
            _countError3 = 0;
            return false;
          }
          else {
            _lastPositionMove = -_lastPositionMove;
          }
          break;
      }
      _moveOneWay = 0;
    }
    else {
      _moveOneWay = _moveOneWay + _lastPositionMove;
      if (_moveOneWay >= 2000 || _moveNoResult <= -2000) {
        tone(PIEZO_PIN, 2500, 3000);
        _currStatus = 5;
        return false;
      }
    }
  }
  else {
    // новое движение c 50
    // если показатель сам ушел или после ошибки с 5
    if (bubbleDuration < _minBubbleDuration) {
      if (_currStatus == 1) _lastPositionMove = -50;
      else _lastPositionMove = -5;
    }
    else {
      if (_currStatus == 1) _lastPositionMove = 50;
      else _lastPositionMove = 5;
    }
    _currStatus = 2;
    _lastBubbleDuration = bubbleDuration;
    _moveNoResult = 0;
    _moveOneWay = 0;
  }

  // движение мотора собственно
  if (EEPROM.read(EEPROM_CONTROL_BUBBLE_SOUND_ON) == 1) tone(PIEZO_PIN, 2500, 500);
  _bubblesAfterMotor = 0;
  globStepMotor.set_positionMove(_lastPositionMove);

  return false;

};
