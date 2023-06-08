/*
  MenuItem.h - Library for displaying menu in TM1638.


*/
#pragma once

#include <EEPROM.h>
#include "CurrSettings.h"

class MenuItem {

  public:
    MenuItem () {
      currSettings = nullptr;
    };
    MenuItem (CurrSettings* _currSettings) {
      currSettings = _currSettings;
    };
    virtual String display() = 0;

  protected:
    CurrSettings* currSettings;
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
      return valToString(currSettings->now.hour, 2);
    };

};

class CurMinute: public MenuItem {

  public:
    CurMinute (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String display() {
      return valToString(currSettings->now.minute, 2);
    };

};
