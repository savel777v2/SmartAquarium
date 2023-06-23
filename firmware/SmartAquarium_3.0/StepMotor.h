/*
  StepMotor.h - Library for step motor


*/
#pragma once

#define MIN_DELAY 4 // минимальная задержка между шагом мотора в мс., соответсвует MAX_USER_SPEED
#define MAX_USER_SPEED 32 // максимально допустимое значение линейной пользовательской скорости
#define MAX_DELAY 128 // максимальная задержка шага мотора при скорости около 0 = MIN_DELAY * MAX_USER_SPEED

class StepMotor {
  public:

    StepMotor(byte pin1, byte pin2, byte pin3, byte pin4);
    void set_userSpeed(int userSpeed);
    int get_userSpeed();
    void set_positionMotor(long positionMotor);
    long get_positionMotor();
    void set_positionMove(long positionMove);
    long get_positionMove();
    bool get_isActive();
    unsigned long get_lastSeekMotor();
    int loop();

  private:

    int getStepDelay(int _UserSpeed); // функция расчета задержки шага мотора от пользовательской скорости мотора
    void stepDelayBringCloser(int& stepDelay, int needStepDelay); // задержку приблизим к нужной в соответствии с нужной скоростью
    void stepDelayMovePosition(int& stepDelay); // двигаемся к нужной позиции сначала ускоряясь, затем замедляясь
    void motorPositionUp(byte& phaseMotor, int& ans); // шаг мотора вверх
    void motorPositionDown(byte& phaseMotor, int& ans); // шаг мотора вниз
   
    byte _pin1, _pin2,  _pin3, _pin4;
    unsigned long _lastSeekMotor = 0; // время последнего шага мотора
    bool _isActive = false; // мотор активен
    long _positionMotor = 0; // относительная позиция мотора
    long _positionMove = 0; // изменение позиции мотора
    int _userSpeed = 0; // нужное направление и скорость текущего движения мотора
    int _directionMotor = 0; // текущее направление мотора
    int _userDelayMotor = MIN_DELAY * MAX_USER_SPEED; // нужная задержка мотора (зависит от userSpeed)
    int _stepDelay = MAX_DELAY; // текущая задержка шага мотора
    byte _phaseMotor = 0; // фаза мотора 0-3
    byte _brakingRouteDelay[16] = {124, 99, 79, 63, 50, 40, 31, 25, 20, 16, 13, 10, 8, 6, 5, 4}; // расчет как нужно тормозить с заданным ускорением

};

StepMotor::StepMotor(byte pin1, byte pin2, byte pin3, byte pin4) {
  _pin1 = pin1;
  _pin2 = pin2;
  _pin3 = pin3;
  _pin4 = pin4;  
  pinMode(_pin1, OUTPUT);
  pinMode(_pin2, OUTPUT);
  pinMode(_pin3, OUTPUT);
  pinMode(_pin4, OUTPUT);
}

int StepMotor::getStepDelay(int userSpeed) {
  if (userSpeed == 0) return MAX_DELAY;
  float _Delay = (float) MAX_DELAY / userSpeed;
  return round(abs(_Delay));
}

// задержку приблизим к нужной
void StepMotor::stepDelayBringCloser(int& stepDelay, int needStepDelay) {
  float _deltaDelayMotor = stepDelay * 0.25;
  if (stepDelay > needStepDelay) {
    stepDelay = stepDelay - round(_deltaDelayMotor);
    if (stepDelay < needStepDelay) stepDelay = needStepDelay;
  }
  else if (stepDelay < needStepDelay) {
    stepDelay = stepDelay + round(_deltaDelayMotor);
    if (stepDelay > needStepDelay) stepDelay = needStepDelay;
  }
}

// двигаемся к нужной позиции сначала ускоряясь, затем замедляясь
void StepMotor::stepDelayMovePosition(int& stepDelay) {
  float _deltaDelayMotor = stepDelay * 0.25;
  if (abs(_positionMove) > 16) {
    if (stepDelay != MIN_DELAY) {
      // можно ускориться в любом случае оттормозимся
      stepDelay = stepDelay - round(_deltaDelayMotor);
      if (stepDelay < MIN_DELAY) stepDelay = MIN_DELAY;
    }
    return;
  }
  // пытаемся ускоритьсz или встаем на маршрут торможения
  stepDelay = stepDelay - round(_deltaDelayMotor);
  byte _brakingDelay = _brakingRouteDelay[abs(_positionMove) - 1];
  if (stepDelay < _brakingDelay) stepDelay = _brakingDelay;
}

// шаг мотора вверх
void StepMotor::motorPositionUp(byte& phaseMotor, int& ans) {
  _positionMotor++;
  if (phaseMotor == 3) phaseMotor = 0;
  else phaseMotor++;
  if (_positionMove != 0) _positionMove--;
  ans = 1;
}

// шаг мотора вниз
void StepMotor::motorPositionDown(byte& phaseMotor, int& ans) {
  _positionMotor--;
  if (phaseMotor == 0) phaseMotor = 3;
  else phaseMotor--;
  if (_positionMove != 0) _positionMove++;
  ans = -1;
}

void StepMotor::set_userSpeed(int userSpeed) {
  if (userSpeed < -1 * MAX_USER_SPEED) _userSpeed = -1 * MAX_USER_SPEED;
  else if (userSpeed > MAX_USER_SPEED) _userSpeed = MAX_USER_SPEED;
  else _userSpeed = userSpeed;
  _userDelayMotor = getStepDelay(_userSpeed);
  _positionMove = 0;
}

int StepMotor::get_userSpeed() {
  return _userSpeed;
}

void StepMotor::set_positionMotor(long positionMotor) {
  _positionMotor = positionMotor;
}

long StepMotor::get_positionMotor() {
  return _positionMotor;
}

void StepMotor::set_positionMove(long positionMove) {
  _positionMove = positionMove;
  _userSpeed = 0;
}

long StepMotor::get_positionMove() {
  return _positionMove;
}

bool StepMotor::get_isActive() {
  return _isActive;
}

unsigned long StepMotor::get_lastSeekMotor() {
  return _lastSeekMotor;
}

int StepMotor::loop() {  
  
  int ans = 255;

  if (((_isActive) || (_userSpeed != 0) || (_directionMotor != 0) || (_positionMove != 0)) && ((millis() - _lastSeekMotor) > _stepDelay)) {
    _lastSeekMotor = millis();
    if ((_userSpeed == 0) && (_positionMove == 0) && (_directionMotor == 0)) {
      // остановка мотора в текущей фазе
      ans = 0;
      _isActive = false;
      _stepDelay = MAX_DELAY;
      digitalWrite(_pin1, LOW);
      digitalWrite(_pin2, LOW);
      digitalWrite(_pin3, LOW);
      digitalWrite(_pin4, LOW);
    }
    else {
      _isActive = true;      
      // двигаем мотор
      if (_directionMotor  > 0) motorPositionUp(_phaseMotor, ans);
      else if (_directionMotor < 0) motorPositionDown(_phaseMotor, ans);
      else if (_positionMove > 0) motorPositionUp(_phaseMotor, ans);
      else if (_positionMove < 0) motorPositionDown(_phaseMotor, ans);
      else if (_userSpeed > 0) motorPositionUp(_phaseMotor, ans);
      else motorPositionDown(_phaseMotor, ans);
      switch (_phaseMotor) {
        case 0:
          digitalWrite(_pin1, HIGH);
          digitalWrite(_pin2, LOW);
          digitalWrite(_pin3, LOW);
          digitalWrite(_pin4, HIGH);
          break;
        case 1:
          digitalWrite(_pin1, LOW);
          digitalWrite(_pin2, LOW);
          digitalWrite(_pin3, HIGH);
          digitalWrite(_pin4, HIGH);
          break;
        case 2:
          digitalWrite(_pin1, LOW);
          digitalWrite(_pin2, HIGH);
          digitalWrite(_pin3, HIGH);
          digitalWrite(_pin4, LOW);
          break;
        case 3:
          digitalWrite(_pin1, HIGH);
          digitalWrite(_pin2, HIGH);
          digitalWrite(_pin3, LOW);
          digitalWrite(_pin4, LOW);
          break;
      }
      // ускорение в нужном направлении
      if (_directionMotor == 0) {
        // меняем направление мотора
        if (_userSpeed < 0 || _positionMove < 0) _directionMotor = -1;
        else _directionMotor = 1;
        _stepDelay = MAX_DELAY;
      }
      else if ((_userSpeed == 0 && _positionMove == 0) || (_directionMotor * _userSpeed < 0) || (_directionMotor * _positionMove < 0)) {
        // движение мотора в разные стороны - снижаем скорость наращивая задержку
        if (_stepDelay == MAX_DELAY) _directionMotor = 0; // можно менять направление
        else stepDelayBringCloser(_stepDelay, MAX_DELAY);
      }
      else if (_userSpeed != 0) {
        // движение мотора в нужную сторону к нужной скорости - приближаем задержку к нужной
        if (_stepDelay != _userDelayMotor) stepDelayBringCloser(_stepDelay, _userDelayMotor);
      }
      else if (_positionMove != 0) {
        // движение мотора в нужную сторону к нужному смещению - приближаем задержку к нужной
        stepDelayMovePosition(_stepDelay);
      }
    }
  }

  return ans;
}
