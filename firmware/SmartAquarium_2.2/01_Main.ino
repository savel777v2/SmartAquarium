MenuItemPart EditingMenuItemPart;

void setup() {

#if (DEBUG_MODE == 1)
  Serial.begin(9600);
  Serial.println("debugging");
#endif

  Module.setDisplayToString("AQUA  22", B00000010, false);
  delay(2000);

  alarmInitialize();

  lampsInitialize();

  heaterInitialize();

  eatingInitialize();

  bubbleCounterInitialize();

  initPartsOfMenuItem(menuItems[currMode.main][currMode.secondary]);
  EditingMenuItemPart.set_isNull(1);

}

void loop() {
  bool _needDisplay = false;

  _needDisplay = _needDisplay || loopTimeNeedDisplay();
  CounterForBubbles.tick();

  _needDisplay = _needDisplay || readTemperatureNeedDisplay();
  CounterForBubbles.tick();

  _needDisplay = _needDisplay || eatingLoopNeedDisplay();
  CounterForBubbles.tick();

  _needDisplay = _needDisplay || timerLoopNeedDisplay();
  CounterForBubbles.tick();

  if (_needDisplay) {
    printDisplay();
    CounterForBubbles.tick();
  }

  alarmLoop();
  CounterForBubbles.tick();

  readKeyboard();
  CounterForBubbles.tick();

  StepMotorBubbles.tick();
  CounterForBubbles.tick();
}

// GENERAL FUNCTIONS
// return it's day or nigth based on Morning and Evening
bool itsDay(int _nowInMinutes, int _morningInMinutes, int _eveningInMinutes) {
  if (_eveningInMinutes > _morningInMinutes) return (_nowInMinutes >= _morningInMinutes && _nowInMinutes < _eveningInMinutes);
  else return (_nowInMinutes >= _morningInMinutes || _nowInMinutes < _eveningInMinutes);
}

// all options control every minute
void minuteControl() {

  // general values
  int _nowInMinutes = (int)currSettings.now.hour * 60 + currSettings.now.minute;
  int _morningInMinutes = (int)EEPROM.read(0) * 60 + EEPROM.read(1);
  int _eveningInMinutes = (int)EEPROM.read(2) * 60 + EEPROM.read(3);

  // it's day or nigth
  currSettings.nowDay = itsDay(_nowInMinutes, _morningInMinutes, _eveningInMinutes);

  // starting eating loop
  controlEatingLoop(EEPROM.read(33), _nowInMinutes, EEPROM.read(31), EEPROM.read(32));
  controlEatingLoop(EEPROM.read(36), _nowInMinutes, EEPROM.read(34), EEPROM.read(35));

  // settings of a bubble speed
  controlBubbleSpeed(_nowInMinutes, _morningInMinutes, _eveningInMinutes);

  // starting alarm
  controlAlarm(_nowInMinutes);

  // control off heater
  controlHeaterOnOff();

  // turn on, turn off lamps
  controlLamps(_nowInMinutes, _morningInMinutes, _eveningInMinutes);

}
// END OF GENERAL FUNCTIONS

// main procedure for control time
bool loopTimeNeedDisplay() {
  static unsigned long _lastLoopTime = 0;

  bool _needDisplay = false;

  // Loop once First time
  if (_lastLoopTime == 0) {
    currSettings.now = Rtc.getTime();
    minuteControl();
    _lastLoopTime  = millis();
    _needDisplay = true;
  }

  // Loop blink Settings
  if (currSettings.setting > 0) {
    if ((millis() - settMode.lastBlinkTime) > 500) {
      settMode.lastBlinkTime  = millis();
      settMode.blinkOff = !settMode.blinkOff;
      _needDisplay = true;
    }
  }

  // Loop increment local time
  if ((millis() - _lastLoopTime) > 1000) {

    // print duration
    durations.printMax1 = durations.max1;
    durations.printMax2 = durations.max2;
    durations.printMax3 = durations.max3;
    durations.printMax4 = durations.max4;
    durations.max1 = 0;
    durations.max2 = 0;
    durations.max3 = 0;
    durations.max4 = 0;
    if ((currMode.main == 3) && (currMode.secondary == 4)) _needDisplay = true;

    // секунда оттикала
    _lastLoopTime  = millis();
    currSettings.now.second++;
    if (currSettings.now.second == 60) {
      currSettings.now.second = 0;
      currSettings.now.minute++;
      if ((currMode.main == 0) && (currMode.secondary == 0)) _needDisplay = true;
      if (currSettings.now.minute == 60) {
        // синхронизация раз в час
        currSettings.now = Rtc.getTime();
      }
      minuteControl();
    }
  }

  return _needDisplay;

}
