// universal class for step motor

class StepMotor {
  public:

    StepMotor(int pin1, int pin2, int pin3, int pin4, void (*function)(int _direction));
    void set_userSpeed(int userSpeed);
    int get_userSpeed();
    void set_positionMotor(long positionMotor);
    long get_positionMotor();
    void set_positionMove(long positionMove);
    long get_positionMove();
    bool get_isActive();
    unsigned long get_lastSeekMotor();
    void tick();

  private:

    void (*_onTheStepMotor)(int _direction);
    int getStepDelay(int _UserSpeed); // функция расчета задержки шага мотора от пользовательской скорости мотора
    void stepDelayBringCloser(int& stepDelay, int needStepDelay); // задержку приблизим к нужной в соответствии с нужной скоростью
    void stepDelayMovePosition(int& stepDelay); // двигаемся к нужной позиции сначала ускоряясь, затем замедляясь
    void motorPositionUp(byte& phaseMotor); // шаг мотора вверх
    void motorPositionDown(byte& phaseMotor); // шаг мотора вниз

    int _pin1, _pin2,  _pin3, _pin4;
    unsigned long _lastSeekMotor = 0; // время последнего шага мотора
    bool _isActive = false; // мотор активен
    long _positionMotor = 0; // относительная позиция мотора
    long _positionMove = 0; // изменение позиции мотора
    int _userSpeed = 0; // нужное направление и скорость текущего движения мотора
    int _directionMotor = 0; // текущее направление мотора
    int _userDelayMotor = _minDelay * _maxUserSpeed; // нужная задержка мотора (зависит от userSpeed)
    int _minDelay = 4; // минимальная задержка между шагом мотора в мс., соответсвует _maxUserSpeed
    int _maxUserSpeed = 32; // максимально допустимое значение линейной пользовательской скорости
    int _maxDelay = _minDelay * _maxUserSpeed; // максимальная задержка шага мотора при скорости около 0
    byte _brakingRouteDelay[16] = {124, 99, 79, 63, 50, 40, 31, 25, 20, 16, 13, 10, 8, 6, 5, 4}; // расчет как нужно тормозить с заданным ускорением

};

StepMotor::StepMotor(int pin1, int pin2, int pin3, int pin4, void (*function)(int _direction)) {
  _pin1 = pin1;
  _pin2 = pin2;
  _pin3 = pin3;
  _pin4 = pin4;
  _onTheStepMotor = *function;
  pinMode(_pin1, OUTPUT);
  pinMode(_pin2, OUTPUT);
  pinMode(_pin3, OUTPUT);
  pinMode(_pin4, OUTPUT);
}

int StepMotor::getStepDelay(int userSpeed) {
  if (userSpeed == 0) return _maxDelay;
  float _Delay = (float) _maxDelay / userSpeed;
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
    if (stepDelay != _minDelay) {
      // можно ускориться в любом случае оттормозимся
      stepDelay = stepDelay - round(_deltaDelayMotor);
      if (stepDelay < _minDelay) stepDelay = _minDelay;
    }
    return;
  }
  // пытаемся ускоритьсz или встаем на маршрут торможения
  stepDelay = stepDelay - round(_deltaDelayMotor);
  byte _brakingDelay = _brakingRouteDelay[abs(_positionMove) - 1];
  if (stepDelay < _brakingDelay) stepDelay = _brakingDelay;
}

// шаг мотора вверх
void StepMotor::motorPositionUp(byte& phaseMotor) {
  _positionMotor++;
  if (phaseMotor == 3) phaseMotor = 0;
  else phaseMotor++;
  if (_positionMove != 0) _positionMove--;
  _onTheStepMotor(+1);
}

// шаг мотора вниз
void StepMotor::motorPositionDown(byte& phaseMotor) {
  _positionMotor--;
  if (phaseMotor == 0) phaseMotor = 3;
  else phaseMotor--;
  if (_positionMove != 0) _positionMove++;
  _onTheStepMotor(-1);
}

void StepMotor::set_userSpeed(int userSpeed) {
  if (userSpeed < -1 * _maxUserSpeed) _userSpeed = -1 * _maxUserSpeed;
  else if (userSpeed > _maxUserSpeed) _userSpeed = _maxUserSpeed;
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

void StepMotor::tick() {  
  static int _stepDelay = _maxDelay; // текущая задержка шага мотора
  static byte _phaseMotor = 0; // фаза мотора 0-3

  if (((_isActive) || (_userSpeed != 0) || (_directionMotor != 0) || (_positionMove != 0)) && ((millis() - _lastSeekMotor) > _stepDelay)) {
    _lastSeekMotor = millis();
    if ((_userSpeed == 0) && (_positionMove == 0) && (_directionMotor == 0)) {
      // остановка мотора в текущей фазе
      _onTheStepMotor(0);
      _isActive = false;      
      _stepDelay = _maxDelay;
      digitalWrite(_pin1, LOW);
      digitalWrite(_pin2, LOW);
      digitalWrite(_pin3, LOW);
      digitalWrite(_pin4, LOW);
    }
    else {
      _isActive = true;      
      // двигаем мотор
      if (_directionMotor  > 0) motorPositionUp(_phaseMotor);
      else if (_directionMotor < 0) motorPositionDown(_phaseMotor);
      else if (_positionMove > 0) motorPositionUp(_phaseMotor);
      else if (_positionMove < 0) motorPositionDown(_phaseMotor);
      else if (_userSpeed > 0) motorPositionUp(_phaseMotor);
      else motorPositionDown(_phaseMotor);
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
        _stepDelay = _maxDelay;
      }
      else if ((_userSpeed == 0 && _positionMove == 0) || (_directionMotor * _userSpeed < 0) || (_directionMotor * _positionMove < 0)) {
        // движение мотора в разные стороны - снижаем скорость наращивая задержку
        if (_stepDelay == _maxDelay) _directionMotor = 0; // можно менять направление
        else stepDelayBringCloser(_stepDelay, _maxDelay);
        //stepDelayUp(_stepDelay, _maxDelay);
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
}
