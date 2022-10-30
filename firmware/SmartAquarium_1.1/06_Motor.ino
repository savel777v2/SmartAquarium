// функция срабатывает на событие шага двигателя от StepMotor
// индикация скороcти через зажигание светодиадов
void onTheStepMotorBubbles(int _direction) {
  static byte _multiplierForLed = 8;
  static byte _activeLed = 0;

  if (_direction == 0) {
    _multiplierForLed = 8;
    Module.setLED(0, _activeLed);
    return;
  }
  if (++_multiplierForLed != 9) return;

  _multiplierForLed = 0;
  Module.setLED(0, _activeLed);
  if (_direction < 0) {    
    if (_activeLed == 1 || _activeLed > 3) _activeLed = 3;
    else _activeLed--;
  }
  else {
    if (_activeLed == 6 || _activeLed < 4) _activeLed = 4;
    else _activeLed++;
  }
  Module.setLED(1, _activeLed);
}
