#include <TM1638.h>
#include <EEPROM.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 Rtc;

#define DISPLAY_INTERVAL 0
#define KEYBOARD_INTERVAL 10
#define PIEZO_PIN 8

/* Описатели шаблона:
  % - начало шаблона
  С - выводимая символьная вличина (не редактируется) номер величины между % и знаком
  H - текущее кол-во часов (редактируется)
  M - текущее кол-во минут (редактируется)
  h - кол-во часов из EEPROM (редактируется) адрес памяти между % и знаком
  m - кол-во минут из EEPROM (редактируется) адрес памяти между % и знаком
  c - выводимая символьная вличина из EEPROM (редактируется) номер величины и адрес памяти между % и знаком
*/

char *displayMode[][4] = {
  {"%1C %H%M%2C%3C", "Sd%0h%1m  ", "Sn%2h%3m  ", "Sb%4h%5m %6c"}
};

struct DisplayPart {
  byte edited;
  byte value;
  byte length;
  byte maxValue;
  byte minValue;
  char charValue;
};

struct TimeHour {
  byte hour;
  byte minute;
};
struct TimeMinute {
  byte minute;
  byte second;
};

struct {
  byte main;
  byte secondary;
} currMode;

struct {
  byte setting;
  byte value;
  bool blinkOff;
} settMode;

struct {
  DateTime now;
  TimeHour morning;
  TimeHour evening;
  TimeHour alarmSet;
  byte alarmOn;
  TimeMinute timerSet;
  bool nowMorning;
  bool timerOn;
} timeSettings;

void setup() {

  pinMode(PIEZO_PIN, OUTPUT); // настраиваем вывод 2 на выход
  tone(PIEZO_PIN, 200);
  delay(500);
  noTone(PIEZO_PIN);
  timeSettings.morning.hour = readEEPROM(0);
  timeSettings.morning.minute = readEEPROM(1);
  timeSettings.evening.hour = readEEPROM(2);
  timeSettings.evening.minute = readEEPROM(3);
  timeSettings.alarmSet.hour = readEEPROM(4);
  timeSettings.alarmSet.minute = readEEPROM(5);
  timeSettings.alarmOn = readEEPROM(6);
  timeSettings.timerSet.minute = readEEPROM(7);
  timeSettings.timerSet.second = readEEPROM(8);
  /*Serial.begin(9600);           //  setup serial
  Serial.println(currMode.main); // отладка*/

}

byte readEEPROM(int _adress) {
  byte _value = EEPROM.read(_adress);
  if (_value == 255) return 0;
  else return _value;
}

void loop() {

  loopTime();
  readKeyboard();

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

  switch (settMode.setting) {
    case 1:
      _maxValue = 23;
      break;
    case 2:
      _maxValue = 59;
      break;
    case 3:
      _maxValue = 1;
      break;
    default:
      return false;
      break;
  }

  if ((Left) && (settMode.value == _minValue)) settMode.value = _maxValue;
  else if ((!Left) && (settMode.value == _maxValue)) settMode.value = _minValue;
  else if (Left) settMode.value--;
  else settMode.value++;

  return true;
}

// обработка нажатия клавиш Down,Up
bool keyDownUpPressed(bool Down) {
  byte _minValue = 0;
  byte _maxValue;

  if (settMode.setting != 0) return false;

  if (currMode.main == 0) _maxValue = 3;

  if ((Down) && (currMode.secondary == _maxValue)) return false;
  if ((!Down) && (currMode.secondary == _minValue)) return false;

  if (Down) currMode.secondary++;
  else currMode.secondary--;

  return true;
}

// обработка нажатия клавиши Esc
bool keyEscPressed() {

  if (settMode.setting != 0) {
    settMode.setting = 0;
    settMode.blinkOff = false;
  }
  else if (currMode.secondary != 0) currMode.secondary = 0;
  else if (currMode.main != 0) currMode.main = 0;
  else return false;
  return true;
}

// обработка нажатия клавиши Mode
bool keyModePressed() {
  bool _nextSetting = true;

  switch (currMode.main) {
    case 0:
      // текущее время
      switch (settMode.setting) {
        case 0:
          // настройка часов
          switch (currMode.secondary) {
            case 0:
              settMode.value = timeSettings.now.hour;
              break;
            case 1:
              settMode.value = timeSettings.morning.hour;
              break;
            case 2:
              settMode.value = timeSettings.evening.hour;
              break;
            case 3:
              settMode.value = timeSettings.alarmSet.hour;
              break;
            default:
              return false;
              break;
          }
          break;
        case 1:
          // завершение часов - настройка минут
          switch (currMode.secondary) {
            case 0:
              timeSettings.now.hour = settMode.value;
              Rtc.setTime(timeSettings.now);
              settMode.value = timeSettings.now.minute;
              break;
            case 1:
              EEPROM.update(0, settMode.value);
              timeSettings.morning.hour = settMode.value;
              settMode.value = timeSettings.morning.minute;
              break;
            case 2:
              EEPROM.update(2, settMode.value);
              timeSettings.evening.hour = settMode.value;
              settMode.value = timeSettings.evening.minute;
              break;
            case 3:
              EEPROM.update(4, settMode.value);
              timeSettings.alarmSet.hour = settMode.value;
              settMode.value = timeSettings.alarmSet.minute;
              break;
            default:
              return false;
              break;
          }
          break;
        case 2:
          // завершение минут (включение если есть)
          switch (currMode.secondary) {
            case 0:
              timeSettings.now.minute = settMode.value;
              Rtc.setTime(timeSettings.now);
              _nextSetting = false;
              break;
            case 1:
              EEPROM.update(1, settMode.value);
              timeSettings.morning.minute = settMode.value;
              _nextSetting = false;
              break;
            case 2:
              EEPROM.update(3, settMode.value);
              timeSettings.evening.minute = settMode.value;
              _nextSetting = false;
              break;
            case 3:
              EEPROM.update(5, settMode.value);
              timeSettings.alarmSet.minute = settMode.value;
              settMode.value = timeSettings.alarmOn;
              break;
            default:
              return false;
              break;
          }
          break;
        case 3:
          // завершение включение
          switch (currMode.secondary) {
            case 3:
              EEPROM.update(6, settMode.value);
              timeSettings.alarmOn = settMode.value;
              break;
            default:
              return false;
              break;
          }
          _nextSetting = false;
          break;
        default:
          return false;
          break;
      }
      break;
    default:
      return false;
      break;
  }

  if (_nextSetting) settMode.setting++;
  else settMode.setting = 0;
  settMode.blinkOff = false;

  return true;
}

// общая функция изменения времени
void loopTime() {
  static unsigned long _lastLoopTime = 0; // последнее время обработки изменения времени
  static unsigned long _lastBlinkTime = 0; // последнее время мигания раз в 0.5 сек
  bool _needDisplay = false; // есть необходимость обновить дисплей

  if (_lastLoopTime == 0) {
    timeSettings.now = Rtc.getTime();
    _lastLoopTime  = millis();
    _needDisplay = true;
  }

  if (settMode.setting > 0) {
    if ((millis() - _lastBlinkTime) > 50) {
      _lastBlinkTime  = millis();
      settMode.blinkOff = !settMode.blinkOff;
      _needDisplay = true;
    }
  }

  if ((millis() - _lastLoopTime) > 1000) {
    // секунда оттикала
    _lastLoopTime  = millis();
    timeSettings.now.second++;
    if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
    if (timeSettings.now.second == 60) {
      timeSettings.now.second = 0;
      timeSettings.now.minute++;
      if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      if (timeSettings.now.minute == 60) {
        // синхронизация раз в час
        timeSettings.now = Rtc.getTime();
      }
    }
  }

  if (_needDisplay) printDisplay();

}

DisplayPart workDisplayPart(int toDo, char _c, char _charAdress[5]) {
  DisplayPart _return;

  // отладка
  /*Serial.print("IN:");
  Serial.print(_c);
  Serial.print("-");
  Serial.println(_charAdress);*/
  
  int _adress = atoi(_charAdress);
  _return.minValue = 0;
  _return.edited = false;
  _return.length = 0;
  _return.charValue = ' ';
  if (_c == 'M' || _c == 'm' || _c == 'S' || _c == 's') {
    _return.maxValue = 59;
    _return.edited = true;
    _return.length = 2;
    if (_c == 'M') _return.value = timeSettings.now.minute;
    else if (_c == 'S') _return.value = timeSettings.now.second;
    else _return.value = readEEPROM(_adress);
  }
  else if (_c == 'H' || _c == 'h') {
    _return.maxValue = 23;
    _return.edited = true;
    _return.length = 2;
    if (_c == 'H') _return.value = timeSettings.now.hour;
    else _return.value = readEEPROM(_adress);
  }
  else if (_c == 'c') {
    _return.maxValue = 1;
    _return.edited = true;
    _return.length = 1;
    _return.value = readEEPROM(_adress);
  }
  else if (_c == 'C') {
    if (_adress == 1) {
      if (timeSettings.nowMorning) _return.charValue = 'd';
      else _return.charValue = 'n';
    }
    else if (_adress == 2) {
      if (timeSettings.timerOn) _return.charValue = 't';
      else _return.charValue = ' ';
    }
    else if (_adress == 3) {
      if (readEEPROM(6) == 1) _return.charValue = 'b';
      else _return.charValue = ' ';
    }
    else _return.charValue = 'E';
  }
  else _return.charValue = 'E';

  // отладка
  /*Serial.print("OUT:");
  Serial.print(_return.value);
  Serial.print("-");
  Serial.println(_return.charValue);*/

  return _return;
}

bool itNumber(char _c) {
  return (_c == '0' || _c == '1' || _c == '2' || _c == '3' || _c == '4' || _c == '5' || _c == '6' || _c == '7' || _c == '8' || _c == '9');
}

// общая функция вывода дисплея
void printDisplay() {
  bool _itData;
  char _c;
  char _toDisplay[9];
  char _charAdress[5];
  char _strValue[2];
  byte _value;
  int _i0 = 0;
  int _i1 = 0;
  int _i2 = 0;
  int _i3 = 0;
  int _adress;
  DisplayPart _displayPart;

  _itData = false;
  //Serial.println(displayMode[currMode.main][currMode.secondary]); // отладка
  _c = displayMode[currMode.main][currMode.secondary][_i0];
  while (_c != '\0') {
    _i0++;
    if (_itData) {
      if (itNumber(_c)) {
        _charAdress[_i2] = _c;
        _i2++;
      }
      else {
        _itData = false;
        _charAdress[_i2] = '\0';
        _displayPart = workDisplayPart(0, _c, _charAdress);
        if (_displayPart.edited) {
          _i3++;
          if (_i3 == settMode.setting) _value = settMode.value;
          else _value = _displayPart.value;
          if (_i3 == settMode.setting && settMode.blinkOff) {
            for (int i = 0; i < _displayPart.length; i++) {
              _toDisplay[_i1] = ' ';
              _i1++;
            }
          }
          else {
            if (_displayPart.length == 1) {
              sprintf(_strValue, "%1d", _value);
              _toDisplay[_i1] = _strValue[0];
              _i1++;
            }
            else if (_displayPart.length == 2) {
              sprintf(_strValue, "%02d", _value);
              _toDisplay[_i1] = _strValue[0];
              _i1++;
              _toDisplay[_i1] = _strValue[1];
              _i1++;
            }
          }
        }
        else {
          _toDisplay[_i1] = _displayPart.charValue;
          _i1++;
        }
      }
    }
    else if (_c == '%') {
      _itData = true;
      _i2 = 0;
    }
    else {
      _toDisplay[_i1] = _c;
      _i1++;
    }
    _c = displayMode[currMode.main][currMode.secondary][_i0];
  }
  _toDisplay[_i1] = '\0';
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
