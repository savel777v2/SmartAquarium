/*
  MenuItem.h - Library for displaying menu in TM1638.


*/

#include "CurrSettings.h"

class MenuItem {

  public:
    MenuItem () {
      currSettings = nullptr;
    };
    MenuItem (CurrSettings* _currSettings) {
      currSettings = _currSettings;
    };
    virtual String print() = 0;

  protected:
    CurrSettings* currSettings;
    String toString(int val, int len) {
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
    String print() {
      return s;
    };

  private:
    String s;

};

class DayFlag: public MenuItem {

  public:
    DayFlag (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String print() {
      if (currSettings->nowDay) return "d";
      else return "n";
    };

};

class CurHour: public MenuItem {

  public:
    CurHour (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String print() {
      return toString(currSettings->now.hour, 2);
    };

};

class CurMinute: public MenuItem {

  public:
    CurMinute (CurrSettings* _currSettings) : MenuItem(_currSettings) {};
    String print() {
      return toString(currSettings->now.minute, 2);
    };

};
