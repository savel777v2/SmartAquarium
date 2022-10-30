// universal class for step motor

class StepMotor {
  public:

    StepMotor(int pin1, int pin2, int pin3, int pin4, void (*function)(int _direction));
    void set_userSpeed(int userSpeed);
    int get_userSpeed();
    void set_positionMotor(long positionMotor);
    long get_positionMotor();
    void tick();

  private:

    void (*_onTheStepMotor)(int _direction);
    int getStepDelay(int _UserSpeed); // функция расчета задержки шага мотора от пользовательской скорости мотора
    void stepDelayUp(int& stepDelay, int needStepDelay); // задержку приблизим вверх
    void stepDelayDown(int& stepDelay, int needStepDelay); // задержку приблизим вниз
    void motorPositionUp(byte& phaseMotor); // шаг мотора вверх
    void motorPositionDown(byte& phaseMotor); // шаг мотора вниз

    int _pin1, _pin2,  _pin3, _pin4;
    long _positionMotor = 0; // относительная позиция мотора
    int _userSpeed = 0; // нужное направление и скорость текущего движения мотора
    int _userDelayMotor = _minDelay * _maxUserSpeed; // нужная задержка мотора (зависит от userSpeed)
    int _minDelay = 4; // минимальная задержка между шагом мотора в мс., соответсвует _maxUserSpeed
    int _maxUserSpeed = 32; // максимально допустимое значение линейной пользовательской скорости

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
  static int _maxDelay = _minDelay * _maxUserSpeed;
  if (userSpeed == 0) return _maxDelay;
  float _Delay = (float) _maxDelay / userSpeed;
  return round(abs(_Delay));
}

// задержку приблизим вверх
void StepMotor::stepDelayUp(int& stepDelay, int needStepDelay) {
  float _deltaDelayMotor = stepDelay * 0.25;
  stepDelay = stepDelay + round(_deltaDelayMotor);
  if (stepDelay > needStepDelay) stepDelay = needStepDelay;
}

// задержку приблизим вниз
void StepMotor::stepDelayDown(int& stepDelay, int needStepDelay) {
  float _deltaDelayMotor = stepDelay * 0.25;
  stepDelay = stepDelay - round(_deltaDelayMotor);
  if (stepDelay < needStepDelay) stepDelay = needStepDelay;
}

// шаг мотора вверх
void StepMotor::motorPositionUp(byte& phaseMotor) {
  _positionMotor++;
  if (phaseMotor == 3) phaseMotor = 0;
  else phaseMotor++;
  _onTheStepMotor(+1);
}

// шаг мотора вниз
void StepMotor::motorPositionDown(byte& phaseMotor) {
  _positionMotor--;
  if (phaseMotor == 0) phaseMotor = 3;
  else phaseMotor--;
  _onTheStepMotor(-1);
}

void StepMotor::set_userSpeed(int userSpeed) {
  if (userSpeed < -1 * _maxUserSpeed) _userSpeed = -1 * _maxUserSpeed;
  else if (userSpeed > _maxUserSpeed) _userSpeed = _maxUserSpeed;
  else _userSpeed = userSpeed;
  _userDelayMotor = getStepDelay(_userSpeed);
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


void StepMotor::tick() {
  static unsigned long _lastSeekMotor = 0; // время последнего шага мотора
  static int _directionMotor = 0; // текущее направление мотора
  static int _stepDelay = _minDelay * _maxUserSpeed; // текущая задержка шага мотора
  static int _maxDelay = _minDelay * _maxUserSpeed;
  static byte _phaseMotor = 0; // фаза мотора 0-3

  if (((_lastSeekMotor != 0) || (_userSpeed != 0) || (_directionMotor != 0)) && ((millis() - _lastSeekMotor) > _stepDelay)) {
    if ((_userSpeed == 0) && (_directionMotor == 0)) {
      // остановка мотора в текущей фазе
      _onTheStepMotor(0);
      _lastSeekMotor = 0;
      digitalWrite(_pin1, LOW);
      digitalWrite(_pin2, LOW);
      digitalWrite(_pin3, LOW);
      digitalWrite(_pin4, LOW);
    }
    else {
      _lastSeekMotor = millis();
      // двигаем мотор
      if (_directionMotor  > 0) motorPositionUp(_phaseMotor);
      else if (_directionMotor < 0) motorPositionDown(_phaseMotor);
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
        if (_userSpeed < 0) _directionMotor = -1;
        else _directionMotor = 1;
        _stepDelay = _maxDelay;
      }
      else if ((_userSpeed == 0) || (_directionMotor * _userSpeed < 0)) {
        // движение мотора в разные стороны - снижаем скорость наращивая задержку
        if (_stepDelay == _maxDelay) _directionMotor = 0; // можно менять направление
        else stepDelayUp(_stepDelay, _maxDelay);
      }
      else {
        // движение мотора в одну сторону - приближаем задержку к нужной
        if (_stepDelay > _userDelayMotor) stepDelayDown(_stepDelay, _userDelayMotor);
        else if (_stepDelay < _userDelayMotor) stepDelayUp(_stepDelay, _userDelayMotor);
      }

    }
  }
}
