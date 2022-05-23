#include <TM1638.h>
#include <EEPROM.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 Rtc;

#define DISPLAY_INTERVAL 0
#define KEYBOARD_INTERVAL 10
#define PIEZO_PIN 8

struct CurrSettings {
  DateTime now;
  byte alarmOn;
  bool nowMorning;
  bool timerOn;
  byte timerMinute;
  byte timerSecond;
  byte setting;
};

struct {
  byte main = 0;
  byte secondary = 0;
} currMode;

struct {
  bool blinkOff;
  unsigned long lastBlinkTime = 0;
} settMode;

CurrSettings currSettings;

/* Описатели шаблона:
  % - начало шаблона
  С - выводимая символьная вличина (не редактируется) номер величины между % и знаком
  H - текущее кол-во часов (редактируется)
  M - текущее кол-во минут (редактируется)
  h - кол-во часов из EEPROM (редактируется) адрес памяти между % и знаком
  m - кол-во минут из EEPROM (редактируется) адрес памяти между % и знаком
  c - выводимая символьная вличина из EEPROM (редактируется) номер величины и адрес памяти между % и знаком
*/

char *displayMode[][5] = {
  {"%1C %H%M%2C%3C", "St%7n%8a %t", "Sd%0h%1m  ", "Sn%2h%3m  ", "Sb%4h%5m %6c"}
};

char modeForParts[8][10];

class ModePart {
  public:

    int get_isNull() {
      return isNull;
    };

    void set_isNull(int _isNull) {
      isNull = _isNull;
    };

    bool get_edited() {
      return edited;
    };

    void set_edited(bool _edited) {
      edited = _edited;
    };

    char get_typeOfPart() {
      return typeOfPart;
    };
    
    byte get_value() {
      return value;
    };

    void set_value(byte _value) {
      value = _value;
    };

    void initialize(char _charMode[10], CurrSettings* _currSettingsPtr) {
      char _charAdress[10];

      typeOfPart = 'E';
      value = 0;      
      isNull = 0;
      charValue[0] = '\0';
      adress = 0;
      minValue = 0;
      maxValue = 0;
      edited = false;
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
        else if (typeOfPart == 'C') {
          maxValue = 1;
          edited = false;
          lengthValue = 1;
        }
      }
    };

    void readValue(CurrSettings* _currSettingsPtr) {

      bool checkFromEEPROM = false;

      if (typeOfPart == 'H') value = _currSettingsPtr->now.hour;
      else if (typeOfPart == 'M') value = _currSettingsPtr->now.minute;
      else if (typeOfPart == 'S') value = _currSettingsPtr->now.second;
      else if (typeOfPart == 't') {
        if (_currSettingsPtr->timerOn) value = 1;
        else value = 0;
      }
      else if (typeOfPart == 'C' && adress == 3) {
        value = EEPROM.read(6);
        checkFromEEPROM = true;
      }
      else if (typeOfPart == 'E' || typeOfPart == 'C' || typeOfPart == 'T') value = 0;
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

    void LeftRightValue(bool _left) {

      if ((_left) && (value == minValue)) value = maxValue;
      else if ((!_left) && (value == maxValue)) value = minValue;
      else if (_left) value--;
      else value++;
    };

    void writeValue(CurrSettings* _currSettingsPtr) {

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
      else if (edited) {
        EEPROM.update(adress, value);
      }
    };

    void valueToDisplay(char* charDisplay, CurrSettings* _currSettingsPtr, bool _blinkOff) {
      int _indexOut = 0;
      char _strValue[2];

      while (charDisplay[_indexOut] != '\0') {
        _indexOut++;
      }

      if (typeOfPart == 'C') {
        char _addChar;
        if (adress == 1) {
          if (_currSettingsPtr->nowMorning) _addChar = 'd';
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
      else if (_blinkOff) {
        for (int i = 0; i < lengthValue; i++) {
          charDisplay[_indexOut] = ' ';
          _indexOut++;
        }
      }
      else {
        if (lengthValue == 1) {
          sprintf(_strValue, "%1d", value);
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

  private:

    byte value;
    bool edited;    
    int isNull;
    char typeOfPart;
    char charValue[10];
    int adress;
    byte minValue;
    byte maxValue;
    byte lengthValue;

};

ModePart EditingModePart;

void setup() {

  pinMode(PIEZO_PIN, OUTPUT); // настраиваем вывод 2 на выход
  tone(PIEZO_PIN, 200);
  delay(500);
  noTone(PIEZO_PIN);

  /*Serial.begin(9600);
  Serial.println("debugging");
  Serial.print("Esc setting != 0: ");     
  debugEditingModePart();*/

  initModeForParts(displayMode[currMode.main][currMode.secondary]);
  EditingModePart.set_isNull(1);

}

/*void debugEditingModePart() {
  Serial.print(EditingModePart.get_typeOfPart());
  Serial.print(",isNull: ");
  Serial.print(EditingModePart.get_isNull());
  Serial.print(",value: ");
  Serial.println(EditingModePart.get_value());
}*/

void loop() {

  loopTime();
  readKeyboard();

}

byte readEEPROM(int _adress) {
  byte _value = EEPROM.read(_adress);
  if (_value == 255) return 0;
  else return _value;
}

void readKeyboard() {
  static unsigned long _LastKeyboardTime = 0; // последнее время считывания клавиатуры
  bool _needDisplay = false; // есть необходимость обновить дисплей
#define KEYBOARD_INTERVAL 10

  if ((millis() - _LastKeyboardTime) > KEYBOARD_INTERVAL) {
    _LastKeyboardTime = millis();
    byte Keys = Module.getButtons();
    if (keyPressed(Keys, 0, 0)) _needDisplay = keyEscPressed();
    if (keyPressed(Keys, 1, 0)) _needDisplay = keyModePressed();
    if (keyPressed(Keys, 2, 1)) _needDisplay = keyLeftRightPressed(true);
    if (keyPressed(Keys, 3, 1)) _needDisplay = keyLeftRightPressed(false);
    if (keyPressed(Keys, 4, 1)) _needDisplay = keyDownUpPressed(true);
    if (keyPressed(Keys, 5, 1)) _needDisplay = keyDownUpPressed(false);
  }

  if (_needDisplay) printDisplay();

}

// обработка нажатия клавиш Left,Right
bool keyLeftRightPressed(bool Left) {
  byte _minValue = 0;
  byte _maxValue;

  if (EditingModePart.get_isNull() == 1) return false;

  EditingModePart.LeftRightValue(Left);
  settMode.lastBlinkTime = millis();
  settMode.blinkOff = false;

  return true;

}

// обработка нажатия клавиш Down,Up
bool keyDownUpPressed(bool Down) {
  byte _minValue = 0;
  byte _maxValue;

  if (currSettings.setting != 0) return false;

  if (currMode.main == 0) _maxValue = 4;

  if ((Down) && (currMode.secondary == _maxValue)) return false;
  if ((!Down) && (currMode.secondary == _minValue)) return false;

  if (Down) currMode.secondary++;
  else currMode.secondary--;

  initModeForParts(displayMode[currMode.main][currMode.secondary]);

  return true;
}

// обработка нажатия клавиши Esc
bool keyEscPressed() {

  /*Serial.print("Esc 0: ");
  debugEditingModePart();*/

  if (currSettings.setting != 0) {
    EditingModePart.set_isNull(1);    
    currSettings.setting = 0;
    settMode.blinkOff = false;
  }
  else if (currMode.secondary != 0) {
    currMode.secondary = 0;
    initModeForParts(displayMode[currMode.main][currMode.secondary]);
  }
  else if (currMode.main != 0) {
    currMode.main = 0;
    initModeForParts(displayMode[currMode.main][currMode.secondary]);
  }  
  else return false;

  /*Serial.print("Esc 1: ");
  debugEditingModePart();*/
  
  return true;  
}

// обработка нажатия клавиши Mode
bool keyModePressed() {
  bool _nextSetting = true;
  int _indexSetting = 0;
  bool _findSetting = false;

  /*Serial.print("Mode 0: ");
  debugEditingModePart();*/

  if (EditingModePart.get_isNull() == 0) {
    EditingModePart.writeValue(&currSettings);
    EditingModePart.set_isNull(1);    
  }

  currSettings.setting++;
  
  for (int _i = 0; (_i < 8 && !_findSetting && modeForParts[_i][0] != '\0') ; _i++) {
    EditingModePart.initialize(modeForParts[_i], &currSettings);
    if (EditingModePart.get_edited()) {
      _indexSetting++;
      if (currSettings.setting == _indexSetting) {
        _findSetting  = true;
      }
    }
  }

  if (_findSetting) EditingModePart.readValue(&currSettings);
  else {
    currSettings.setting = 0;
    settMode.blinkOff = false;
    EditingModePart.set_isNull(1);
  }

  /*Serial.print("Mode 1: ");
  debugEditingModePart();*/

  return true;
}

// общая функция изменения времени
void loopTime() {
  static unsigned long _lastLoopTime = 0; // последнее время обработки изменения времени
  static unsigned long _lastTimerTime = 0; // последнее время обработки изменения времени
  bool _needDisplay = false; // есть необходимость обновить дисплей

  if (_lastLoopTime == 0) {
    currSettings.now = Rtc.getTime();
    _lastLoopTime  = millis();
    _needDisplay = true;
  }

  if (currSettings.timerOn && ((millis() - _lastTimerTime) > 1000)) {
    if (_lastTimerTime == 0) currSettings.timerSecond++; // first second compensation
    _lastTimerTime  = millis();
    if (currSettings.timerSecond == 0) {
      if (currSettings.timerMinute == 0) {
        currSettings.timerOn = false;
        _lastTimerTime  = 0;
        if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      }
      currSettings.timerSecond = 59;
      currSettings.timerMinute--;
    }
    else currSettings.timerSecond--;
    if ((currMode.main == 0) && (currMode.secondary == 1)) _needDisplay = true;
  }

  if (currSettings.setting > 0) {
    if ((millis() - settMode.lastBlinkTime) > 500) {
      settMode.lastBlinkTime  = millis();
      settMode.blinkOff = !settMode.blinkOff;
      _needDisplay = true;
    }
  }

  if ((millis() - _lastLoopTime) > 1000) {
    // секунда оттикала
    _lastLoopTime  = millis();
    currSettings.now.second++;
    if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
    if (currSettings.now.second == 60) {
      currSettings.now.second = 0;
      currSettings.now.minute++;
      if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      if (currSettings.now.minute == 60) {
        // синхронизация раз в час
        currSettings.now = Rtc.getTime();
      }
    }
  }

  if (_needDisplay) printDisplay();

}

bool itNumber(char _c) {
  return (_c == '0' || _c == '1' || _c == '2' || _c == '3' || _c == '4' || _c == '5' || _c == '6' || _c == '7' || _c == '8' || _c == '9');
}

void initModeForParts(char _char[30]) {
  int _indexIN = 0;
  int _indexPart = 0;
  int _indexOUT = 0;
  char _c;

  for (int _i = 0; _i < 8; _i++) {
    modeForParts[_i][0] = '\0';
  }

  do {
    _c = _char[_indexIN];
    bool _endDisplayPart = false;
    if (_indexOUT > 1) if (modeForParts[_indexPart][0] == '%' && !itNumber(modeForParts[_indexPart][_indexOUT - 1])) _endDisplayPart = true;
    if ((_c == '\0' || _c == '%' || _endDisplayPart) && _indexOUT != 0) {
      modeForParts[_indexPart][_indexOUT] = '\0';
      _indexPart++;
      _indexOUT = 0;
    }
    if (_c != '\0') {
      modeForParts[_indexPart][_indexOUT] = _c;
      _indexOUT++;
    }
    _indexIN++;
  } while (_c != '\0');

}

// общая функция вывода дисплея
void printDisplay() {
  char _toDisplay[9];
  ModePart _PrintModePart;

  _toDisplay[0] = '\0';

  int _indexSetting = 0;
  for (int _i = 0; _i < 8; _i++) if (modeForParts[_i][0] != '\0') {
      _PrintModePart.initialize(modeForParts[_i], &currSettings);
      bool _findSetting = false;
      if (_PrintModePart.get_edited()) {
        _indexSetting++;
        if (currSettings.setting == _indexSetting) _findSetting = true;
      }
      if (!_findSetting) _PrintModePart.readValue(&currSettings);
      else _PrintModePart.set_value(EditingModePart.get_value());
      _PrintModePart.valueToDisplay(_toDisplay, &currSettings, _findSetting && settMode.blinkOff);

    }

  Module.setDisplayToString(_toDisplay, 0, false);

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
