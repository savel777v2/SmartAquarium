
bool menuLoopNeedControl() {
  menuBlinkDisplay();
  return menuReadKeyboardNeedControl();
}

void menuBlinkDisplay() {  

  if (menuSettings.numEditItem == 0 || (millis() -  menuSettings.lastBlinkTime) <= BLINK_INTERVAL) return;
  menuSettings.lastBlinkTime = millis();

  menuSettings.subMenu[ menuSettings.numEditItem - 1]->changeBlink();
  menuDisplay();

}

bool menuReadKeyboardNeedControl() {
  static unsigned long nextKeyboardTime;

  if (millis() <= nextKeyboardTime) return false;

  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  bool ans = false;

  byte keys = module.keysPressed(B00111111, B00111100);
  if (module.keyPressed(0, keys) && currSettings.alarmMelody != nullptr) {
    // Esc - выход из мелодии
    delete currSettings.alarmMelody;
    currSettings.alarmMelody = nullptr;
  }
  if (menuSettings.numEditItem) {
    if (module.keyPressed(0, keys)) {
      // Esc - выход из редактирования
      menuSettings.subMenu[menuSettings.numEditItem - 1]->exitEditing();
      menuSettings.numEditItem = 0;
      menuDisplay();
    }
    if (module.keyPressed(1, keys)) {
      // Enter - сохраняем и к следующему
      menuSettings.subMenu[menuSettings.numEditItem - 1]->saveEditing();
      ans = true;
      int i;
      int sizeSubMenu = sizeof(menuSettings.subMenu) / sizeof(menuSettings.subMenu[0]);
      for (i = menuSettings.numEditItem; i < sizeSubMenu; i++) {
        if (menuSettings.subMenu[i] != nullptr && menuSettings.subMenu[i]->editing()) break;
      }
      if (i < sizeSubMenu) {
        menuSettings.numEditItem = i + 1;
        menuSettings.subMenu[menuSettings.numEditItem - 1]->enterEditing();
        menuDisplay();
      }
      else {
        menuSettings.numEditItem = 0;
        menuDisplay();
      }
    }
    if (module.keyPressed(2, keys)) {
      menuSettings.subMenu[menuSettings.numEditItem - 1]->downValue();
      menuSettings.lastBlinkTime = millis();
      menuDisplay();
    }
    if (module.keyPressed(3, keys)) {
      menuSettings.subMenu[menuSettings.numEditItem - 1]->upValue();
      menuSettings.lastBlinkTime = millis();
      menuDisplay();
    }
  }
  else {
    if (module.keyPressed(0, keys)) {
      // Esc - возврат меню на адрес 0-0
      if (menuSettings.verInd) {
        menuSettings.verInd = 0;
        initSubmenu(submenuName(menuSettings.gorInd, menuSettings.verInd));
        menuDisplay();
      }
      else if (menuSettings.gorInd) {
        menuSettings.gorInd = 0;
        initSubmenu(submenuName(menuSettings.gorInd, menuSettings.verInd));
        menuDisplay();
      }
    }
    if (module.keyPressed(1, keys)) {
      // Enter - режим редактирования
      int i;
      int sizeSubMenu = sizeof(menuSettings.subMenu) / sizeof(menuSettings.subMenu[0]);
      for (i = menuSettings.numEditItem; i < sizeSubMenu; i++) {
        if (menuSettings.subMenu[i] != nullptr && menuSettings.subMenu[i]->editing()) break;
      }
      if (i < sizeSubMenu) {
        menuSettings.numEditItem = i + 1;
        menuSettings.subMenu[menuSettings.numEditItem - 1]->enterEditing();
        menuDisplay();
      }
    }
    int dVer = 0, dGor = 0;
    if (menuSettings.verInd == 0 && module.keyPressed(2, keys)) dGor--; // left
    if (menuSettings.verInd == 0 && module.keyPressed(3, keys)) dGor++; // right
    if (module.keyPressed(4, keys)) dVer++; // down
    if (module.keyPressed(5, keys)) dVer--; // up
    if ((dGor || dVer) && (submenuName(menuSettings.gorInd + dGor, menuSettings.verInd + dVer) != anon)) {
      menuSettings.gorInd += dGor;
      menuSettings.verInd += dVer;
      initSubmenu(submenuName(menuSettings.gorInd, menuSettings.verInd));
      menuDisplay();
    }
  }

  return ans;
}

void menuDisplay() {
  String sOut;
  currSettings.startEndDurations(0);
  for (auto& menuItem : menuSettings.subMenu) {
    if (menuItem == nullptr) break;
    sOut += menuItem->display();
  }
  int deltaLen = sOut.length() - 8;
  if (deltaLen > 0) sOut = sOut.substring(0, 8);
  while (deltaLen++ < 0) {
    sOut += ' ';
  }
  module.setDisplayToString(sOut, menuGetDots(submenuName(menuSettings.gorInd, menuSettings.verInd)));
  currSettings.startEndDurations(1);
}

submenu getSubmenu() {
  return submenuName(menuSettings.gorInd, menuSettings.verInd);
};

byte menuGetDots(submenu _submenu) {
  switch (_submenu) {
    case timeMenu: return currSettings.secondLed ? B00010000 : 0; break;
    case bubbleDaySpeed:
    case bubbleNightSpeed:
    case timer:
    case morning:
    case evening:
    case morningFeeding:
    case eveningFeeding:
    case alarm: return B00010000; break;
    case curTemp: return controlTemp.getAquaTempConnected() ? B00100010 : B00100000; break;
    case bubbleControlSettings: return B01000100; break;
    case logTemp: return B01000010; break;
    case nightFeeding: return B01000100; break;
    case deltaTemp: return B00001000; break;
    case dayFeedingSettings:
    case bubblesInSecond: return B01000000; break;
  }
  return 0;
}

submenu submenuName(byte _gorInd, byte _verInd) {

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
        case 0: return feedingMenu; break;
        case 1: return morningFeeding; break;
        case 2: return eveningFeeding; break;
        case 3: return dayFeedingSettings; break;
        case 4: return nightFeeding; break;
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

void initSubmenu(submenu _submenu) {

  for (auto& menuItem : menuSettings.subMenu) {
    if (menuItem != nullptr) {
      delete menuItem;
      menuItem = nullptr;
    }
  }

  switch (_submenu) {
    case timeMenu:
      menuSettings.subMenu[0] = new SettingsValue(&currSettings, dayNight);
      menuSettings.subMenu[1] = new TextItem(" ");
      menuSettings.subMenu[2] = new TimeValue(&currSettings, 0, &rtc);
      menuSettings.subMenu[3] = new TimeValue(&currSettings, 1, &rtc);
      menuSettings.subMenu[4] = new SettingsValue(&currSettings, timerOn);
      menuSettings.subMenu[5] = new AlarmFlag();
      break;
    case timer:
      menuSettings.subMenu[0] = new TextItem("St");
      menuSettings.subMenu[1] = new TimerValue(&currSettings, 0);
      menuSettings.subMenu[2] = new TimerValue(&currSettings, 1);
      menuSettings.subMenu[3] = new TimerStart(&currSettings);
      break;
    case morning:
      menuSettings.subMenu[0] = new TextItem("Sd");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_MORNING_HOUR, 0, 23, 2);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_MORNING_MINUTE, 0, 59, 2);
      break;
    case evening:
      menuSettings.subMenu[0] = new TextItem("Sn");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_EVENING_HOUR, 0, 23, 2);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_EVENING_MINUTE, 0, 59, 2);
      break;
    case alarm:
      menuSettings.subMenu[0] = new TextItem("Sb");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_ALARM_HOUR, 0, 23, 2);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_ALARM_MINUTE, 0, 59, 2);
      menuSettings.subMenu[3] = new byteEEPROMvalue(EEPROM_ALARM, 0, 1, 2, 1);
      break;
    case lampInterval:
      menuSettings.subMenu[0] = new TextItem("Sdn ");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_LAMP_INTERVAL, 0, 30, 2);
      break;
    case curTemp:
      menuSettings.subMenu[0] = new TextItem("i");
      menuSettings.subMenu[1] = new RtsTemp(&rtc);
      menuSettings.subMenu[2] = new TextItem("o");
      menuSettings.subMenu[3] = new AquaTemp(&controlTemp);
      break;
    case logTemp:
      menuSettings.subMenu[0] = new TempLog(&currSettings, &controlTemp);
      break;
    case dayTemp:
      menuSettings.subMenu[0] = new TextItem("Td  ");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_DAY_TEMP, 14, 30, 2);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_DAY_TEMP_ON, 0, 1, 2, 1);
      break;
    case nightTemp:
      menuSettings.subMenu[0] = new TextItem("Tn  ");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_NIGHT_TEMP, 14, 30, 2);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_NIGHT_TEMP_ON, 0, 1, 2, 1);
      break;
    case deltaTemp:
      menuSettings.subMenu[0] = new TextItem("dt  ");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_DELTA_TEMP, 5, 10, 2);
      break;
    case bubblesInSecond:
      menuSettings.subMenu[0] = new bubbleCounterValue(&bubbleCounter, bubbleIn100Second);
      menuSettings.subMenu[1] = new bubbleControlValue(&bubbleControl, controlCondition);
      break;
    case bubbleControlSettings:
      menuSettings.subMenu[0] = new bubbleControlValue(&bubbleControl, minBubblesIn100Second);
      menuSettings.subMenu[1] = new bubbleControlValue(&bubbleControl, maxBubblesIn100Second);
      break;
    case sensorValue:
      menuSettings.subMenu[0] = new bubbleCounterValue(&bubbleCounter, minLevel);
      menuSettings.subMenu[1] = new bubbleCounterValue(&bubbleCounter, maxLevel);
      break;
    case bubbleSettings:
      menuSettings.subMenu[0] = new TextItem("d ");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_MAX_DURATION_BUBBLE, 0, 50, 2, 1);
      menuSettings.subMenu[2] = new TextItem("h ");
      menuSettings.subMenu[3] = new byteEEPROMvalue(EEPROM_MIN_LEVEL_BUBBLE, 0, 50, 2, 1);
      break;
    case bubbleDaySpeed:
      menuSettings.subMenu[0] = new TextItem("Bd");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_DAY_BUBBLE_SPEED, 19, 250, 4, 1);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_DAY_BUBBLE_ON, 0, 1, 2, 1);
      break;
    case bubbleNightSpeed:
      menuSettings.subMenu[0] = new TextItem("Bn");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_NIGHT_BUBBLE_SPEED, 19, 250, 4, 1);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_NIGHT_BUBBLE_ON, 0, 1, 2, 1);
      break;
    case beforeMorningStart:
      menuSettings.subMenu[0] = new TextItem("bd");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_BEFORE_MORNING_BUBBLE_START, 0, 250, 4, 3);
      break;
    case bubbleControlSound:
      menuSettings.subMenu[0] = new TextItem("Sound");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_CONTROL_BUBBLE_SOUND_ON, 0, 1, 3, 2);
      break;
    case feedingMenu:
      menuSettings.subMenu[0] = new TextItem("Feed");
      menuSettings.subMenu[1] = new FeedingValue(&feeding);
      break;
    case morningFeeding:
      menuSettings.subMenu[0] = new TextItem("Fd");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_MORNING_FEEDING_HOUR, 0, 23, 2);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_MORNING_FEEDING_MINUTE, 0, 59, 2);
      menuSettings.subMenu[3] = new byteEEPROMvalue(EEPROM_MORNING_FEEDING_LOOP, 0, 20, 2, 1);
      break;
    case eveningFeeding:
      menuSettings.subMenu[0] = new TextItem("Fd");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_EVENING_FEEDING_HOUR, 0, 23, 2);
      menuSettings.subMenu[2] = new byteEEPROMvalue(EEPROM_EVENING_FEEDING_MINUTE, 0, 59, 2);
      menuSettings.subMenu[3] = new byteEEPROMvalue(EEPROM_EVENING_FEEDING_LOOP, 0, 20, 2, 1);
      break;
    case dayFeedingSettings:
      menuSettings.subMenu[0] = new TextItem("d");
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_DAY_FEEDING_DURATION, 0, 250, 3, 0);
      menuSettings.subMenu[2] = new TextItem("p");
      menuSettings.subMenu[3] = new byteEEPROMvalue(EEPROM_DAY_FEEDING_PAUSE, 0, 99, 3, 2, 10);
      break;
    case nightFeeding:
      menuSettings.subMenu[0] = new byteEEPROMvalue(EEPROM_NIGHT_FEEDING_HOUR, 0, 23, 2);
      menuSettings.subMenu[1] = new byteEEPROMvalue(EEPROM_NIGHT_FEEDING_MINUTE, 0, 23, 2);
      menuSettings.subMenu[2] = new TextItem("d");
      menuSettings.subMenu[3] = new byteEEPROMvalue(EEPROM_NIGHT_FEEDING_DURATION, 0, 250, 3, 0);
      break;
    case durations:
      menuSettings.subMenu[0] = new SettingsValue(&currSettings, dur);
      break;
    case motorPosition:
      menuSettings.subMenu[0] = new TextItem("POS ");
      menuSettings.subMenu[1] = new MotorPosition(&stepMotor);
      break;
    case motorSpeed:
      menuSettings.subMenu[0] = new TextItem("51");
      break;
    case bubbleDurations:
      menuSettings.subMenu[0] = new TextItem("52");
      break;
    case bubbleCount:
      menuSettings.subMenu[0] = new TextItem("53");
      break;
    case bubblesInMinute:
      menuSettings.subMenu[0] = new TextItem("54");
      break;
    case sensorInSecond:
      menuSettings.subMenu[0] = new TextItem("55");
      break;
    case errorsInSecond:
      menuSettings.subMenu[0] = new TextItem("56");
      break;
  }
}
