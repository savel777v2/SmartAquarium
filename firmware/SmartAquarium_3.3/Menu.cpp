/*
  Menu.cpp - Программная реализация модуля реализации меню в модуле дисплея
*/

#include "Menu.h"

Menu::Menu() {
  gorInd = 0;
  verInd = 0;
  numEditItem = 0;
  initSubmenu(submenuName(gorInd, verInd));
  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
};

bool Menu::editingMenu() {
  return numEditItem;
};

void Menu::changeBlink() {

  if (numEditItem == 0) return;
  subMenu[numEditItem - 1]->changeBlink();
  display();

};

bool Menu::readKeyboardNeedControl() {

  if (millis() <= nextKeyboardTime) return false;

  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  bool ans = false;

  byte keys = globModule1638.keysPressed(B10111111, B00111100);
  if (globModule1638.keyPressed(7, keys)) {
    // ручная регулировка света
    globLamps.changeManualLamp();
  }
  if (globModule1638.keyPressed(0, keys) && globCurrSettings.alarmMelody != nullptr) {
    // Esc - выход из мелодии
    delete globCurrSettings.alarmMelody;
    globCurrSettings.alarmMelody = nullptr;
  }
  if (numEditItem) {
    if (globModule1638.keyPressed(0, keys)) {
      // Esc - выход из редактирования
      subMenu[numEditItem - 1]->exitEditing();
      numEditItem = 0;
      display();
    }
    if (globModule1638.keyPressed(1, keys)) {
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
    if (globModule1638.keyPressed(2, keys)) {
      subMenu[numEditItem - 1]->downValue();
      globCurrSettings.lastBlinkTime = millis();
      display();
    }
    if (globModule1638.keyPressed(3, keys)) {
      subMenu[numEditItem - 1]->upValue();
      globCurrSettings.lastBlinkTime = millis();
      display();
    }
  }
  else {
    if (globModule1638.keyPressed(0, keys)) {
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
    if (globModule1638.keyPressed(1, keys)) {
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
    if (verInd == 0 && globModule1638.keyPressed(2, keys)) dGor--; // left
    if (verInd == 0 && globModule1638.keyPressed(3, keys)) dGor++; // right
    if (globModule1638.keyPressed(4, keys)) dVer++; // down
    if (globModule1638.keyPressed(5, keys)) dVer--; // up
    if ((dGor || dVer) && (submenuName(gorInd + dGor, verInd + dVer) != anon)) {
      gorInd += dGor;
      verInd += dVer;
      initSubmenu(submenuName(gorInd, verInd));
      display();
    }
  }

  return ans;
};

submenu Menu::getSubmenu() {
  return submenuName(gorInd, verInd);
};

void Menu::display() {
  String sOut;
  //globCurrSettings.startEndDurations(0);
  for (auto& menuItem : subMenu) {
    if (menuItem == nullptr) break;
    sOut += menuItem->display();
  }
  int deltaLen = sOut.length() - 8;
  if (deltaLen > 0) sOut = sOut.substring(0, 8);
  while (deltaLen++ < 0) {
    sOut += ' ';
  }
  globModule1638.setDisplayToString(sOut, getDots(submenuName(gorInd, verInd)));
  //globCurrSettings.startEndDurations(1);
};

submenu Menu::submenuName(byte _gorInd, byte _verInd) {

  switch (_gorInd) {
    case 0: switch (_verInd) {
        case 0: return timeMenu; break;
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
        case 0: return feedingLoop; break;
        case 1: return morningFeeding; break;
        case 2: return eveningFeeding; break;
        case 3: return dayFeedingSettings; break;
        case 4: return nightFeeding; break;        
        default: return anon; break;
      }
    case 4: switch (_verInd) {
        case 0: return motorPosition; break;
        case 1: return durations; break;
        default: return anon; break;
      }
  }
  return anon;

};

byte Menu::getDots(submenu _submenu) {
  switch (_submenu) {
    case timeMenu: return globCurrSettings.secondLed ? B00010000 : 0; break;
    case bubbleDaySpeed:
    case bubbleNightSpeed:
    case timer:
    case morning:
    case evening:
    case morningFeeding:
    case eveningFeeding:
    case alarm: return B00010000; break;
    case curTemp: return globControlTemp.getAquaTempConnected() ? B00100010 : B00100000; break;
    case bubbleControlSettings: return B01000100; break;
    case logTemp: return B01000010; break;
    case nightFeeding: return B01000100; break;
    case deltaTemp: return B00001000; break;
    case dayFeedingSettings:
    case bubblesInSecond: return B01000000; break;
  }
  return 0;
};

void Menu::initSubmenu(submenu _submenu) {

  for (auto& menuItem : subMenu) {
    if (menuItem != nullptr) {
      delete menuItem;
      menuItem = nullptr;
    }
  }

  switch (_submenu) {
    case timeMenu:
      subMenu[0] = new SettingsValue(dayNight);
      subMenu[1] = new TextItem(" ");
      subMenu[2] = new TimeValue(0);
      subMenu[3] = new TimeValue(1);
      subMenu[4] = new SettingsValue(timerOn);
      subMenu[5] = new AlarmFlag();
      break;
    case timer:
      subMenu[0] = new TextItem("St");
      subMenu[1] = new TimerValue(0);
      subMenu[2] = new TimerValue(1);
      subMenu[3] = new TimerStart();
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
      subMenu[3] = new byteEEPROMvalue(EEPROM_ALARM, 0, 1, 2, 1);
      break;
    case lampInterval:
      subMenu[0] = new TextItem("Sdn ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_LAMP_INTERVAL, 0, 30, 2);
      break;
    case curTemp:
      subMenu[0] = new TextItem("i");
      subMenu[1] = new RtsTemp();
      subMenu[2] = new TextItem("o");
      subMenu[3] = new AquaTemp();
      break;
    case logTemp:
      subMenu[0] = new TempLog();
      break;
    case dayTemp:
      subMenu[0] = new TextItem("Td  ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_DAY_TEMP, 14, 30, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_DAY_TEMP_ON, 0, 1, 2, 1);
      break;
    case nightTemp:
      subMenu[0] = new TextItem("Tn  ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_NIGHT_TEMP, 14, 30, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_NIGHT_TEMP_ON, 0, 1, 2, 1);
      break;
    case deltaTemp:
      subMenu[0] = new TextItem("dt  ");
      subMenu[1] = new byteEEPROMvalue(EEPROM_DELTA_TEMP, 5, 10, 2);
      break;
    case bubblesInSecond:
      subMenu[0] = new bubbleCounterValue(bubbleIn100Second);
      subMenu[1] = new bubbleControlValue(controlCondition);
      break;
    case bubbleControlSettings:
      subMenu[0] = new bubbleControlValue(minBubblesIn100Second);
      subMenu[1] = new bubbleControlValue(maxBubblesIn100Second);
      break;
    case sensorValue:
      subMenu[0] = new bubbleCounterValue(minLevel);
      subMenu[1] = new bubbleCounterValue(maxLevel);
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
    case feedingLoop:
      subMenu[0] = new TextItem("Feed");
      subMenu[1] = new FeedingValue();
      break;
    case morningFeeding:
      subMenu[0] = new TextItem("Fd");
      subMenu[1] = new byteEEPROMvalue(EEPROM_MORNING_FEEDING_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_MORNING_FEEDING_MINUTE, 0, 59, 2);
      subMenu[3] = new byteEEPROMvalue(EEPROM_MORNING_FEEDING_LOOP, 0, 20, 2, 1);
      break;
    case eveningFeeding:
      subMenu[0] = new TextItem("Fd");
      subMenu[1] = new byteEEPROMvalue(EEPROM_EVENING_FEEDING_HOUR, 0, 23, 2);
      subMenu[2] = new byteEEPROMvalue(EEPROM_EVENING_FEEDING_MINUTE, 0, 59, 2);
      subMenu[3] = new byteEEPROMvalue(EEPROM_EVENING_FEEDING_LOOP, 0, 20, 2, 1);
      break;
    case dayFeedingSettings:
      subMenu[0] = new TextItem("d");
      subMenu[1] = new byteEEPROMvalue(EEPROM_DAY_FEEDING_DURATION, 0, 250, 3, 0);
      subMenu[2] = new TextItem("p");
      subMenu[3] = new byteEEPROMvalue(EEPROM_DAY_FEEDING_PAUSE, 0, 99, 3, 2, 10);
      break;
    case nightFeeding:
      subMenu[0] = new byteEEPROMvalue(EEPROM_NIGHT_FEEDING_HOUR, 0, 23, 2);
      subMenu[1] = new byteEEPROMvalue(EEPROM_NIGHT_FEEDING_MINUTE, 0, 59, 2);
      subMenu[2] = new TextItem("d");
      subMenu[3] = new byteEEPROMvalue(EEPROM_NIGHT_FEEDING_DURATION, 0, 250, 3, 0);
      break;
    case motorPosition:
      subMenu[0] = new TextItem("POS ");
      subMenu[1] = new MotorPosition();
      break;
    case durations:
      subMenu[0] = new SettingsValue(dur);
      break;    
  }
};
