// функция срабатывает на событие шага двигателя от StepMotor
// индикация скороcти через зажигание светодиадов
void onTheStepMotorBubbles(int _direction) {
  static byte _activeLed = 0;

  if (_direction == 0) {
    Module.setLED(0, _activeLed);
    return;
  }
  else {
    // пузыри уже нерегулярны    
    Module.setLED(1, 6);
  }
  
  Module.setLED(0, _activeLed);
  if (_direction < 0) {    
    if (_activeLed == 0) _activeLed = 3;
    else _activeLed--;
  }
  else {
    if (_activeLed == 3) _activeLed = 0;
    else _activeLed++;
  }
  Module.setLED(1, _activeLed);
  
}
