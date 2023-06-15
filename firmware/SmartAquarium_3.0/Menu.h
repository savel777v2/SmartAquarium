/*
  Menu.h - Library for displaying menu in TM1638.


*/
#pragma once

#define BLINK_INTERVAL 500

#include "TM1638My.h"
#include "CurrSettings.h"
#include "MenuItem.h"

enum submenu
{
  time, timer, morning, evening, alarm, lampInterval,
  curTemp, logTemp, dayTemp, nightTemp, deltaTemp,
  durations,
  anon
};

class Menu {

  public:
    Menu (TM1638My* _Module, CurrSettings* _currSettings);
    void display();
    bool loopNeedControl();
    submenu getSubmenu();

  private:
    TM1638My* Module;
    MenuItem* subMenu[6];
    CurrSettings* currSettings;
    byte gorInd, verInd;
    unsigned long nextKeyboardTime, lastBlinkTime;
    byte numEditItem;

    void initSubmenu(submenu _submenu);
    submenu submenuName(byte _gorInd, byte _verInd);
    byte getDots(submenu _submenu);
    bool readKeyboardNeedControl();
    void blinkDisplay();
};

Menu::Menu (TM1638My* _Module, CurrSettings* _currSettings) {
  Module = _Module;
  currSettings = _currSettings;
  gorInd = 0;
  verInd = 0;
  numEditItem = 0;
  initSubmenu(submenuName(gorInd, verInd));
  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  lastBlinkTime = 0;
};

bool Menu::loopNeedControl() {
  blinkDisplay();
  return readKeyboardNeedControl();
}

void Menu::blinkDisplay() {

  if (numEditItem == 0 || (millis() - lastBlinkTime) <= BLINK_INTERVAL) return;
  lastBlinkTime = millis();

  subMenu[numEditItem - 1]->changeBlink();
  display();

}

bool Menu::readKeyboardNeedControl() {

  if (millis() <= nextKeyboardTime) return false;

  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  bool ans = false;

  byte keys = Module->keysPressed(B00111111, B00111100);
  if (Module->keyPressed(0, keys) && currSettings->alarmMelody != nullptr) {
    // Esc - выход из мелодии
    delete currSettings->alarmMelody;
    currSettings->alarmMelody = nullptr;
  }
  if (numEditItem) {
    if (Module->keyPressed(0, keys)) {
      // Esc - выход из редактирования
      subMenu[numEditItem - 1]->exitEditing();      
      numEditItem = 0;
      display();
    }
    if (Module->keyPressed(1, keys)) {
      // Enter - сохраняем и к следующему
      subMenu[numEditItem - 1]->saveEditing();
      ans = true;
      int i;
      int sizeSubMenu = sizeof(subMenu) / sizeof(subMenu[0]);
      for (i = numEditItem; i < sizeSubMenu; i++) {
        if (subMenu[i] != nullptr && subMenu[i]->editing()) break;
      }
      if (i < sizeSubMenu) {
        numEditItem = i + 1;
        subMenu[numEditItem - 1]->enterEditing();
        display();
      }
      else {
        numEditItem = 0;
        display();
      }
    }
    if (Module->keyPressed(2, keys)) {
      subMenu[numEditItem - 1]->downValue();
      lastBlinkTime = millis();
      display();
    }
    if (Module->keyPressed(3, keys)) {
      subMenu[numEditItem - 1]->upValue();
      lastBlinkTime = millis();
      display();
    }
  }
  else {
    if (Module->keyPressed(0, keys)) {
      // Esc - возврат меню на адрес 0-0
      if (verInd) {
        verInd = 0;
        initSubmenu(submenuName(gorInd, verInd));
        display();
      }
      else if (gorInd) {
        gorInd = 0;
        initSubmenu(submenuName(gorInd, verInd));
        display();
      }
    }
    if (Module->keyPressed(1, keys)) {
      // Enter - режим редактирования
      int i;
      int sizeSubMenu = sizeof(subMenu) / sizeof(subMenu[0]);
      for (i = numEditItem; i < sizeSubMenu; i++) {
        if (subMenu[i] != nullptr && subMenu[i]->editing()) break;
      }
      if (i < sizeSubMenu) {
        numEditItem = i + 1;
        subMenu[numEditItem - 1]->enterEditing();
        display();
      }
    }
    int dVer = 0, dGor = 0;
    if (verInd == 0 && Module->keyPressed(2, keys)) dGor--; // left
    if (verInd == 0 && Module->keyPressed(3, keys)) dGor++; // right
    if (Module->keyPressed(4, keys)) dVer++; // down
    if (Module->keyPressed(5, keys)) dVer--; // up
    if ((dGor || dVer) && (submenuName(gorInd + dGor, verInd + dVer) != anon)) {
      gorInd += dGor;
      verInd += dVer;
      initSubmenu(submenuName(gorInd, verInd));
      display();
    }
  }

  return ans;
}

submenu Menu::getSubmenu() {
  return submenuName(gorInd, verInd);
};

void Menu::display() {
  String sOut;
  for (auto& menuItem : subMenu) {
    if (menuItem == nullptr) break;
    sOut += menuItem->display();
  }
  int deltaLen = sOut.length() - 8;
  if (deltaLen > 0) sOut = sOut.substring(0, 8);
  while (deltaLen++ < 0) {
    sOut += ' ';
  }
  Module->setDisplayToString(sOut, getDots(submenuName(gorInd, verInd)));
}

submenu Menu::submenuName(byte _gorInd, byte _verInd) {

  switch (_gorInd) {
    case 0: switch (_verInd) {
        case 0: return time; break;
        case 1: return timer; break;
        case 2: return morning; break;
        case 3: return evening; break;
        case 4: return alarm; break;
        case 5: return lampInterval; break;
      }
    case 1: switch (_verInd) {
        case 0: return curTemp; break;
        case 1: return logTemp; break;
        case 2: return dayTemp; break;
        case 3: return nightTemp; break;
        case 4: return deltaTemp; break;
      }
  }
  return anon;

}

byte Menu::getDots(submenu _submenu) {
  switch (_submenu) {
    case time: return currSettings->secondLed ? B00010000 : 0; break;
    case timer: return B00010000; break;
    case morning: return B00010000; break;
    case evening: return B00010000; break;
    case alarm: return B00010000; break;
    case lampInterval: return 0; break;
    case curTemp: return B00010000; break;
    case logTemp: return B00010000; break;
    case dayTemp: return B00010000; break;
    case nightTemp: return B00010000; break;
    case deltaTemp: return B00010000; break;
  }
  return 0;
}

void Menu::initSubmenu(submenu _submenu) {

  for (auto& menuItem : subMenu) {
    if (menuItem != nullptr) delete menuItem;
  }

  switch (_submenu) {
    case time:
      subMenu[0] = new DayFlag(currSettings);
      subMenu[1] = new TextItem(" ");
      subMenu[2] = new CurHour(currSettings);
      subMenu[3] = new CurMinute(currSettings);
      subMenu[4] = new TimerFlag(currSettings);
      subMenu[5] = new AlarmFlag();
      break;
    case timer:
      subMenu[0] = new TextItem("St");
      subMenu[1] = new TimerMinute(currSettings);
      subMenu[2] = new TimerSecond(currSettings);
      subMenu[3] = new TextItem(" ");
      subMenu[4] = new TimerStart(currSettings);
      subMenu[5] = nullptr;
      break;
    case morning:
      subMenu[0] = new TextItem("Sd");
      subMenu[1] = new byteEEPROMvalue(EEPROM_MORNING_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_MORNING_MINUTE, 0, 59, 2);
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
    case evening:
      subMenu[0] = new TextItem("Sn");
      subMenu[1] = new byteEEPROMvalue(EEPROM_EVENING_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_EVENING_MINUTE, 0, 59, 2);
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
    case alarm:
      subMenu[0] = new TextItem("Sb");
      subMenu[1] = new byteEEPROMvalue(EEPROM_ALARM_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_ALARM_MINUTE, 0, 59, 2);
      subMenu[3] = new TextItem(" ");
      subMenu[4] = new byteEEPROMvalue(EEPROM_ALARM, 0, 1, 1);
      subMenu[5] = nullptr;
      break;
    case lampInterval:
      subMenu[0] = new TextItem("Sdn");
      subMenu[1] = new TextItem(" ");
      subMenu[2] = new byteEEPROMvalue(EEPROM_LAMP_INTERVAL, 0, 30, 2);
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
    case curTemp:
      subMenu[0] = new TextItem("10");
      subMenu[1] = nullptr;
      subMenu[2] = nullptr;
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
    case logTemp:
      subMenu[0] = new TextItem("11");
      subMenu[1] = nullptr;
      subMenu[2] = nullptr;
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
    case dayTemp:
      subMenu[0] = new TextItem("123456789");
      subMenu[1] = nullptr;
      subMenu[2] = nullptr;
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
    case nightTemp:
      subMenu[0] = new TextItem("13");
      subMenu[1] = nullptr;
      subMenu[2] = nullptr;
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
    case deltaTemp:
      subMenu[0] = new TextItem("14");
      subMenu[1] = nullptr;
      subMenu[2] = nullptr;
      subMenu[3] = nullptr;
      subMenu[4] = nullptr;
      subMenu[5] = nullptr;
      break;
  }
}
