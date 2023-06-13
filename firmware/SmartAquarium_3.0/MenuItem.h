/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

#include <EEPROM.h>
#include "CurrSettings.h"

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

    String valToString(int val, int len) {
      String ans(val);
      int ind = ans.length() - len;
      if (ind > 0) return ans.substring(ind);
      String pref = "";
      while (ind++ < 0) {
        pref += '0';
      }
      return pref + ans;
    }

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
      if (currSettings->timerOn) return "t";
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
      else return valToString(currSettings->now.hour, 2);
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = currSettings->now.hour;
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
      currSettings->now.hour = editValue;
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
      else return valToString(currSettings->now.minute, 2);
    };
    boolean editing() {
      return true;
    };
    void enterEditing() {
      currMode.editing = currMode.blinkOn = true;
      editValue = currSettings->now.minute;
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
      currSettings->now.minute = editValue;
    };
  private:
    byte editValue;

};

class byteEEPROMvalue: public MenuItem {

  public:
    byteEEPROMvalue (int _adressEEPROM, byte _minValue, byte _maxValue, byte _len) {
      adressEEPROM = _adressEEPROM;
      minValue = _minValue;
      maxValue = _maxValue;
      len = _len;
    };
    String display() {
      if (currMode.editing) {
        if (currMode.blinkOn) return valToString(editValue, len);
        else return emptyString(len);
      }
      else {
        return valToString(EEPROM.read(adressEEPROM), len);
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
    int adressEEPROM;
    byte editValue;
    byte minValue, maxValue, len;

};
