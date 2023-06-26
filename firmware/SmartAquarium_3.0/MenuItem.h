/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

#include <EEPROM.h>

enum typeBubbleCounterValue
{
  bubbleIn100Second, minLevel, maxLevel
};

enum typeBubbleControlValue
{
  controlCondition, minBubblesIn100Second, maxBubblesIn100Second
};

class MenuItem {

  public:
    MenuItem() {
      currSettings = nullptr;
      currMode.editing = false;
    };
    MenuItem(CurrSettings* _currSettings) {
      currSettings = _currSettings;
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
    CurrSettings* currSettings;
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
    TextItem (String _s) {
      s = _s;
    };
    String display() {
      return s;
    };

  private:
    String s;

};

class DayFlag: public MenuItem {

  public:
    DayFlag (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      if (currSettings->nowDay) return "d";
      else return "n";
    };

};

class TimerFlag: public MenuItem {

  public:
    TimerFlag (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      if (currSettings->timer != nullptr) return "t";
      else return " ";
    };

};

class AlarmFlag: public MenuItem {

  public:
    String display() {
      if (EEPROM.read(EEPROM_ALARM) == 1) return "b";
      else return " ";
    };

};

class CurHour: public MenuItem {

  public:
    CurHour (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 2);
        else return "  ";
      }
      else return valToString(currSettings->nowHour, 2);
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = currSettings->nowHour;
    };
    void downValue() {
      changeValue(editValue, -1, 0, 23);
      currMode.blinkOn = true;
    };
    void upValue() {
      changeValue(editValue, 1, 0, 23);
      currMode.blinkOn = true;
    };
    void saveEditing() {
      currMode.editing = false;
      currSettings->nowHour = editValue;
    };
  private:
    byte editValue;
};

class CurMinute: public MenuItem {

  public:
    CurMinute (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 2);
        else return "  ";
      }
      else return valToString(currSettings->nowMinute, 2);
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = currSettings->nowMinute;
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
      currSettings->nowMinute = editValue;
    };
  private:
    byte editValue;

};

class byteEEPROMvalue: public MenuItem {

  public:
    byteEEPROMvalue (int _adressEEPROM, byte _minValue, byte _maxValue, byte _len, byte _leadingSpaces = 0, byte _multiplier = 1) {
      adressEEPROM = _adressEEPROM;
      minValue = _minValue;
      maxValue = _maxValue;
      len = _len;
      leadingSpaces = _leadingSpaces;
      multiplier = _multiplier;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString((int)editValue*multiplier, len, leadingSpaces);
        else return emptyString(len);
      }
      else {
        return valToString((int)EEPROM.read(adressEEPROM)*multiplier, len, leadingSpaces);
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
    MotorPosition (StepMotor* _stepMotor) : byteEEPROMvalue (EEPROM_MOTOR_POSITION, 0, 250, 4, 3) {
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

class TimerMinute: public MenuItem {

  public:
    TimerMinute (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      if (currSettings->timer != nullptr) {
        return valToString(currSettings->timer->getMinute(), 2);
      }
      else if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 2);
        else return "  ";
      }
      else {
        return valToString(EEPROM.read(EEPROM_TIMER_MINUTE), 2);
      }
    };
    boolean editing() {
      return currSettings->timer == nullptr;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = EEPROM.read(EEPROM_TIMER_MINUTE);
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
      EEPROM.update(EEPROM_TIMER_MINUTE, editValue);
    };
  private:
    byte editValue;

};

class TimerSecond: public MenuItem {

  public:
    TimerSecond (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      if (currSettings->timer != nullptr) {
        return valToString(currSettings->timer->getSecond(), 2);
      }
      else if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 2);
        else return "  ";
      }
      else {
        return valToString(EEPROM.read(EEPROM_TIMER_SECOND), 2);
      }
    };
    boolean editing() {
      return currSettings->timer == nullptr;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = EEPROM.read(EEPROM_TIMER_SECOND);
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
      EEPROM.update(EEPROM_TIMER_SECOND, editValue);
    };
  private:
    byte editValue;

};

class TimerStart: public MenuItem {

  public:
    TimerStart (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, 1);
        else return " ";
      }
      else {
        return currSettings->timer != nullptr ? "1" : "0";
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

};

class RtsTemp: public MenuItem {

  public:
    RtsTemp (MicroDS3231* _rtc) {
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
    AquaTemp (ControlTemp* _controlTemp) {
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
    TempLog (CurrSettings* _currSettings, ControlTemp* _controlTemp) : MenuItem(_currSettings) {
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
    bubbleCounterValue (BubbleCounter* _bubbleCounter, typeBubbleCounterValue _typeValue) {
      bubbleCounter = _bubbleCounter;
      typeValue = _typeValue;
    };
    String display() {
      int _intValue = 0;
      switch (typeValue) {
        case bubbleIn100Second: _intValue = bubbleCounter->getBubbleIn100Second(); break;
        case minLevel: _intValue = bubbleCounter->getMinLevel(); break;
        case maxLevel: _intValue = bubbleCounter->getMaxLevel(); break;
      }
      switch (_intValue) {
        case -1: return "Err1"; break;
        case -2: return "Err2"; break;
        case -3: return valToString(0, 4, 1); break;
        default:
          switch (typeValue) {
            case bubbleIn100Second: return valToString(_intValue, 4, 1); break;
            default: return valToString(_intValue, 4, 3); break;
          }
          break;
      }
    };
  private:
    BubbleCounter* bubbleCounter;
    typeBubbleCounterValue typeValue;

};

class bubbleControlValue: public MenuItem {

  public:
    bubbleControlValue (BubbleControl* _bubbleControl, typeBubbleControlValue _typeValue) {
      bubbleControl = _bubbleControl;
      typeValue = _typeValue;
    };
    String display() {      
      switch (typeValue) {
        case controlCondition: return bubbleControl->get_condition(); break;
        case minBubblesIn100Second: return valToString(bubbleControl->get_minBubblesIn100Second(), 4, 1); break;
        case maxBubblesIn100Second: return valToString(bubbleControl->get_maxBubblesIn100Second(), 4, 1); break;
      }
    };
  private:
    BubbleControl* bubbleControl;
    typeBubbleControlValue typeValue;

};
