void controlTempLoop() {
  // temp reader and display
  if (controlTemp.loopNeedDisplay() && menu.getSubmenu() == curTemp) menu.display();  
}

void feedingLoop() {
  // Feeding and display
  if (feeding.loopNeedDisplay() && menu.getSubmenu() == feedingMenu) menu.display();
}

void bubbleCounterControlLoop() {
  // loop BubbleCounter and display
  byte needDisplayCounter = bubbleCounter.loopNeedDisplay();
  if ((needDisplayCounter & 0b00000001) == 0b00000001 && menu.getSubmenu() == sensorValue) menu.display();
  if ((needDisplayCounter & 0b00000100) == 0b00000100) module.setLED(1, 7); // начало пузырька
  if ((needDisplayCounter & 0b00001000) == 0b00001000) module.setLED(0, 7); // конец пузырька
  if ((needDisplayCounter & 0b00010000) == 0b00010000) {
    // контроль пузырьков - по ошибке или пузырьку
    if (bubbleControl.controlWaiting()) module.setLED(1, 6);
    else module.setLED(0, 6);
    if (menu.getSubmenu() == bubblesInSecond) menu.display();
  }
}

void stepMotorLoop() {
  static byte activeLedMotor = 0;
  
  // loop StepMotor and display
  int _direction = stepMotor.loopDirection();
  if (_direction != 255) {
    module.setLED(0, activeLedMotor);
    if (_direction < 0) activeLedMotor = activeLedMotor == 0 ? 3 : activeLedMotor - 1;
    else if (_direction > 0) activeLedMotor = activeLedMotor == 3 ? 0 : activeLedMotor + 1;
    if (_direction != 0) module.setLED(1, activeLedMotor);
  }
}

void timerLoop() {
  // loop timer
  if (currSettings.timer != nullptr) {
    int needDisplay = currSettings.timer->loopNeedDisplay();
    if (needDisplay == -1) {
      delete currSettings.timer;
      currSettings.timer = nullptr;
      if (currSettings.alarmMelody == nullptr) currSettings.alarmMelody = new Melody();
      else (currSettings.alarmMelody->restart());
      if (menu.getSubmenu() == timeMenu || menu.getSubmenu() == timer) {
        menu.display();
      }
    }
    else if (needDisplay == 1 && menu.getSubmenu() == timer) menu.display();
  }
}

void melodyLoop() {
  // turn off alarm Melody
  if (currSettings.alarmMelody != nullptr) {
    int needLoop = currSettings.alarmMelody->loopNeedLoop();
    if (!needLoop) {
      delete currSettings.alarmMelody;
      currSettings.alarmMelody = nullptr;
    }
  }
}
