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

*/
char *menuItems[][7] = {
  {"%1C %H%M%2C%3C", "St%7n%8a %t", "Sd%0h%1m  ", "Sn%2h%3m  ", "Sb%4h%5m %6c", "Sdn %9m %10c", ""},
  {"i%1Qo%2Q", "Td%11q  %12c", "Tn%13q  %14c", "dt %15w   ", "%L"},
  {""}
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
};

void MenuItemPart::set_isNull(int _isNull) {
  isNull = _isNull;
};

bool MenuItemPart::get_edited() {
  return edited;
};

void MenuItemPart::set_edited(bool _edited) {
  edited = _edited;
};

char MenuItemPart::get_typeOfPart() {
  return typeOfPart;
};

byte MenuItemPart::get_value() {
  return value;
};

void MenuItemPart::set_value(byte _value) {
  value = _value;
};

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
    if (typeOfPart == 'M' || typeOfPart == 'm' || typeOfPart == 'n' || typeOfPart == 'S' || typeOfPart == 's' || typeOfPart == 'a') {
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
  }
};

void MenuItemPart::readValue(CurrSettings* _currSettingsPtr) {

  bool checkFromEEPROM = false;

  if (typeOfPart == 'H') value = _currSettingsPtr->now.hour;
  else if (typeOfPart == 'M') value = _currSettingsPtr->now.minute;
  else if (typeOfPart == 'S') value = _currSettingsPtr->now.second;
  else if (typeOfPart == 'L') value = 95;
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
  else {
    value = EEPROM.read(adress);
    checkFromEEPROM = true;
  }

  if (checkFromEEPROM) {
    if (value < minValue) {
      value = minValue;
      //EEPROM.update(adress, value);
    }
    if (value > maxValue) {
      value = maxValue;
      //EEPROM.update(adress, value);
    }
  }
};

void MenuItemPart::LeftRightValue(bool _left) {

  if ((_left) && (value == minValue))  {
    if (circleEdit) value = maxValue;
  }
  else if ((!_left) && (value == maxValue)) {
    if (circleEdit) value = minValue;
  }
  else if (_left) value--;
  else value++;
};

void MenuItemPart::writeValue(CurrSettings* _currSettingsPtr) {

  if (typeOfPart == 'H') {
    _currSettingsPtr->now.hour = value;
    Rtc.setTime(currSettings.now);
  }
  else if (typeOfPart == 'M') {
    _currSettingsPtr->now.minute = value;
    Rtc.setTime(currSettings.now);
  }
  else if (typeOfPart == 'S') {
    _currSettingsPtr->now.second = value;
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
  else if (typeOfPart == 'L') value = 95;
  else if (edited) {
    EEPROM.update(adress, value);
  }
};

void MenuItemPart::valueToDisplay(char* charDisplay, CurrSettings* _currSettingsPtr, bool _blinkOff) {
  int _indexOut = 0;
  char _strValue[10];

  while (charDisplay[_indexOut] != '\0') {
    _indexOut++;
  }

  if (typeOfPart == 'C') {
    char _addChar;
    if (adress == 1) {
      if (_currSettingsPtr->nowDay) _addChar = 'd';
      else _addChar = 'n';
    }
    else if (adress == 2) {
      if (_currSettingsPtr->timerOn) _addChar = 't';
      else _addChar = ' ';
    }
    else if (adress == 3) {
      if (value == 1) _addChar = 'b';
      else _addChar = ' ';
    }
    else _addChar = 'E';
    charDisplay[_indexOut] = _addChar;
    _indexOut++;
  }
  else if (typeOfPart == 'T') {
    for (int i = 0; charValue[i] != '\0'; i++) {
      charDisplay[_indexOut] = charValue[i];
      _indexOut++;
    }
  }
  else if (typeOfPart == 'Q') {
    float _floatValue;
    if (adress == 2 & _currSettingsPtr->aquaTempErr) {
      _strValue[0] = 'E';
      _strValue[1] = 'r';
      _strValue[2] = 'r';
    }
    else {
      if (adress == 1) _floatValue = Rtc.getTemperatureFloat();
      else _floatValue = _currSettingsPtr->aquaTemp;
      int _intValue = _floatValue * 10;
      sprintf(_strValue, "%03d", _intValue);
    }
    for (int i = 0; i < 3; i++) {
      charDisplay[_indexOut] = _strValue[i];
      _indexOut++;
    }
  }
  else if (_blinkOff) {
    for (int i = 0; i < lengthValue; i++) {
      charDisplay[_indexOut] = ' ';
      _indexOut++;
    }
  }
  else if (typeOfPart == 'q') {
    sprintf(_strValue, "%02d", value);
    charDisplay[_indexOut] = _strValue[0];
    _indexOut++;
    charDisplay[_indexOut] = _strValue[1];
    _indexOut++;
    charDisplay[_indexOut] = '0';
    _indexOut++;
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
    charDisplay[_indexOut] = _strValue[0];
    _indexOut++;
    charDisplay[_indexOut] = _strValue[1];
    _indexOut++;
    _toPrint = _indexOfLog % 4 * 15;
    sprintf(_strValue, "%02d", _toPrint);
    charDisplay[_indexOut] = _strValue[0];
    _indexOut++;
    charDisplay[_indexOut] = _strValue[1];
    _indexOut++;
    word _valueOfLog = heaterTempLog[_indexOfLog];
    if (_valueOfLog > 10000) {
      charDisplay[_indexOut] = 'o';
      _valueOfLog = _valueOfLog - 10000;
    }
    else charDisplay[_indexOut] = ' ';
    _indexOut++;
    if (_valueOfLog == 0) {
      charDisplay[_indexOut] = 'E';
      _indexOut++;
      charDisplay[_indexOut] = 'r';
      _indexOut++;
      charDisplay[_indexOut] = 'r';
      _indexOut++;
    }
    else {
      _valueOfLog = _valueOfLog - 1000;
      sprintf(_strValue, "%03d", _valueOfLog);
      for (int i = 0; i < 3; i++) {
        charDisplay[_indexOut] = _strValue[i];
        _indexOut++;
      }
    }
  }
  else {
    if (lengthValue == 1) {
      sprintf(_strValue, "%01d", value);
      charDisplay[_indexOut] = _strValue[0];
      _indexOut++;
    }
    else if (lengthValue == 2) {
      sprintf(_strValue, "%02d", value);
      charDisplay[_indexOut] = _strValue[0];
      _indexOut++;
      charDisplay[_indexOut] = _strValue[1];
      _indexOut++;
    }
  }
  charDisplay[_indexOut] = '\0';
};
