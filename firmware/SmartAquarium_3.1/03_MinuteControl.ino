void minuteControl() {

  // local values
  int _nowInMinutes = timeInMinutes(currSettings.nowHour, currSettings.nowMinute);
  int _morningInMinutes = timeInMinutes(EEPROM.read(EEPROM_MORNING_HOUR), EEPROM.read(EEPROM_MORNING_MINUTE));
  int _eveningInMinutes = timeInMinutes(EEPROM.read(EEPROM_EVENING_HOUR), EEPROM.read(EEPROM_EVENING_MINUTE));


  // it's day or nigth
  currSettings.nowDay = itsDay(_nowInMinutes, _morningInMinutes, _eveningInMinutes);

  // control alarm
  if (EEPROM.read(EEPROM_ALARM) == 1) {
    int _alarmInMinutes = timeInMinutes(EEPROM.read(EEPROM_ALARM_HOUR), EEPROM.read(EEPROM_ALARM_MINUTE));
    if (_nowInMinutes == _alarmInMinutes) {
      if (currSettings.alarmMelody == nullptr) currSettings.alarmMelody = new Melody();
      else currSettings.alarmMelody->restart();
    }
  }

  // change control bubble settings
  int _morningBubbles = _morningInMinutes - EEPROM.read(EEPROM_BEFORE_MORNING_BUBBLE_START);
  if (_morningBubbles < 0) _morningBubbles += 1440;
  int _eveningBubbles = _eveningInMinutes - EEPROM.read(EEPROM_LAMP_INTERVAL) * 2;
  if (_eveningBubbles < 0) _eveningBubbles += 1440;
  byte _needingBubbleSpeed;
  byte _needingStatus;
  if (itsDay(_nowInMinutes, _morningBubbles, _eveningBubbles)) {
    _needingBubbleSpeed = EEPROM.read(EEPROM_DAY_BUBBLE_SPEED);
    _needingStatus = EEPROM.read(EEPROM_DAY_BUBBLE_ON);
  }
  else {
    _needingBubbleSpeed = EEPROM.read(EEPROM_NIGHT_BUBBLE_SPEED);
    _needingStatus = EEPROM.read(EEPROM_NIGHT_BUBBLE_ON);
  }
  bubbleControl.set_currStatus(_needingStatus);
  bubbleControl.set_bubblesIn100Second(_needingBubbleSpeed);

  // control lamps
  lamps.scheduler();

  // control heater
  controlTemp.scheduler(currSettings.nowDay, currSettings.nowMinute, currSettings.nowHour);

  // control feeding
  feeding.scheduler(_nowInMinutes);

}
