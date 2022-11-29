void readKeyboard() {
  static unsigned long _LastKeyboardTime = 0; // последнее время считывания клавиатуры
  bool _needDisplay = false; // есть необходимость обновить дисплей

  if ((millis() - _LastKeyboardTime) > KEYBOARD_INTERVAL) {
    _LastKeyboardTime = millis();
    byte Keys = Module.getButtons();
    if (Keys != 255)
    {
      if (keyPressed(Keys, 0, 0)) _needDisplay = keyEscPressed();
      if (keyPressed(Keys, 1, 0)) _needDisplay = keyModePressed();
      if (keyPressed(Keys, 2, 1)) _needDisplay = keyLeftRightPressed(true);
      if (keyPressed(Keys, 3, 1)) _needDisplay = keyLeftRightPressed(false);
      if (keyPressed(Keys, 4, 1)) _needDisplay = keyDownUpPressed(true);
      if (keyPressed(Keys, 5, 1)) _needDisplay = keyDownUpPressed(false);
      if (keyPressed(Keys, 6, 1)) _needDisplay = keySavePressed();
      if (keyPressed(Keys, 7, 0)) _needDisplay = keyLampsPressed();
    }
  }

  if (_needDisplay) printDisplay();

}

// обработка нажатия клавиши Save
bool keySavePressed() {
  /*unsigned long _value4 = CounterForBubbles.get_bubbleCounter();
  EEPROM.update(16, _value4);*/
  /*byte _positionMove = EEPROM.read(22);
  StepMotorBubbles.set_positionMove(_positionMove - 125);*/
  /*StepMotorBubbles.set_userSpeed(0);
  printDisplay();*/
  BubbleSpeedControl.clearError();
  printDisplay();
  tone(PIEZO_PIN, 2500, 100);
}

// обработка нажатия клавиши Lamps
bool keyLampsPressed() {
  byte _newLampLevel;

  if (currSettings.manualLamp == 2) {
    currSettings.manualLamp = 0;
  }
  else {
    currSettings.manualLamp++;
    if (currSettings.manualLamp == 1) {
      if (currSettings.nowDay) _newLampLevel = 0;
      else _newLampLevel = 1;
    }
    else {
      if (lampPinsLevel[0][1] == 0) _newLampLevel = 1;
      else _newLampLevel = 0;
    }
    for (int i = 0; i < 3; i++) lampPinsLevel[i][1] = _newLampLevel;
  }
  conditionControl();
  return false;

}

// обработка нажатия клавиш Left,Right
bool keyLeftRightPressed(bool _left) {

  if (EditingMenuItemPart.get_isNull() == 0) {
    EditingMenuItemPart.LeftRightValue(_left);
    settMode.lastBlinkTime = millis();
    settMode.blinkOff = false;
    return true;
  }

  if (currMode.secondary != 0) return false;
  if ((!_left) && (menuItems[currMode.main + 1][currMode.secondary][0] == '\0')) return false;
  if ((_left) && (currMode.main == 0)) return false;

  if (_left) currMode.main--;
  else currMode.main++;

  initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);

  return true;

}

// обработка нажатия клавиш Down,Up
bool keyDownUpPressed(bool _down) {

  if (currSettings.setting != 0) return false;

  if ((_down) && (menuItems[currMode.main][currMode.secondary + 1][0] == '\0')) return false;
  if ((!_down) && (currMode.secondary == 0)) return false;

  if (_down) currMode.secondary++;
  else currMode.secondary--;

  initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);

  return true;
}

// обработка нажатия клавиши Esc
bool keyEscPressed() {

  /*Serial.print("Esc 0: ");
    debugEditingMenuItemPart();*/

  if (currSettings.alarmStartSound != 0) {
    currSettings.alarmStartSound = 0;
    return false;
  }

  if (currSettings.setting != 0) {
    EditingMenuItemPart.set_isNull(1);
    currSettings.setting = 0;
    settMode.blinkOff = false;
  }
  else if (currMode.secondary != 0) {
    currMode.secondary = 0;
    initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);
  }
  else if (currMode.main != 0) {
    currMode.main = 0;
    initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);
  }
  else return false;

  /*Serial.print("Esc 1: ");
   debugEditingMenuItemPart();*/

  return true;
}

// обработка нажатия клавиши Mode
bool keyModePressed() {
  bool _nextSetting = true;
  int _indexSetting = 0;
  bool _findSetting = false;

  /*Serial.print("Mode 0: ");
   debugEditingMenuItemPart();*/

  if (EditingMenuItemPart.get_isNull() == 0) {
    EditingMenuItemPart.writeValue(&currSettings);
    EditingMenuItemPart.set_isNull(1);
    // settings change - all need to control
    conditionControl();
  }

  currSettings.setting++;

  for (int _i = 0; (_i < 8 && !_findSetting && partsOfMenuItem[_i][0] != '\0') ; _i++) {
    EditingMenuItemPart.initialize(partsOfMenuItem[_i], &currSettings);
    if (EditingMenuItemPart.get_edited()) {
      _indexSetting++;
      if (currSettings.setting == _indexSetting) {
        _findSetting  = true;
      }
    }
  }

  if (_findSetting) EditingMenuItemPart.readValue(&currSettings);
  else {
    currSettings.setting = 0;
    settMode.blinkOff = false;
    EditingMenuItemPart.set_isNull(1);
  }

  /*Serial.print("Mode 1: ");
   debugEditingMenuItemPart();*/

  return true;
}

// Функция анализа нажатия клавиатуры
//  _keys - байт с нажатыми клавишами
//  _key - индекс клавишы 0-7
//  _mode - режим - 0 - обычное нажатие
//        1 - режим удержания более 1 сек - быстрыее, 5 сек - еще быстрее
//
boolean keyPressed(byte _keys, int _key, int _mode) {
  static unsigned long keyTimePressed[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // нач. время нажатия клавиш
  static unsigned long keyTimeLoopPressed[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // время счетчика начала удержания клавиши

  if ((_keys & (1 << _key)) == (1 << _key)) {
    if (keyTimePressed[_key] == 0) {
      keyTimePressed[_key] = millis();
      return true;
    }
    else if ((_mode == 1) && ((millis() - keyTimePressed[_key]) > 5000) && ((millis() - keyTimeLoopPressed[_key]) > 100)) {
      keyTimeLoopPressed[_key] = millis();
      return true;
    }
    else if ((_mode == 1) && ((millis() - keyTimePressed[_key]) > 1000) && ((millis() - keyTimeLoopPressed[_key]) > 200)) {
      keyTimeLoopPressed[_key] = millis();
      return true;
    }
  }
  else keyTimePressed[_key] = 0;
  return false;
}
