
// Initialize object CounterForBubbles
void bubbleCounterInitialize() {
  byte _value1;

  // initialization settinggs BubbleCounter from EEPROM
  unsigned long _value4 = EEPROM.read(16);
  if (_value4 != 4294967295) CounterForBubbles.set_bubbleCounter(_value4);
  _value1 = EEPROM.read(20);
  // _maxDurationBubble
  if (_value1 == 255) {
    _value1 = CounterForBubbles.get_maxDurationBubble();
    EEPROM.update(20, _value1);
  }
  else CounterForBubbles.set_maxDurationBubble(_value1);
  // _maxDurationBubble
  _value1 = EEPROM.read(21);
  if (_value1 == 255) {
    _value1 = CounterForBubbles.get_minLevelBubble();
    EEPROM.update(21, _value1);
  }
  else CounterForBubbles.set_minLevelBubble(_value1);
}

// set bubble Speed in the BubbleSpeedControl object on the schedule if necessary
void controlBubbleSpeed(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes) {
  int _minutesBetweenLamps = 0;

  if (EEPROM.read(10) == 1) _minutesBetweenLamps = EEPROM.read(9);

  int _morningBubbles = _morningInMinutes - EEPROM.read(27);
  if (_morningBubbles < 0) _morningBubbles = _morningBubbles + 1440;
  int _eveningBubbles = _eveningInMinutes - _minutesBetweenLamps * 2;
  if (_eveningBubbles < 0) _eveningBubbles = _eveningBubbles + 1440;
  byte _needingBubbleSpeed;
  byte _needingStatus;
  if (itsDay(_nowInMinutes, _morningBubbles, _eveningBubbles)) {
    _needingBubbleSpeed = EEPROM.read(23);
    _needingStatus = EEPROM.read(24);
  }
  else {
    _needingBubbleSpeed = EEPROM.read(25);
    _needingStatus = EEPROM.read(26);
  }
  BubbleSpeedControl.set_currStatus(_needingStatus);
  BubbleSpeedControl.set_bubblesIn100Second(_needingBubbleSpeed);
}

// on the external event of the bubble inside CounterForBubbles object
void onTheBubble(byte _events) {
  bool _needDisplay = false; // есть необходимость обновить дисплей

  if ((_events & 0b00000100) == 0b00000100) Module.setLED(1, 7); // начало пузырька
  if ((_events & 0b00001000) == 0b00001000) Module.setLED(0, 7); // конец пузырька

  // индикация регулярность пузырьков
  if ((_events & 0b00010000) == 0b00010000 && !StepMotorBubbles.get_isActive()) {
    if (CounterForBubbles.get_itsRegularBubbles()) Module.setLED(0, 6);
    else Module.setLED(1, 6);
  }

  // контроль пузырьеков
  if ((_events & 0b00010000) == 0b00010000) BubbleSpeedControl.control(CounterForBubbles.get_lastDuration());

  // ловим пузырьки в специальном режиме
  if ((_events & 0b00010000) == 0b00010000 && currSettings.waitingBubble != 0) {
    // waiting bubbles in special mode
    bool _enterSensorLogs = false;
    if (currSettings.waitingBubble == 255) {
      if (CounterForBubbles.get_durationBubble() < 20) _enterSensorLogs = true;
    }
    else {
      if (--currSettings.waitingBubble == 0) _enterSensorLogs = true;
    }
    if (_enterSensorLogs) {
      currSettings.waitingBubble = 0;
      if (currSettings.setting != 0) keyEscPressed();
      if (menuItems[currMode.main][currMode.secondary] != "%X") {
        currMode.main = 4;
        currMode.secondary = 7;
        initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);
      }
      keyModePressed();
      _needDisplay = true;
    }
  }

  // обновляем меню если в нужном режиме
  if (currMode.main == 2 && currMode.secondary == 2) {
    // обсчет Min\Max
    if ((_events & 0b00000001) == 0b00000001) _needDisplay = true;
  }
  else if (currMode.main == 4 && currMode.secondary >= 5) {
    // считываний сенсора в секунду
    if ((_events & 0b00000010) == 0b00000010) _needDisplay = true;
  }
  else if ((currMode.main == 2 && currMode.secondary == 0) || (currMode.main == 4 && currMode.secondary != 0 && currMode.secondary != 1)) {
    // прочие режимы от пузырька в составе 2 и 3 на конец пузырька или ошибка
    if ((_events & 0b00010000) == 0b00010000) _needDisplay = true;
  }

  if (_needDisplay) printDisplay();

}
