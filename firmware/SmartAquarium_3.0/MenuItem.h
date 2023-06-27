/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

enum typeBubbleCounterValue
{
  bubbleIn100Second, minLevel, maxLevel
};

enum typeBubbleControlValue
{
  controlCondition, minBubblesIn100Second, maxBubblesIn100Second
};

enum typeSettingsValue
{
  dayNight, timerOn
};

class MenuItem {

  public:
    MenuItem() {
      currMode.editing = false;
    };
    virtual ~MenuItem() {};
    virtual String display() = 0;
    virtual boolean editing() {
      return false;
    };
    virtual void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
    };
    virtual void downValue() {};
    virtual void upValue() {};
    virtual void exitEditing() {
      currMode.editing = false;
    };
    virtual void saveEditing() {
      currMode.editing = false;
    };
    void changeBlink() {
      currMode.blinkOn = !currMode.blinkOn;
    }

  protected:
    struct CurrMode {
      boolean editing: 1;
      boolean blinkOn: 1;
    } currMode;

    String emptyString(int len) {
      String ans;
      while (len--) ans += ' ';
      return ans;
    }

    void changeValue(byte& editValue, const int delta, const byte minValue, const byte maxValue, boolean circleEdit = true) {
      int newValue = editValue + delta;
      if (newValue < minValue) editValue = circleEdit ? maxValue : minValue;
      else if (newValue > maxValue) editValue = circleEdit ? minValue : maxValue;
      else editValue = newValue;
    }

};

class TextItem: public MenuItem {

  public:
    TextItem (const String _s) {
      s = _s;
    };
    String display() {
      return s;
    };

  private:
    String s;

};

class SettingsValue: public MenuItem {

  public:
    SettingsValue (const CurrSettings* _currSettings, const typeSettingsValue _valueType) {
      currSettings = _currSettings;
      valueType = _valueType;
    };
    String display() {
      return valueType == timerOn ? (currSettings->timer == nullptr ? " " : "t") : (currSettings->nowDay ? "d" : "n");
    };
  private:
    typeSettingsValue valueType;
    CurrSettings* currSettings;

};

class AlarmFlag: public MenuItem {

  public:
    String display() {
      if (EEPROM.read(EEPROM_ALARM) == 1) return "b";
      else return " ";
    };

};

class TimeValue: public MenuItem {

  public:
    TimeValue (const CurrSettings* _currSettings, const byte _valueIndex, const MicroDS3231* _rtc) {
      currSettings = _currSettings;
      valueIndex = _valueIndex;
      rtc = _rtc;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 2);
        else return "  ";
      }
      else return valToString(valueIndex ? currSettings->nowMinute : currSettings->nowHour, 2);
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = valueIndex ? currSettings->nowMinute : currSettings->nowHour;
    };
    void downValue() {
      changeValue(editValue, -1, 0, valueIndex ? 59 : 23);
      currMode.blinkOn = true;
    };
    void upValue() {
      changeValue(editValue, 1, 0, valueIndex ? 59 : 23);
      currMode.blinkOn = true;
    };
    void saveEditing() {
      currMode.editing = false;
      DateTime rtcNow = rtc->getTime();
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
      rtc->setTime(rtcNow);
    };
  private:
    byte editValue;
    byte valueIndex;
    CurrSettings* currSettings;
    MicroDS3231* rtc;

};

class byteEEPROMvalue: public MenuItem {

  public:
    byteEEPROMvalue (const int _adressEEPROM, const byte _minValue, const byte _maxValue, const byte _len, const byte _leadingSpaces = 0, const byte _multiplier = 1) {
      adressEEPROM = _adressEEPROM;
      minValue = _minValue;
      maxValue = _maxValue;
      len = _len;
      leadingSpaces = _leadingSpaces;
      multiplier = _multiplier;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString((int)editValue * multiplier, len, leadingSpaces);
        else return emptyString(len);
      }
      else {
        return valToString((int)EEPROM.read(adressEEPROM) * multiplier, len, leadingSpaces);
      }
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = EEPROM.read(adressEEPROM);
    };
    void downValue() {
      changeValue(editValue, -1, minValue, maxValue);
      currMode.blinkOn = true;
    };
    void upValue() {
      changeValue(editValue, 1, minValue, maxValue);
      currMode.blinkOn = true;
    };
    void saveEditing() {
      currMode.editing = false;
      EEPROM.update(adressEEPROM, editValue);
    };
  private:
    byte minValue, maxValue;
  protected:
    int adressEEPROM;
    byte editValue;
    byte len, leadingSpaces;
    byte multiplier;
};

class MotorPosition: public byteEEPROMvalue {

  public:
    MotorPosition (const StepMotor* _stepMotor) : byteEEPROMvalue (EEPROM_MOTOR_POSITION, 0, 250, 4, 3) {
      stepMotor = _stepMotor;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue - 125, len, leadingSpaces);
        else return emptyString(len);
      }
      else {
        return valToString(EEPROM.read(adressEEPROM) - 125, len, leadingSpaces);
      }
    };
    void saveEditing() {
      currMode.editing = false;
      stepMotor->set_positionMove(editValue - 125);
      EEPROM.update(adressEEPROM, editValue);
    };
  private:
    StepMotor* stepMotor;

};

class TimerValue: public MenuItem {

  public:
    TimerValue (const CurrSettings* _currSettings, const byte _valueIndex) {
      currSettings = _currSettings;
      valueIndex = _valueIndex;
    };
    String display() {
      if (currSettings->timer != nullptr) {
        return valToString(valueIndex ? currSettings->timer->getSecond() : currSettings->timer->getMinute(), 2);
      }
      else if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 2);
        else return "  ";
      }
      else {
        return valToString(EEPROM.read(valueIndex ? EEPROM_TIMER_SECOND : EEPROM_TIMER_MINUTE), 2);
      }
    };
    boolean editing() {
      return currSettings->timer == nullptr;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = EEPROM.read(valueIndex ? EEPROM_TIMER_SECOND : EEPROM_TIMER_MINUTE);
    };
    void downValue() {
      changeValue(editValue, -1, 0, 59);
      currMode.blinkOn = true;
    };
    void upValue() {
      changeValue(editValue, 1, 0, 59);
      currMode.blinkOn = true;
    };
    void saveEditing() {
      currMode.editing = false;
      EEPROM.update(valueIndex ? EEPROM_TIMER_SECOND : EEPROM_TIMER_MINUTE, editValue);
    };
  private:
    CurrSettings* currSettings;
    bool valueIndex;
    byte editValue;

};

class TimerStart: public MenuItem {

  public:
    TimerStart (const CurrSettings* _currSettings) {
      currSettings = _currSettings;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 2, 1);
        else return "  ";
      }
      else {
        return currSettings->timer != nullptr ? " 1" : " 0";
      }
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = currSettings->timer != nullptr;
    };
    void downValue() {
      changeValue(editValue, -1, 0, 1);
      currMode.blinkOn = true;
    };
    void upValue() {
      changeValue(editValue, 1, 0, 1);
      currMode.blinkOn = true;
    };
    void saveEditing() {
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
  private:
    byte editValue;
    CurrSettings* currSettings;

};

class RtsTemp: public MenuItem {

  public:
    RtsTemp (const MicroDS3231* _rtc) {
      rtc = _rtc;
    };
    String display() {
      int _intValue = rtc->getTemperature() * 10;
      return valToString(_intValue, 3);
    };
  private:
    MicroDS3231* rtc;

};

class AquaTemp: public MenuItem {

  public:
    AquaTemp (const ControlTemp* _controlTemp) {
      controlTemp = _controlTemp;
    };
    String display() {
      if (!controlTemp->getAquaTempConnected()) return "Err";
      else {
        int _intValue = controlTemp->getAquaTemp() * 10;
        return valToString(_intValue, 3);
      }
    };
  private:
    ControlTemp* controlTemp;

};

class TempLog: public MenuItem {

  public:
    TempLog (const CurrSettings* _currSettings, const ControlTemp* _controlTemp) {
      currSettings = _currSettings;
      controlTemp = _controlTemp;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return logToString(editValue);
        else return "        ";
      }
      else {
        return logToString(23);
      }
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = 23;
    };
    void downValue() {
      changeValue(editValue, -1, 0, 23, false);
      currMode.blinkOn = true;
    };
    void upValue() {
      changeValue(editValue, 1, 0, 23, false);
      currMode.blinkOn = true;
    };
  private:
    CurrSettings* currSettings;
    ControlTemp* controlTemp;
    byte editValue;
    String logToString(byte _index) {
      byte _indexOfNow = currSettings->nowHour;
      byte _indexOfLog;
      if ((23 - _index) > _indexOfNow) _indexOfLog = _indexOfNow + _index + 1;
      else _indexOfLog = _indexOfNow + _index - 23;
      String ans = valToString(_indexOfLog, 2);
      ans += "00";
      word _valueOfLog = controlTemp->getHeaterTempLog(_indexOfLog);
      if (_valueOfLog > 10000) {
        ans += "o";
        _valueOfLog -= 10000;
      }
      else ans += " ";
      if (_valueOfLog == 0) ans += "Err";
      else {
        _valueOfLog -= 1000;
        ans += valToString(_valueOfLog, 3);
      }
      return ans;
    }

};

class bubbleCounterValue: public MenuItem {

  public:
    bubbleCounterValue (const BubbleCounter* _bubbleCounter, const typeBubbleCounterValue _valueType) {
      bubbleCounter = _bubbleCounter;
      valueType = _valueType;
    };
    String display() {
      int _intValue = 0;
      switch (valueType) {
        case bubbleIn100Second: _intValue = bubbleCounter->getBubbleIn100Second(); break;
        case minLevel: _intValue = bubbleCounter->getMinLevel(); break;
        case maxLevel: _intValue = bubbleCounter->getMaxLevel(); break;
      }
      switch (_intValue) {
        case -1: return "Err1"; break;
        case -2: return "Err2"; break;
        case -3: return valToString(0, 4, 1); break;
        default:
          switch (valueType) {
            case bubbleIn100Second: return valToString(_intValue, 4, 1); break;
            default: return valToString(_intValue, 4, 3); break;
          }
          break;
      }
    };
  private:
    BubbleCounter* bubbleCounter;
    typeBubbleCounterValue valueType;

};

class bubbleControlValue: public MenuItem {

  public:
    bubbleControlValue (const BubbleControl* _bubbleControl, const typeBubbleControlValue _valueType) {
      bubbleControl = _bubbleControl;
      valueType = _valueType;
    };
    String display() {
      switch (valueType) {
        case controlCondition: return bubbleControl->get_condition(); break;
        case minBubblesIn100Second: return valToString(bubbleControl->get_minBubblesIn100Second(), 4, 1); break;
        case maxBubblesIn100Second: return valToString(bubbleControl->get_maxBubblesIn100Second(), 4, 1); break;
      }
    };
  private:
    BubbleControl* bubbleControl;
    typeBubbleControlValue valueType;

};

class FeedingValue: public MenuItem {

  public:
    FeedingValue (const Feeding* _feeding) {
      feeding = _feeding;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 4, 3);
        else return "  ";
      }
      else {
        return valToString(feeding->getFeedingLoop(), 4, 3);
      }
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = feeding->getFeedingLoop();
    };
    void downValue() {
      changeValue(editValue, -1, 0, 10);
      currMode.blinkOn = true;
    };
    void upValue() {
      changeValue(editValue, 1, 0, 10);
      currMode.blinkOn = true;
    };
    void saveEditing() {
      currMode.editing = false;
      feeding->setFeedingLoop(editValue);
    };
  private:
    byte editValue;
    Feeding* feeding;

};
