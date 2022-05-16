#include <TM1638.h>
#include <EEPROM.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 Rtc;

#define DISPLAY_INTERVAL 0
#define KEYBOARD_INTERVAL 10
#define PIEZO_PIN 8

struct {
  byte main;
  byte secondary;
} currMode;

struct {
  byte setting;
  byte value;
  bool blinkOff;
} settMode;

struct TimeHour {
  byte hour;
  byte minute;
};
struct TimeMinute {
  byte minute;
  byte second;
};

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
  Serial.begin(9600);           //  setup serial
  Serial.println(currMode.main); // отладка


}

byte readEEPROM(byte _adress) {
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

  Serial.println(settMode.value); // отладка

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

// общая функция вывода дисплея
void printDisplay() {

  switch (currMode.main) {
    case 0:
      printDisplay0_Time();
      break;
    case 1:
      break;
    default:
      break;
  }
}

// функция вывода дисплея в режиме 0: вывод времени
void printDisplay0_Time() {

  char _s[8];
  String _s0;
  String _s1;
  byte b1;
  byte b2;
  byte b3;

  switch (currMode.secondary) {
    case 0:
      if (timeSettings.nowMorning) _s0 = String("d "); else _s0 = String("n ");
      b1 = timeSettings.now.hour;
      b2 = timeSettings.now.minute;
      if (timeSettings.timerOn) _s1 = String("t"); else _s1 = String(" ");
      if (timeSettings.alarmOn == 1) _s1.concat("b"); else _s1.concat(" ");
      break;
    case 1:
      _s0 = String("Sd");
      b1 = timeSettings.morning.hour;
      b2 = timeSettings.morning.minute;
      _s1 = String("  ");
      break;
    case 2:
      _s0 = String("Sn");
      b1 = timeSettings.evening.hour;
      b2 = timeSettings.evening.minute;
      _s1 = String("  ");
      break;
    case 3:
      _s0 = String("Sb");
      b1 = timeSettings.alarmSet.hour;
      b2 = timeSettings.alarmSet.minute;
      if (settMode.setting == 3) {
        if (settMode.value == 1) _s1 = String(" 1"); else _s1 = String(" 0");
      }
      else {
        if (timeSettings.alarmOn == 1) _s1 = String(" 1"); else _s1 = String(" 0");
      }
      break;
  }

  if (settMode.setting == 1) b1 = settMode.value;
  if (settMode.setting == 2) b2 = settMode.value;  

  if ((settMode.blinkOff) && (settMode.setting == 1)) _s0.concat("  ");
  else {
    char _sTemp[2];
    sprintf(_sTemp, "%02d", b1 % 99);
    _s0.concat(_sTemp);
  }

  if ((settMode.blinkOff) && (settMode.setting == 2)) _s0.concat("  ");
  else {
    char _sTemp[2];
    sprintf(_sTemp, "%02d", b2 % 99);
    _s0.concat(_sTemp);
  }

  if ((settMode.blinkOff) && (settMode.setting == 3)) _s0.concat("  ");
  else _s0.concat(_s1);

  char _sTemp[9];
  _s0.toCharArray(_sTemp, 9);

  Module.setDisplayToString(_sTemp, 0, false);
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
