/*
  Menu.h - Library for displaying menu in TM1638.


*/
#pragma once

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
    submenu getSubmenu();

  private:
    TM1638* Module;
    MenuItem* subMenu[6];
    CurrSettings* currSettings;
    byte gorInd, verInd;
    void goIndex();
    submenu submenuName(byte gorInd, byte verInd);
};

Menu::Menu (TM1638* _Module, CurrSettings* _currSettings) {
  Module = _Module;
  currSettings = _currSettings;
  gorInd = 0;
  verInd = 0;
  goIndex();
};

submenu Menu::getSubmenu() {
  return submenuName(gorInd, verInd);
};

submenu Menu::submenuName(byte _gorInd, byte _verInd) {

  switch (_gorInd) {
    case 0: switch (_verInd) {
        case 0:
          return time;
          break;
        case 1:
          return timer;
          break;
      }
  }
  return anon;
}

void Menu::goIndex() {

  switch (gorInd) {
    case 0: switch (verInd) {
        case 0:
          subMenu[0] = new DayFlag(currSettings);
          subMenu[1] = new TextItem(" ");
          subMenu[2] = new CurHour(currSettings);
          subMenu[3] = new CurMinute(currSettings);
          subMenu[4] = new TimerFlag(currSettings);
          subMenu[5] = new AlarmFlag();
          break;
        case 1:
          subMenu[0] = nullptr;
          subMenu[1] = nullptr;
          subMenu[2] = nullptr;
          subMenu[3] = nullptr;
          subMenu[4] = nullptr;
          subMenu[5] = nullptr;
          break;
      }
  }
}

void Menu::display() {
  String sOut;
  for (auto& menuItem : subMenu) {
    if (menuItem == nullptr) break;
    sOut += menuItem->display();
  }
  Module->setDisplayToString(sOut);
}
