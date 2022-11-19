/* Описатели шаблона:
  % - начало шаблона
  С - выводимая символьная вличина (не редактируется) номер величины между % и знаком
  H - текущее кол-во часов (редактируется)
  M - текущее кол-во минут (редактируется)
  h - кол-во часов из EEPROM (редактируется) адрес памяти между % и знаком
  m - кол-во минут из EEPROM (редактируется) адрес памяти между % и знаком
  c - выводимая символьная вличина из EEPROM (редактируется) номер величины и адрес памяти между % и знаком
  t - таймер включен, отключен
  T - выводимая текстовая информация
  Q - информация с датчиков температуры, адрес - номер датчика
  q - настройка температуры, адрес - номер настройки в EEPROM
  w - настройка люфта температуры в целях ее регулировки, адрес - номер настройки в EEPROM
  L - информация по логам температуры из heaterTempLog
  b - счетчик пузырьков абсалютный
  B - информация с счетчика пузырьков в 4-х значном формате, где адрес:
   1 - минимальный уровень сенсора,
   2 - максимальный уровень сенсора,
   3 - продолжительность пузыря,
   4 - продолжительность между пузырями,
   5 - скорость пузырьков в секунду
   6 - скорость пузырьков в минуту
   7 - считываний сенсора в секунду
  R - информация о контроле пузырьков bubbleControl в 4-х значном формате, где адрес:
   1 - текущее состояние контроля ()
   2 - отладка minBubbleDuration
   3 - отладка maxBubbleDuration
   4 - отладка minBubblesIn100Second
   5 - отладка maxBubblesIn100Second
  N - настройка необходимая скорости пузырька в секунду, адрес - номер настройки в EEPROM
  v - уровень вибрации пузырька в мс., адрес - номер настройки в EEPROM
  V - уровень отсечки сигнала пузырька, адрес - номер настройки в EEPROM
  S - пользовательская скорость мотора
  P - нужная позиция мотора, адрес - номер настройки в EEPROM
*/

BubbleControl BubbleSpeedControl;

char *menuItems[][9] = {
  {"%1C %H%M%2C%3C", "St%7n%8a %t", "Sd%0h%1m  ", "Sn%2h%3m  ", "Sb%4h%5m %6c", "Sdn %9m %10c", ""},
  {"i%1Qo%2Q", "%L", "Td%11q  %12c", "Tn%13q  %14c", "dt %15w   ", ""},
  {"%5B%1R", "%4R%5R", "%1B%2B", " %20v %21V", "Bd%23N %24c", "Bn%25N %26c", ""},
  {"SP  %S", "POS %22P", "C%b", "Min %6B", "Loop%7B", "%3B%4B", "%2R%3R", ""},
  {""}
};

byte menuPointer[][9] = {
  {B00010000, B00010000, B00010000, B00010000, B00010000, B00000000, B00000000, B00000000, 0},
  {B00100010, B01000010, B00010000, B00010000, B00010000, B00000000, B00000000, B00000000, 0},
  {B01000000, B01000100, B00000000, B00000000, B00010000, B00010000, B00000000, B00000000, 0},
  {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, 0}
};

char partsOfMenuItem[8][10];

bool itNumber(char _c) {
  return (_c == '0' || _c == '1' || _c == '2' || _c == '3' || _c == '4' || _c == '5' || _c == '6' || _c == '7' || _c == '8' || _c == '9');
}

void initPartsOfMenuItem(char _char[30]) {
  int _indexIN = 0;
  int _indexPart = 0;
  int _indexOUT = 0;
  char _c;

  for (int _i = 0; _i < 8; _i++) {
    partsOfMenuItem[_i][0] = '\0';
  }

  do {
    _c = _char[_indexIN];
    bool _endDisplayPart = false;
    if (_indexOUT > 1) if (partsOfMenuItem[_indexPart][0] == '%' && !itNumber(partsOfMenuItem[_indexPart][_indexOUT - 1])) _endDisplayPart = true;
    if ((_c == '\0' || _c == '%' || _endDisplayPart) && _indexOUT != 0) {
      partsOfMenuItem[_indexPart][_indexOUT] = '\0';
      _indexPart++;
      _indexOUT = 0;
    }
    if (_c != '\0') {
      partsOfMenuItem[_indexPart][_indexOUT] = _c;
      _indexOUT++;
    }
    _indexIN++;
  } while (_c != '\0');

}

class MenuItemPart {
  public:

    int get_isNull();
    void set_isNull(int _isNull);
    bool get_edited();
    void set_edited(bool _edited);
    char get_typeOfPart();
    byte get_value();
    void set_value(byte _value);
    void initialize(char _charMode[10], CurrSettings* _currSettingsPtr);
    void readValue(CurrSettings* _currSettingsPtr);
    void LeftRightValue(bool _left);
    void writeValue(CurrSettings* _currSettingsPtr);
    void valueToDisplay(char* charDisplay, CurrSettings* _currSettingsPtr, bool _blinkOff);

  private:

    void _addSybstring(char* _str1, int& _indexOut, char* _str2);

    byte value;
    bool edited;
    bool circleEdit;
    int isNull;
    char typeOfPart;
    char charValue[10];
    int adress;
    byte minValue;
    byte maxValue;
    byte lengthValue;
};

int MenuItemPart::get_isNull() {
  return isNull;
}

void MenuItemPart::set_isNull(int _isNull) {
  isNull = _isNull;
}

bool MenuItemPart::get_edited() {
  return edited;
}

void MenuItemPart::set_edited(bool _edited) {
  edited = _edited;
}

char MenuItemPart::get_typeOfPart() {
  return typeOfPart;
}

byte MenuItemPart::get_value() {
  return value;
}

void MenuItemPart::set_value(byte _value) {
  value = _value;
}

void MenuItemPart::initialize(char _charMode[10], CurrSettings* _currSettingsPtr) {
  char _charAdress[10];

  typeOfPart = 'E';
  value = 0;
  isNull = 0;
  charValue[0] = '\0';
  adress = 0;
  minValue = 0;
  maxValue = 0;
  edited = false;
  circleEdit = true;
  lengthValue = 0;

  if (_charMode[0] != '%') {
    typeOfPart = 'T';
    lengthValue = 0;
    while (_charMode[lengthValue] != '\0') {
      charValue[lengthValue] = _charMode[lengthValue];
      lengthValue++;
    }
    charValue[lengthValue] = '\0';
  }
  else {
    int _lengthAdress;
    for (int i = 0; _charMode[i] != '\0'; i++) {
      if (i != 0) {
        _charAdress[i - 1] = _charMode[i];
        typeOfPart = _charMode[i];
        _lengthAdress = i - 1;
      }
    }
    _charAdress[_lengthAdress] = '\0';
    adress = atoi(_charAdress);
    if (typeOfPart == 'M' || typeOfPart == 'm' || typeOfPart == 'n' || typeOfPart == 'a') {
      maxValue = 59;
      edited = true;
      lengthValue = 2;
    }
    if (typeOfPart == 'n' || typeOfPart == 'a') {
      maxValue = 59;
      edited = !_currSettingsPtr->timerOn;
      lengthValue = 2;
    }
    else if (typeOfPart == 'H' || typeOfPart == 'h') {
      maxValue = 23;
      edited = true;
      lengthValue = 2;
    }
    else if (typeOfPart == 'c' || typeOfPart == 't') {
      maxValue = 1;
      edited = true;
      lengthValue = 1;
    }
    else if (typeOfPart == 'Q') {
      maxValue = 1;
      edited = false;
      lengthValue = 3;
    }
    else if (typeOfPart == 'q') {
      minValue = 14;
      maxValue = 30;
      edited = true;
      lengthValue = 3;
    }
    else if (typeOfPart == 'w') {
      minValue = 5;
      maxValue = 10;
      edited = true;
      lengthValue = 2;
    }
    else if (typeOfPart == 'C') {
      maxValue = 1;
      edited = false;
      lengthValue = 1;
    }
    else if (typeOfPart == 'L') {
      maxValue = 95;
      edited = true;
      circleEdit = false;
      lengthValue = 8;
    }
    else if (typeOfPart == 'b') {
      edited = false;
      lengthValue = 7;
    }
    else if (typeOfPart == 'B' || typeOfPart == 'R') {
      edited = false;
      lengthValue = 4;
    }
    else if (typeOfPart == 'v') {
      minValue = 1;
      maxValue = 10;
      edited = true;
      lengthValue = 3;
    }
    else if (typeOfPart == 'V') {
      maxValue = 250;
      edited = true;
      lengthValue = 3;
    }
    else if (typeOfPart == 'S') {
      minValue = 68;
      maxValue = 132;
      edited = true;
      lengthValue = 4;
    }
    else if (typeOfPart == 'P') {
      maxValue = 250;
      edited = true;
      lengthValue = 4;
    }
    else if (typeOfPart == 'N') {
      minValue = 19;
      maxValue = 250;
      edited = true;
      circleEdit = false;
      lengthValue = 4;
    }
  }
}

void MenuItemPart::readValue(CurrSettings* _currSettingsPtr) {

  bool checkFromEEPROM = false;

  if (typeOfPart == 'H') value = _currSettingsPtr->now.hour;
  else if (typeOfPart == 'M') value = _currSettingsPtr->now.minute;
  else if (typeOfPart == 'L') value = 95;
  else if (typeOfPart == 'b' || typeOfPart == 'B' || typeOfPart == 'R') value = 0;
  else if (typeOfPart == 't') {
    if (_currSettingsPtr->timerOn) value = 1;
    else value = 0;
  }
  else if (typeOfPart == 'C' && adress == 3) {
    value = EEPROM.read(6);
    checkFromEEPROM = true;
  }
  else if (typeOfPart == 'E' || typeOfPart == 'C' || typeOfPart == 'T' || typeOfPart == 'Q') value = 0;
  else if (typeOfPart == 'n' && _currSettingsPtr->timerOn) value = _currSettingsPtr->timerMinute;
  else if (typeOfPart == 'a' && _currSettingsPtr->timerOn) value = _currSettingsPtr->timerSecond;
  else if (typeOfPart == 'S') value = StepMotorBubbles.get_userSpeed() + 100;
  else {
    value = EEPROM.read(adress);
    checkFromEEPROM = true;
  }

  if (checkFromEEPROM) {
    if (value < minValue) value = minValue;
    if (value > maxValue) value = maxValue;
  }
}

void MenuItemPart::LeftRightValue(bool _left) {

  if ((_left) && (value == minValue))  {
    if (circleEdit) value = maxValue;
  }
  else if ((!_left) && (value == maxValue)) {
    if (circleEdit) value = minValue;
  }
  else if (_left) value--;
  else value++;
}

void MenuItemPart::writeValue(CurrSettings* _currSettingsPtr) {

  if (typeOfPart == 'H') {
    _currSettingsPtr->now.hour = value;
    Rtc.setTime(currSettings.now);
  }
  else if (typeOfPart == 'M') {
    _currSettingsPtr->now.minute = value;
    Rtc.setTime(currSettings.now);
  }
  else if (typeOfPart == 't') {
    if (value == 1) {
      _currSettingsPtr->timerOn = true;
      _currSettingsPtr->timerMinute = EEPROM.read(7);
      _currSettingsPtr->timerSecond = EEPROM.read(8);
    }
    else {
      if (_currSettingsPtr->timerOn) _currSettingsPtr->setting = 3;
      _currSettingsPtr->timerOn = false;
    }
  }
  else if (typeOfPart == 'v') {
    EEPROM.update(adress, value);
    CounterForBubbles.set_bubbleVibration(value);
  }
  else if (typeOfPart == 'V') {
    EEPROM.update(adress, value);
    CounterForBubbles.set_minBubbleLevel(value + 100);
  }
  else if (typeOfPart == 'L') {
    value = 95;
  }
  else if (typeOfPart == 'S') {
    StepMotorBubbles.set_userSpeed(value - 100);
  }
  else if (typeOfPart == 'P') {
    EEPROM.update(adress, value);
    StepMotorBubbles.set_positionMove(value - 125);
  }
  else if (edited) {
    EEPROM.update(adress, value);
  }
}

void MenuItemPart::_addSybstring(char* _str1, int& _indexOut, char* _str2) {
  for (int i = 0; _str2[i] != '\0'; i++) {
    _str1[_indexOut] = _str2[i];
    _indexOut++;
  }
}

void MenuItemPart::valueToDisplay(char* charDisplay, CurrSettings* _currSettingsPtr, bool _blinkOff) {
  int _indexOut = 0;
  char _strValue[10];

  while (charDisplay[_indexOut] != '\0') {
    _indexOut++;
  }

  if (typeOfPart == 'C') {
    if (adress == 1) {
      if (_currSettingsPtr->nowDay) _addSybstring(charDisplay, _indexOut, "d");
      else _addSybstring(charDisplay, _indexOut, "n");
    }
    else if (adress == 2) {
      if (_currSettingsPtr->timerOn) _addSybstring(charDisplay, _indexOut, "t");
      else _addSybstring(charDisplay, _indexOut, " ");
    }
    else if (adress == 3) {
      if (value == 1) _addSybstring(charDisplay, _indexOut, "b");
      else _addSybstring(charDisplay, _indexOut, " ");
    }
    else _addSybstring(charDisplay, _indexOut, "E");
  }
  else if (typeOfPart == 'T') _addSybstring(charDisplay, _indexOut, charValue);
  else if (typeOfPart == 'Q') {
    float _floatValue;
    if (adress == 2 & _currSettingsPtr->aquaTempErr) _addSybstring(charDisplay, _indexOut, "Err");
    else {
      if (adress == 1) _floatValue = Rtc.getTemperatureFloat();
      else _floatValue = _currSettingsPtr->aquaTemp;
      int _intValue = _floatValue * 10;
      sprintf(_strValue, "%03d", _intValue);
      _addSybstring(charDisplay, _indexOut, _strValue);
    }
  }
  else if (_blinkOff) {
    for (int i = 0; i < lengthValue; i++) {
      charDisplay[_indexOut] = ' ';
      _indexOut++;
    }
  }
  else if (typeOfPart == 'q') {
    sprintf(_strValue, "%02d0", value);
    _addSybstring(charDisplay, _indexOut, _strValue);
  }
  else if (typeOfPart == 'b') {
    unsigned long _longValue = CounterForBubbles.get_bubbleCounter();
    sprintf(_strValue, "%7d", _longValue);
    _addSybstring(charDisplay, _indexOut, _strValue);
  }
  else if (typeOfPart == 'B') {
    int _intValue;

    switch (adress) {
      case 1: _intValue = CounterForBubbles.get_MinLevel(); break;
      case 2: _intValue = CounterForBubbles.get_MaxLevel(); break;
      case 3: _intValue = CounterForBubbles.get_durationBubble(); break;
      case 4: _intValue = CounterForBubbles.get_durationNoBubble(); break;
      case 5: _intValue = CounterForBubbles.get_bubbleIn100Second(); break;
      case 6: _intValue = CounterForBubbles.get_bubbleInMinute(); break;
      case 7: _intValue = CounterForBubbles.get_sensorInSecond(); break;
    }

    switch (_intValue) {
      case -1: _addSybstring(charDisplay, _indexOut, "Err1"); break;
      case -2: _addSybstring(charDisplay, _indexOut, "Err2"); break;
      default: sprintf(_strValue, "%4d", _intValue); _addSybstring(charDisplay, _indexOut, _strValue); break;
    }
  }
  else if (typeOfPart == 'R') {

    switch (adress) {
      case 1: BubbleSpeedControl.get_condition(_strValue); break;
      case 2: sprintf(_strValue, "%4d", BubbleSpeedControl.get_minBubbleDuration()); break;
      case 3: sprintf(_strValue, "%4d", BubbleSpeedControl.get_maxBubbleDuration()); break;
      case 4: sprintf(_strValue, "%4d", BubbleSpeedControl.get_minBubblesIn100Second()); break;
      case 5: sprintf(_strValue, "%4d", BubbleSpeedControl.get_maxBubblesIn100Second()); break;
    }
    _addSybstring(charDisplay, _indexOut, _strValue);
  }
  else if (typeOfPart == 'L') {
    byte _indexOfNow = _currSettingsPtr->now.hour * 4;
    if (_currSettingsPtr->now.minute >= 45) _indexOfNow = _indexOfNow + 3;
    else if (_currSettingsPtr->now.minute >= 30) _indexOfNow = _indexOfNow + 2;
    else if (_currSettingsPtr->now.minute >= 15) _indexOfNow = _indexOfNow + 1;

    byte _indexOfLog;
    if ((95 - value) > _indexOfNow) _indexOfLog = _indexOfNow + value + 1;
    else _indexOfLog = _indexOfNow + value - 95;

    // печатаем на экран
    byte _toPrint;
    _toPrint = _indexOfLog / 4;
    sprintf(_strValue, "%02d", _toPrint);
    _addSybstring(charDisplay, _indexOut, _strValue);
    _toPrint = _indexOfLog % 4 * 15;
    sprintf(_strValue, "%02d", _toPrint);
    _addSybstring(charDisplay, _indexOut, _strValue);
    word _valueOfLog = heaterTempLog[_indexOfLog];
    if (_valueOfLog > 10000) {
      charDisplay[_indexOut] = 'o';
      _valueOfLog = _valueOfLog - 10000;
    }
    else charDisplay[_indexOut] = ' ';
    _indexOut++;
    if (_valueOfLog == 0) _addSybstring(charDisplay, _indexOut, "Err");
    else {
      _valueOfLog = _valueOfLog - 1000;
      sprintf(_strValue, "%03d", _valueOfLog);
      _addSybstring(charDisplay, _indexOut, _strValue);
    }
  }
  else if (typeOfPart == 'S') {
    sprintf(_strValue, "%4d", value - 100);
    _addSybstring(charDisplay, _indexOut, _strValue);
  }
  else if (typeOfPart == 'P') {
    sprintf(_strValue, "%4d", value - 125);
    _addSybstring(charDisplay, _indexOut, _strValue);
  }
  else if (typeOfPart == 'N') {
    if (value == 19) _addSybstring(charDisplay, _indexOut, "Lock");
    else {
      sprintf(_strValue, "%4d", value);
      _addSybstring(charDisplay, _indexOut, _strValue);
    }
  }
  else if (typeOfPart == 'V') {
    sprintf(_strValue, "%3d", value + 100);
    _addSybstring(charDisplay, _indexOut, _strValue);
  }
  else {
    switch (lengthValue) {
      case 1: sprintf(_strValue, "%01d", value); break;
      case 2: sprintf(_strValue, "%02d", value); break;
      case 3: sprintf(_strValue, "%3d", value); break;
      case 4: sprintf(_strValue, "%4d", value); break;
      case 6: sprintf(_strValue, "%6d", value); break;
    }
    _addSybstring(charDisplay, _indexOut, _strValue);
  }
  charDisplay[_indexOut] = '\0';
}
