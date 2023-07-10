// return it's day or nigth based on Morning and Evening
bool itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes) {
  if (_eveningInMinutes > _morningInMinutes) return (_nowInMinutes >= _morningInMinutes && _nowInMinutes < _eveningInMinutes);
  else return (_nowInMinutes >= _morningInMinutes || _nowInMinutes < _eveningInMinutes);
}

void readKeyboard() {
  static unsigned long nextKeyboardTime;

  if (millis() <= nextKeyboardTime) return false;

  nextKeyboardTime = millis() + KEYBOARD_INTERVAL;
  byte keys = module.keysPressed(B10000000, B00000000);
  if (module.keyPressed(7, keys)) {
    // manualLamp
    lamps.changeManualLamp();
  }
}

void loopTime() {
  static unsigned long nextSecondTime;  

  // Loop once First time
  if (nextSecondTime == 0) {
    nextSecondTime = millis() + 1000;
    currSettings.nowSecond = rtc.getSeconds();
    currSettings.nowMinute = rtc.getMinutes();
    currSettings.nowHour = rtc.getHours();
    minuteControl();
    menuDisplay();
  }  

  // turn off second Led
  if (currSettings.secondLed && millis() > (nextSecondTime - SECOND_NOLED_DURATION)) {
    currSettings.secondLed = false;
    menuDisplay();
  }

  // Loop increment local time
  if (millis() > nextSecondTime) {

    // секунда оттикала
    nextSecondTime += 1000;
    if (getSubmenu() == timeMenu) {
      currSettings.secondLed = true;
      menuDisplay();
    }

    if (getSubmenu() == durations) {
      // print durations
      for (int i = 0; i < DURATIONS_SIZE; i++) {
        currSettings.printDurations[i] = currSettings.curDurations[i];
        currSettings.curDurations[i] = 0;
      }
      menuDisplay();
    }

    currSettings.nowSecond++;
    if (currSettings.nowSecond == 60) {
      currSettings.nowSecond = 0;
      currSettings.nowMinute++;
      if (currSettings.nowMinute == 60) {
        currSettings.nowMinute = 0;
        currSettings.nowHour++;
        // синхронизация времени раз в час
        currSettings.nowSecond = rtc.getSeconds();
        currSettings.nowMinute = rtc.getMinutes();
        currSettings.nowHour = rtc.getHours();
      }
      minuteControl();
    }
  }
}
