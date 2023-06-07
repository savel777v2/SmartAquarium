/*
  Menu.h - Library for displaying menu in TM1638.


*/

#include <TM1638.h>
#include "CurrSettings.h"
#include "MenuItem.h"

class Menu {

  public:
    Menu (TM1638* _Module, CurrSettings* _currSettings);
    void display2(const String& s) {
      Module->setDisplayToString(s);
    };
    void display();

  private:
    TM1638* Module;
    MenuItem* subMenu[4];
    CurrSettings* currSettings;
};

Menu::Menu (TM1638* _Module, CurrSettings* _currSettings) {
  Module = _Module;
  currSettings = _currSettings;  
  subMenu[0] = new DayFlag(currSettings);
  subMenu[1] = new TextItem(" ");
  subMenu[2] = new CurHour(currSettings);
  subMenu[3] = new CurMinute(currSettings);
};

void Menu::display() {
  String sOut;
  for (auto& s : subMenu) {
    if (s == nullptr) break;
    sOut += s->print();
  }
  Module->setDisplayToString(sOut);
}
