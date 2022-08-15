#include <TM1638.h>
#include <EEPROM.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 Rtc;

#define KEYBOARD_INTERVAL 10
#define PIEZO_PIN 3
#define TURN_OFF_MANUAL_LAMP 120000 // at least on change minute
#define DS18B20_PIN 7
#define TEMP_RENEW_INTERVAL 200
#define HEATER_PIN A0

#include <OneWire.h>
#include <DS18B20.h>

OneWire oneWire(DS18B20_PIN);
DS18B20 sensor(&oneWire);

struct CurrSettings {
  DateTime now;
  byte alarmOn;
  bool nowDay;
  bool timerOn;
  byte timerMinute;
  byte timerSecond;
  byte setting;
  unsigned long alarmStartSound;
  byte manualLamp;
  float aquaTemp;
  bool aquaTempErr;
  bool heaterOn = false;
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

// frequency, duration
#define NUMBER_OF_NOTES 10
unsigned int alarmMelody[14][2] = {{2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 3000}};

byte lampPinsLevel[][2] = {{A1, 0}, {A2, 0}, {A3, 0}}; // Pin from left to right, Level

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
*/
char *displayMode[][7] = {
  {"%1C %H%M%2C%3C", "St%7n%8a %t", "Sd%0h%1m  ", "Sn%2h%3m  ", "Sb%4h%5m %6c", "Sdn %9m %10c", ""},
  {"i%1Qo%2Q", "Td%11q  %12c", "Tn%13q  %14c", "dt %15w   "},
  {""}
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
  tone(PIEZO_PIN, 2500, 100);
  for (int i = 0; i < 3; i++) {
    pinMode(lampPinsLevel[i][0], OUTPUT);
  }

  pinMode(HEATER_PIN, OUTPUT);  
  digitalWrite(HEATER_PIN, HIGH); // false - off

  currSettings.aquaTempErr = !sensor.begin();
  if (!currSettings.aquaTempErr) sensor.setResolution(12);

  initModeForParts(displayMode[currMode.main][currMode.secondary]);
  EditingModePart.set_isNull(1);

  /*Serial.begin(9600);
  Serial.println("debugging");*/
}

/*
  void debugTemp() {
  Serial.print("aquaTemp: ");
  Serial.print(currSettings.aquaTemp);
  Serial.print(",aquaTempErr: ");
  Serial.println(currSettings.aquaTempErr);
  }
*/

/*
  void debugEditingModePart() {
  Serial.print(EditingModePart.get_typeOfPart());
  Serial.print(",isNull: ");
  Serial.print(EditingModePart.get_isNull());
  Serial.print(",value: ");
  Serial.println(EditingModePart.get_value());
  }
*/

/*
  void debugManualLamp() {
  Serial.print("manualLamp: ");
  Serial.print(currSettings.manualLamp);
  Serial.print(",nowDay: ");
  Serial.println(currSettings.nowDay);
  }
*/

/*void debugOnOffLamps(char _char[10], int _i, int _nowInMinutes, int _minutesLamp) {
  Serial.print(_char);
  Serial.print(" _i: ");
  Serial.print(_i);
  Serial.print(" _nowInMinutes: ");
  Serial.print(_nowInMinutes);
  Serial.print(" _minutesLamp: ");
  Serial.println(_minutesLamp);
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
      if (keyPressed(Keys, 7, 0)) _needDisplay = keyLampsPressed();
    }    
  }

  if (_needDisplay) printDisplay();

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

  if (EditingModePart.get_isNull() == 0) {
    EditingModePart.LeftRightValue(_left);
    settMode.lastBlinkTime = millis();
    settMode.blinkOff = false;
    return true;
  }

  if (currMode.secondary != 0) return false;
  if ((!_left) && (displayMode[currMode.main + 1][currMode.secondary][0] == '\0')) return false;
  if ((_left) && (currMode.main == 0)) return false;

  if (_left) currMode.main--;
  else currMode.main++;

  initModeForParts(displayMode[currMode.main][currMode.secondary]);

  return true;

}

// обработка нажатия клавиш Down,Up
bool keyDownUpPressed(bool _down) {

  if (currSettings.setting != 0) return false;

  if ((_down) && (displayMode[currMode.main][currMode.secondary + 1][0] == '\0')) return false;
  if ((!_down) && (currMode.secondary == 0)) return false;

  if (_down) currMode.secondary++;
  else currMode.secondary--;

  initModeForParts(displayMode[currMode.main][currMode.secondary]);

  return true;
}

// обработка нажатия клавиши Esc
bool keyEscPressed() {

  /*Serial.print("Esc 0: ");
    debugEditingModePart();*/

  if (currSettings.alarmStartSound != 0) {
    currSettings.alarmStartSound = 0;
    return false;
  }

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

// all options control
void conditionControl() {
  static unsigned long _manualLampTime = 0;

  // it's day or nigth
  int _morningInMinutes = (int)EEPROM.read(0) * 60 + EEPROM.read(1);
  int _eveningInMinutes = (int)EEPROM.read(2) * 60 + EEPROM.read(3);
  int _nowInMinutes = (int)currSettings.now.hour * 60 + currSettings.now.minute;
  if (_eveningInMinutes > _morningInMinutes) currSettings.nowDay = (_nowInMinutes >= _morningInMinutes && _nowInMinutes < _eveningInMinutes);
  else currSettings.nowDay = (_nowInMinutes >= _morningInMinutes || _nowInMinutes < _eveningInMinutes);

  // starting alarm
  if (currSettings.alarmStartSound == 0) {
    if (EEPROM.read(6) == 1) {
      int _alarmInMinutes = (int)EEPROM.read(4) * 60 + EEPROM.read(5);
      if (_nowInMinutes == _alarmInMinutes) currSettings.alarmStartSound = millis();
    }
  }

  // control off heater
  byte needingTemp;
  bool heaterAlwaysOff = false;
  if (currSettings.nowDay) {
    needingTemp = EEPROM.read(11);
    if (EEPROM.read(12) == 0) heaterAlwaysOff = true;
  }
  else {
    needingTemp = EEPROM.read(13);
    if (EEPROM.read(14) == 0) heaterAlwaysOff = true;
  }
  byte deltaTemp = EEPROM.read(15);
  float minTemp = needingTemp - (float)deltaTemp / 10;
  float maxTemp = needingTemp + (float)deltaTemp / 10;
  if (currSettings.heaterOn && (heaterAlwaysOff || currSettings.aquaTempErr || currSettings.aquaTemp >= maxTemp)) {
    currSettings.heaterOn = false;
    digitalWrite(HEATER_PIN, HIGH);
  }
  else if (!currSettings.heaterOn && !heaterAlwaysOff && currSettings.aquaTemp <= minTemp) {
    currSettings.heaterOn = true;
    digitalWrite(HEATER_PIN, LOW);
  }

  // turn off manual lamp
  if (currSettings.manualLamp > 0) {
    if (_manualLampTime == 0) _manualLampTime = millis();
    if ((millis() - _manualLampTime) >= TURN_OFF_MANUAL_LAMP) currSettings.manualLamp = 0;
  }

  // clear static values for manual lamp
  if (_manualLampTime > 0 && currSettings.manualLamp == 0) {
    _manualLampTime = 0;    
    tone(PIEZO_PIN, 2500, 100);
  }

  // turn on\off flags of lamps
  if (currSettings.manualLamp == 0) {
    if (!currSettings.nowDay) {
      for (int i = 0; i < 3; i++) lampPinsLevel[i][1] = 0;
    }
    else if (EEPROM.read(10) == 0) {
      for (int i = 0; i < 3; i++) lampPinsLevel[i][1] = 1;
    }
    else {
      int _minutesBetweenLamps = EEPROM.read(9);
      for (int i = 0; i < 3; i++) {
        // after morning
        int _minutesLamp = _morningInMinutes + _minutesBetweenLamps * i;
        lampPinsLevel[i][1] = 0;
        if (_nowInMinutes >= _morningInMinutes) {
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 1;
        }
        else if (_minutesLamp >= 1440) {
          _minutesLamp = _minutesLamp - 1440;
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 1;
        }
        else lampPinsLevel[i][1] = 1;
        // before evening
        _minutesLamp = _eveningInMinutes + _minutesBetweenLamps * (i - 2);
        if (_nowInMinutes < _eveningInMinutes) {
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 0;
        }
        else if (_minutesLamp < 0) {
          _minutesLamp = _minutesLamp + 1440;
          if (_nowInMinutes >= _minutesLamp) lampPinsLevel[i][1] = 0;
        }
      }
    }
  }

  // turn on\off lamps
  for (int i = 0; i < 3; i++) {
    if (lampPinsLevel[i][1] == 1) digitalWrite(lampPinsLevel[i][0], LOW);
    else digitalWrite(lampPinsLevel[i][0], HIGH);
  }

}

// main procedure for control time
void loopTime() {
  static unsigned long _lastLoopTime = 0;
  static unsigned long _lastTempTime = 0;
  static bool _waitingTemp = false;
  static unsigned long _intWaitingTemp = 0;
  static unsigned long _lastTimerTime = 0;
  static unsigned long _nextNoteTime = 0;
  static byte _iNote = 0;
  bool _needDisplay = false;

  // Loop once First time
  if (_lastLoopTime == 0) {
    currSettings.now = Rtc.getTime();    
    conditionControl();
    _lastLoopTime  = millis();
    _needDisplay = true;
  }

  // Alarm sound loop
  if (currSettings.alarmStartSound != 0) {
    if ((millis() - currSettings.alarmStartSound) >= 60000) {
      currSettings.alarmStartSound = 0;
      _nextNoteTime = 0;      
      noTone(PIEZO_PIN);
    }
    else if (millis() > _nextNoteTime) {
      if (alarmMelody[_iNote][0] == 0) noTone(PIEZO_PIN);
      else tone(PIEZO_PIN, alarmMelody[_iNote][0]);      
      _nextNoteTime = millis() + alarmMelody[_iNote][1];
      if (++_iNote == NUMBER_OF_NOTES) _iNote = 0;
    }
  }
  else if (_nextNoteTime != 0) {
    _nextNoteTime = 0;    
    noTone(PIEZO_PIN);
  }

  // request temperature
  if (!currSettings.aquaTempErr && !_waitingTemp && (millis() - _lastTempTime) > TEMP_RENEW_INTERVAL) {
    _lastTempTime = millis();
    sensor.requestTemperatures();
    _intWaitingTemp = 0;
    _waitingTemp = true;
  }

  // Renew temperature
  if (_waitingTemp && (millis() - _lastTempTime) > 10) {
    _lastTempTime = millis();
    if (sensor.isConversionComplete()) {
      currSettings.aquaTempErr = false;
      currSettings.aquaTemp = sensor.getTempC();
      _waitingTemp = false;
      if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
    }
    else if (_intWaitingTemp++ > 10) {
      currSettings.aquaTempErr = true;
      if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
    }
  }

  // Loop decrement timer
  if (currSettings.timerOn && ((millis() - _lastTimerTime) > 1000)) {
    if (_lastTimerTime == 0) currSettings.timerSecond++; // first second compensation
    _lastTimerTime  = millis();
    if (currSettings.timerSecond == 0) {
      if (currSettings.timerMinute == 0) {
        currSettings.timerOn = false;
        currSettings.alarmStartSound = millis();
        _lastTimerTime  = 0;
        if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      }
      currSettings.timerSecond = 59;
      currSettings.timerMinute--;
    }
    else currSettings.timerSecond--;
    if ((currMode.main == 0) && (currMode.secondary == 1)) _needDisplay = true;
  }

  // Loop blink Settings
  if (currSettings.setting > 0) {
    if ((millis() - settMode.lastBlinkTime) > 500) {
      settMode.lastBlinkTime  = millis();
      settMode.blinkOff = !settMode.blinkOff;
      _needDisplay = true;
    }
  }

  // Loop increment local time
  if ((millis() - _lastLoopTime) > 1000) {
    // секунда оттикала
    _lastLoopTime  = millis();
    currSettings.now.second++;
    if (currSettings.now.second == 60) {
      currSettings.now.second = 0;
      currSettings.now.minute++;
      if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      if (currSettings.now.minute == 60) {
        // синхронизация раз в час
        currSettings.now = Rtc.getTime();
      }
      conditionControl();
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
