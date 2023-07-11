/*
  MenuItem.cpp - Программная реализация элементов меню для меню
*/

#include "MenuItem.h"

// реализация объекта родоначальника
MenuItem::MenuItem() {
  currMode.editing = false;
};

MenuItem::~MenuItem() {};

boolean MenuItem::editing() {
  return false;
};

void MenuItem::enterEditing() {
  currMode.editing = currMode.blinkOn = true;
};

void MenuItem::downValue() {};

void MenuItem::upValue() {};

void MenuItem::exitEditing() {
  currMode.editing = false;
};

void MenuItem::saveEditing() {
  currMode.editing = false;
};

void MenuItem::changeBlink() {
  currMode.blinkOn = !currMode.blinkOn;
};

String MenuItem::emptyString(int len) {
  String ans;
  while (len--) ans += ' ';
  return ans;
};

void MenuItem::changeValue(byte& editValue, const int delta, const byte minValue, const byte maxValue, boolean circleEdit = true) {
  int newValue = editValue + delta;
  if (newValue < minValue) editValue = circleEdit ? maxValue : minValue;
  else if (newValue > maxValue) editValue = circleEdit ? minValue : maxValue;
  else editValue = newValue;
};

// текстовый элемент меню - тупо текст выводит
TextItem::TextItem (const char* _s) {
  for (int i = 0; i < sizeof(s); i++) {
    s[i] = _s[i];
    if (_s[i] == '\0') break;
  }
};

String TextItem::display() {
  return String(s);
};

// выводит настройку из currSettings
SettingsValue::SettingsValue (const global::CurrSettings* _currSettings, const typeSettingsValue _valueType) {
  currSettings = _currSettings;
  valueType = _valueType;
};

String SettingsValue::display() {
  return valueType == timerOn ? (currSettings->timer == nullptr ? " " : "t") : (currSettings->nowDay ? "d" : "n");
};

// выводит флаг будильника из EEPROM
String AlarmFlag::display() {
  if (EEPROM.read(EEPROM_ALARM) == 1) return "b";
  else return " ";
};

// выводит текущее время часы или минуты, редактирует и сохраняет его в MicroDS3231
TimeValue::TimeValue (const global::CurrSettings* _currSettings, const byte _valueIndex) {
  currSettings = _currSettings;
  valueIndex = _valueIndex;
};

String TimeValue::display() {
  if (currMode.editing) {
    if (currMode.blinkOn) return global::valToString(editValue, 2);
    else return "  ";
  }
  else return global::valToString(valueIndex ? currSettings->nowMinute : currSettings->nowHour, 2);
};

boolean TimeValue::editing() {
  return true;
};

void TimeValue::enterEditing() {
  currMode.editing = currMode.blinkOn = true;
  editValue = valueIndex ? currSettings->nowMinute : currSettings->nowHour;
};

void TimeValue::downValue() {
  changeValue(editValue, -1, 0, valueIndex ? 59 : 23);
  currMode.blinkOn = true;
};

void TimeValue::upValue() {
  changeValue(editValue, 1, 0, valueIndex ? 59 : 23);
  currMode.blinkOn = true;
};

void TimeValue::saveEditing() {
  currMode.editing = false;
  DateTime rtcNow = globDS3231.getTime();
  switch (valueIndex) {
    case 0:
      currSettings->nowHour = editValue;
      rtcNow.hour = editValue;
      break;
    case 1:
      currSettings->nowMinute = editValue;
      rtcNow.minute = editValue;
      break;
  }
 globDS3231.setTime(rtcNow);
};

// универсально выводит значение из EEPROM, редактирует и сохраняет его в EEPROM
byteEEPROMvalue::byteEEPROMvalue (const int _adressEEPROM, const byte _minValue, const byte _maxValue, const byte _len, const byte _leadingSpaces = 0, const byte _multiplier = 1) {
  adressEEPROM = _adressEEPROM;
  minValue = _minValue;
  maxValue = _maxValue;
  len = _len;
  leadingSpaces = _leadingSpaces;
  multiplier = _multiplier;
};

String byteEEPROMvalue::display() {
  if (currMode.editing) {
    if (currMode.blinkOn) return global::valToString((int)editValue * multiplier, len, leadingSpaces);
    else return emptyString(len);
  }
  else {
    return global::valToString((int)EEPROM.read(adressEEPROM) * multiplier, len, leadingSpaces);
  }
};

boolean byteEEPROMvalue::editing() {
  return true;
};

void byteEEPROMvalue::enterEditing() {
  currMode.editing = currMode.blinkOn = true;
  editValue = EEPROM.read(adressEEPROM);
};

void byteEEPROMvalue::downValue() {
  changeValue(editValue, -1, minValue, maxValue);
  currMode.blinkOn = true;
};

void byteEEPROMvalue::upValue() {
  changeValue(editValue, 1, minValue, maxValue);
  currMode.blinkOn = true;
};

void byteEEPROMvalue::saveEditing() {
  currMode.editing = false;
  EEPROM.update(adressEEPROM, editValue);
};

// наследник от byteEEPROMvalue, дополнительно передает значение в шаговый мотор для его движения
MotorPosition::MotorPosition (const StepMotor* _stepMotor) : byteEEPROMvalue (EEPROM_MOTOR_POSITION, 0, 250, 4, 3) {
  stepMotor = _stepMotor;
};

String MotorPosition::display() {
  if (currMode.editing) {
    if (currMode.blinkOn) return global::valToString(editValue - 125, len, leadingSpaces);
    else return emptyString(len);
  }
  else {
    return global::valToString(EEPROM.read(adressEEPROM) - 125, len, leadingSpaces);
  }
};

void MotorPosition::saveEditing() {
  currMode.editing = false;
  stepMotor->set_positionMove(editValue - 125);
  EEPROM.update(adressEEPROM, editValue);
};

// выводит текущие минуты или секунды таймера, если он тикает,
// иначе выводит и редактирует параметры по умолчанию таймера и сохраняет их в EEPROM
TimerValue::TimerValue (const global::CurrSettings* _currSettings, const byte _valueIndex) {
  currSettings = _currSettings;
  valueIndex = _valueIndex;
};

String TimerValue::display() {
  if (currSettings->timer != nullptr) {
    return global::valToString(valueIndex ? currSettings->timer->getSecond() : currSettings->timer->getMinute(), 2);
  }
  else if (currMode.editing) {
    if (currMode.blinkOn) return global::valToString(editValue, 2);
    else return "  ";
  }
  else {
    return global::valToString(EEPROM.read(valueIndex ? EEPROM_TIMER_SECOND : EEPROM_TIMER_MINUTE), 2);
  }
};

boolean TimerValue::editing() {
  return currSettings->timer == nullptr;
};

void TimerValue::enterEditing() {
  currMode.editing = currMode.blinkOn = true;
  editValue = EEPROM.read(valueIndex ? EEPROM_TIMER_SECOND : EEPROM_TIMER_MINUTE);
};

void TimerValue::downValue() {
  changeValue(editValue, -1, 0, 59);
  currMode.blinkOn = true;
};

void TimerValue::upValue() {
  changeValue(editValue, 1, 0, 59);
  currMode.blinkOn = true;
};

void TimerValue::saveEditing() {
  currMode.editing = false;
  EEPROM.update(valueIndex ? EEPROM_TIMER_SECOND : EEPROM_TIMER_MINUTE, editValue);
};

// выводит и редактирует текущее состояние таймера - если меняем то перезапускаем или запускаем таймер
TimerStart::TimerStart (const global::CurrSettings* _currSettings) {
  currSettings = _currSettings;
};

String TimerStart::display() {
  if (currMode.editing) {
    if (currMode.blinkOn) return global::valToString(editValue, 2, 1);
    else return "  ";
  }
  else {
    return currSettings->timer != nullptr ? " 1" : " 0";
  }
};

boolean TimerStart::editing() {
  return true;
};

void TimerStart::enterEditing() {
  currMode.editing = currMode.blinkOn = true;
  editValue = currSettings->timer != nullptr;
};

void TimerStart::downValue() {
  changeValue(editValue, -1, 0, 1);
  currMode.blinkOn = true;
};

void TimerStart::upValue() {
  changeValue(editValue, 1, 0, 1);
  currMode.blinkOn = true;
};

void TimerStart::saveEditing() {
  currMode.editing = false;
  if (editValue) {
    if (currSettings->timer == nullptr) currSettings->timer = new Timer(EEPROM.read(EEPROM_TIMER_MINUTE), EEPROM.read(EEPROM_TIMER_SECOND));
    else currSettings->timer->restart(EEPROM.read(EEPROM_TIMER_MINUTE), EEPROM.read(EEPROM_TIMER_SECOND));
  }
  else if (currSettings->timer != nullptr) {
    delete currSettings->timer;
    currSettings->timer = nullptr;
  }
};

// выводит текущую температуру модуля MicroDS3231
String RtsTemp::display() {
  int _intValue = globDS3231.getTemperature() * 10;
  return global::valToString(_intValue, 3);
};

// выводит текущую температуру ворды аквариума через объект ControlTemp
String AquaTemp::display() {
  if (!globControlTemp.getAquaTempConnected()) return "Err";
  else {
    int _intValue = globControlTemp.getAquaTemp() * 10;
    return global::valToString(_intValue, 3);
  }
};

// выводит логи температуру воды аквариума через объект ControlTemp
// в режиме редактирования - просматриваем историю логов
TempLog::TempLog (const global::CurrSettings* _currSettings) {
  currSettings = _currSettings;
};

String TempLog::display() {
  if (currMode.editing) {
    if (currMode.blinkOn) return logToString(editValue);
    else return "        ";
  }
  else {
    return logToString(23);
  }
};

boolean TempLog::editing() {
  return true;
};

void TempLog::enterEditing() {
  currMode.editing = currMode.blinkOn = true;
  editValue = 23;
};

void TempLog::downValue() {
  changeValue(editValue, -1, 0, 23, false);
  currMode.blinkOn = true;
};

void TempLog::upValue() {
  changeValue(editValue, 1, 0, 23, false);
  currMode.blinkOn = true;
};

String TempLog::logToString(byte _index) {
  byte _indexOfNow = currSettings->nowHour;
  byte _indexOfLog;
  if ((23 - _index) > _indexOfNow) _indexOfLog = _indexOfNow + _index + 1;
  else _indexOfLog = _indexOfNow + _index - 23;
  String ans = global::valToString(_indexOfLog, 2);
  ans += "00";
  word _valueOfLog = globControlTemp.getHeaterTempLog(_indexOfLog);
  if (_valueOfLog > 10000) {
    ans += "o";
    _valueOfLog -= 10000;
  }
  else ans += " ";
  if (_valueOfLog == 0) ans += "Err";
  else {
    _valueOfLog -= 1000;
    ans += global::valToString(_valueOfLog, 3);
  }
  return ans;
};

// выводит заданный показатель счетчика CO2 через объект BubbleCounter
bubbleCounterValue::bubbleCounterValue (const BubbleCounter* _bubbleCounter, const typeBubbleCounterValue _valueType) {
  bubbleCounter = _bubbleCounter;
  valueType = _valueType;
};

String bubbleCounterValue::display() {
  int _intValue = 0;
  switch (valueType) {
    case bubbleIn100Second: _intValue = bubbleCounter->getBubbleIn100Second(); break;
    case minLevel: _intValue = bubbleCounter->getMinLevel(); break;
    case maxLevel: _intValue = bubbleCounter->getMaxLevel(); break;
  }
  switch (_intValue) {
    case -1: return "Err1"; break;
    case -2: return "Err2"; break;
    case -3: return global::valToString(0, 4, 1); break;
    default:
      switch (valueType) {
        case bubbleIn100Second: return global::valToString(_intValue, 4, 1); break;
        default: return global::valToString(_intValue, 4, 3); break;
      }
      break;
  }
};

// выводит заданный показатель управления расходом CO2 через объект BubbleControl
bubbleControlValue::bubbleControlValue (const BubbleControl* _bubbleControl, const typeBubbleControlValue _valueType) {
  bubbleControl = _bubbleControl;
  valueType = _valueType;
};

String bubbleControlValue::display() {
  switch (valueType) {
    case controlCondition: return bubbleControl->get_condition(); break;
    case minBubblesIn100Second: return global::valToString(bubbleControl->get_minBubblesIn100Second(), 4, 1); break;
    case maxBubblesIn100Second: return global::valToString(bubbleControl->get_maxBubblesIn100Second(), 4, 1); break;
  }
};

// выводит редактирует счетчик кормления через объект Feeding
FeedingValue::FeedingValue (const Feeding* _feeding) {
  feeding = _feeding;
};

String FeedingValue::display() {
  if (currMode.editing) {
    if (currMode.blinkOn) return global::valToString(editValue, 4, 3);
    else return "  ";
  }
  else {
    return global::valToString(feeding->getFeedingLoop(), 4, 3);
  }
};

boolean FeedingValue::editing() {
  return true;
};

void FeedingValue::enterEditing() {
  currMode.editing = currMode.blinkOn = true;
  editValue = feeding->getFeedingLoop();
};

void FeedingValue::downValue() {
  changeValue(editValue, -1, 0, 10);
  currMode.blinkOn = true;
};

void FeedingValue::upValue() {
  changeValue(editValue, 1, 0, 10);
  currMode.blinkOn = true;
};

void FeedingValue::saveEditing() {
  currMode.editing = false;
  feeding->setFeedingLoop(editValue);
};
