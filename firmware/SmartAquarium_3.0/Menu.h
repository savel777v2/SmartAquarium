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
  bubblesInSecond, bubbleControlSettings, sensorValue, bubbleSettings, bubbleDaySpeed, bubbleNightSpeed, beforeMorningStart, bubbleControlSound,
  eating, morningEat, eveningEat, dayEatingSettings, nightEat, durations,
  motorPosition, motorSpeed, bubbleDurations, bubbleCount, bubblesInMinute, sensorInSecond, errorsInSecond,
  anon
};

class Menu {

  public:
    Menu (TM1638My* _module, ControlTemp* _controlTemp, BubbleCounter* _bubbleCounter, StepMotor* _stepMotor, BubbleControl* _bubbleControl, MicroDS3231* _rtc, CurrSettings* _currSettings);
    void display();
    bool loopNeedControl();
    submenu getSubmenu();

  private:
    TM1638My* module;
    MenuItem* subMenu[6];
    ControlTemp* controlTemp;
    BubbleCounter* bubbleCounter;
    StepMotor* stepMotor;
    BubbleControl* bubbleControl;
    MicroDS3231* rtc;
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

Menu::Menu (TM1638My* _module, ControlTemp* _controlTemp, BubbleCounter* _bubbleCounter, StepMotor* _stepMotor, BubbleControl* _bubbleControl, MicroDS3231* _rtc, CurrSettings* _currSettings) {
  module = _module;
  controlTemp = _controlTemp;
  bubbleCounter = _bubbleCounter;
  stepMotor = _stepMotor;
  bubbleControl = _bubbleControl;
  rtc = _rtc;
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

  byte keys = module->keysPressed(B00111111, B00111100);
  if (module->keyPressed(0, keys) && currSettings->alarmMelody != nullptr) {
    // Esc - выход из мелодии
    delete currSettings->alarmMelody;
    currSettings->alarmMelody = nullptr;
  }
  if (numEditItem) {
    if (module->keyPressed(0, keys)) {
      // Esc - выход из редактирования
      subMenu[numEditItem - 1]->exitEditing();
      numEditItem = 0;
      display();
    }
    if (module->keyPressed(1, keys)) {
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
    if (module->keyPressed(2, keys)) {
      subMenu[numEditItem - 1]->downValue();
      lastBlinkTime = millis();
      display();
    }
    if (module->keyPressed(3, keys)) {
      subMenu[numEditItem - 1]->upValue();
      lastBlinkTime = millis();
      display();
    }
  }
  else {
    if (module->keyPressed(0, keys)) {
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
    if (module->keyPressed(1, keys)) {
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
    if (verInd == 0 && module->keyPressed(2, keys)) dGor--; // left
    if (verInd == 0 && module->keyPressed(3, keys)) dGor++; // right
    if (module->keyPressed(4, keys)) dVer++; // down
    if (module->keyPressed(5, keys)) dVer--; // up
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
  module->setDisplayToString(sOut, getDots(submenuName(gorInd, verInd)));
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
        default: return anon; break;
      }
    case 1: switch (_verInd) {
        case 0: return curTemp; break;
        case 1: return logTemp; break;
        case 2: return dayTemp; break;
        case 3: return nightTemp; break;
        case 4: return deltaTemp; break;
        default: return anon; break;
      }
    case 2: switch (_verInd) {
        case 0: return bubblesInSecond; break;
        case 1: return bubbleControlSettings; break;
        case 2: return sensorValue; break;
        case 3: return bubbleSettings; break;
        case 4: return bubbleDaySpeed; break;
        case 5: return bubbleNightSpeed; break;
        case 6: return beforeMorningStart; break;
        case 7: return bubbleControlSound; break;
        default: return anon; break;
      }
    case 3: switch (_verInd) {
        case 0: return eating; break;
        case 1: return morningEat; break;
        case 2: return eveningEat; break;
        case 3: return dayEatingSettings; break;
        case 4: return nightEat; break;
        case 5: return durations; break;
        default: return anon; break;
      }
    case 4: switch (_verInd) {
        case 0: return motorPosition; break;
        case 1: return motorSpeed; break;
        case 2: return bubbleDurations; break;
        case 3: return bubbleCount; break;
        case 4: return bubblesInMinute; break;
        case 5: return sensorInSecond; break;
        case 6: return errorsInSecond; break;
        default: return anon; break;
      }
  }
  return anon;

}

byte Menu::getDots(submenu _submenu) {
  switch (_submenu) {
    case time: return currSettings->secondLed ? B00010000 : 0; break;
    case bubbleDaySpeed:
    case bubbleNightSpeed:
    case timer:
    case morning:
    case evening:
    case alarm: return B00010000; break;
    case curTemp: return controlTemp->getAquaTempStatus() == normal ? B00100010 : B00100000; break;
    case logTemp: return B01000010; break;
    case deltaTemp: return B00001000; break;
    case bubblesInSecond: return B01000000; break;
  }
  return 0;
}

void Menu::initSubmenu(submenu _submenu) {

  for (auto& menuItem : subMenu) {
    if (menuItem != nullptr) {
      delete menuItem;
      menuItem = nullptr;
    }
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
      break;
    case morning:
      subMenu[0] = new TextItem("Sd");
      subMenu[1] = new byteEEPROMvalue(EEPROM_MORNING_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_MORNING_MINUTE, 0, 59, 2);
      break;
    case evening:
      subMenu[0] = new TextItem("Sn");
      subMenu[1] = new byteEEPROMvalue(EEPROM_EVENING_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_EVENING_MINUTE, 0, 59, 2);
      break;
    case alarm:
      subMenu[0] = new TextItem("Sb");
      subMenu[1] = new byteEEPROMvalue(EEPROM_ALARM_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_ALARM_MINUTE, 0, 59, 2);
      subMenu[3] = new TextItem(" ");
      subMenu[4] = new byteEEPROMvalue(EEPROM_ALARM, 0, 1, 1);
      break;
    case lampInterval:
      subMenu[0] = new TextItem("Sdn");
      subMenu[1] = new TextItem(" ");
      subMenu[2] = new byteEEPROMvalue(EEPROM_LAMP_INTERVAL, 0, 30, 2);
      break;
    case curTemp:
      subMenu[0] = new TextItem("i");
      subMenu[1] = new RtsTemp(rtc);
      subMenu[2] = new TextItem("o");
      subMenu[3] = new AquaTemp(controlTemp);
      break;
    case logTemp:
      subMenu[0] = new TempLog (currSettings, controlTemp);
      break;
    case dayTemp:
      subMenu[0] = new TextItem("Td  ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_DAY_TEMP, 14, 30, 2);
      subMenu[2] = new TextItem(" ");
      subMenu[3] = new byteEEPROMvalue(EEPROM_DAY_TEMP_ON, 0, 1, 1);
      break;
    case nightTemp:
      subMenu[0] = new TextItem("Tn  ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_NIGHT_TEMP, 14, 30, 2);
      subMenu[2] = new TextItem(" ");
      subMenu[3] = new byteEEPROMvalue(EEPROM_NIGHT_TEMP_ON, 0, 1, 1);
      break;
    case deltaTemp:
      subMenu[0] = new TextItem("dt  ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_DELTA_TEMP, 5, 10, 2);
      break;
    case bubblesInSecond:
      subMenu[0] = new bubbleCounterValue(bubbleCounter, bubbleIn100Second);
      break;
    case bubbleControlSettings:
      subMenu[0] = new TextItem("31");
      break;
    case sensorValue:
      subMenu[0] = new bubbleCounterValue(bubbleCounter, minLevel);
      subMenu[1] = new bubbleCounterValue(bubbleCounter, maxLevel);
      break;
    case bubbleSettings:
      subMenu[0] = new TextItem("d ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_MAX_DURATION_BUBBLE, 0, 50, 2, 1);
      subMenu[2] = new TextItem("h ");
      subMenu[3] = new byteEEPROMvalue(EEPROM_MIN_LEVEL_BUBBLE, 0, 50, 2, 1);
      break;
    case bubbleDaySpeed:
      subMenu[0] = new TextItem("Bd");
      subMenu[1] = new byteEEPROMvalue(EEPROM_DAY_BUBBLE_SPEED, 19, 250, 4, 1);
      subMenu[2] = new byteEEPROMvalue(EEPROM_DAY_BUBBLE_ON, 0, 1, 2, 1);
      break;
    case bubbleNightSpeed:
      subMenu[0] = new TextItem("Bn");
      subMenu[1] = new byteEEPROMvalue(EEPROM_NIGHT_BUBBLE_SPEED, 19, 250, 4, 1);
      subMenu[2] = new byteEEPROMvalue(EEPROM_NIGHT_BUBBLE_ON, 0, 1, 2, 1);
      break;
    case beforeMorningStart:
      subMenu[0] = new TextItem("bd");
      subMenu[1] = new byteEEPROMvalue(EEPROM_BEFORE_MORNING_BUBBLE_START, 0, 250, 4, 3);
      break;
    case bubbleControlSound:
      subMenu[0] = new TextItem("Sound");
      subMenu[1] = new byteEEPROMvalue(EEPROM_CONTROL_BUBBLE_SOUND_ON, 0, 1, 3, 2);
      break;
    case eating:
      subMenu[0] = new TextItem("40");
      break;
    case morningEat:
      subMenu[0] = new TextItem("41");
      break;
    case eveningEat:
      subMenu[0] = new TextItem("42");
      break;
    case dayEatingSettings:
      subMenu[0] = new TextItem("43");
      break;
    case nightEat:
      subMenu[0] = new TextItem("44");
      break;
    case durations:
      subMenu[0] = new TextItem("45");
      break;
    case motorPosition:
      subMenu[0] = new TextItem("POS ");
      subMenu[1] = new MotorPosition(stepMotor);
      break;
    case motorSpeed:
      subMenu[0] = new TextItem("51");
      break;
    case bubbleDurations:
      subMenu[0] = new TextItem("52");
      break;
    case bubbleCount:
      subMenu[0] = new TextItem("53");
      break;
    case bubblesInMinute:
      subMenu[0] = new TextItem("54");
      break;
    case sensorInSecond:
      subMenu[0] = new TextItem("55");
      break;
    case errorsInSecond:
      subMenu[0] = new TextItem("56");
      break;
  }
}
