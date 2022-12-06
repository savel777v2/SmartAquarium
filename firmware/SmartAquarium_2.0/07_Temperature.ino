
// Initialize heater pins
void heaterInitialize() {
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, HIGH); // false - off  
}

// turn on or turn off heater depending on the aqua temperature and settings of it
void heaterOnOff() {
  byte needingTemp;
  bool heaterAlwaysOff = false;

  if (currSettings.nowDay) {
    needingTemp = EEPROM.read(11);
    if (EEPROM.read(12) == 0) heaterAlwaysOff = true;
  }
  else {
    needingTemp = EEPROM.read(13);
    if (EEPROM.read(14) == 0) heaterAlwaysOff = true;
  }

  byte deltaTemp = EEPROM.read(15);
  float minTemp = needingTemp - (float)deltaTemp / 10;
  float maxTemp = needingTemp + (float)deltaTemp / 10;
  if (currSettings.heaterOn && (heaterAlwaysOff || currSettings.aquaTempStatus != 2 || currSettings.aquaTemp >= maxTemp)) {
    currSettings.heaterOn = false;
    Module.setLED(0, 4);
    digitalWrite(HEATER_PIN, HIGH);
  }
  else if (!currSettings.heaterOn && !heaterAlwaysOff && currSettings.aquaTempStatus == 2 && currSettings.aquaTemp <= minTemp) {
    currSettings.heaterOn = true;
    Module.setLED(1, 4);
    digitalWrite(HEATER_PIN, LOW);
  }

  // heaterTempLog each our
  if (currSettings.now.minute == 0) {

    byte _indexOfLog = currSettings.now.hour;
    word _logValue;
    if (currSettings.aquaTempStatus != 2) _logValue = 0;
    else _logValue = (float)currSettings.aquaTemp * 10 + 1000;
    if (currSettings.heaterOn) _logValue = _logValue + 10000;
    heaterTempLog[_indexOfLog] = _logValue;

  }

}

// read temperature from sensor in asynchronous mode
// ATTENTION read once getTempC() is longer then 8 ms
bool readTemperatureNeedDisplay() {
  static unsigned long _lastTime = 0;
  static byte _countWaiting = 10;

  bool _needDisplay = false;
  unsigned long _currentTime = millis();

  switch (_countWaiting) {
    case 0:
      // request temperature once in minute
      if ((_currentTime - _lastTime) > 60000) {
        _lastTime = _currentTime;
        sensor.requestTemperatures();
        _countWaiting = 1;
      }
      break;
    case 10:
      // waiting minute after error
      if (_lastTime == 0 || (_currentTime - _lastTime) > 60000) {
        if (currSettings.aquaTempStatus == 0) {
          if (sensor.begin()) currSettings.aquaTempStatus = 1; // read temp
        }
        if (currSettings.aquaTempStatus == 1) {
          sensor.requestTemperatures();
          _countWaiting = 1;
        }
        _lastTime = _currentTime;
      }
      break;
    default:
      // try read Temperature each second
      if ((_currentTime - _lastTime) > 1000) {
        _lastTime = _currentTime;
        if (sensor.isConversionComplete()) {
          currSettings.aquaTempStatus = 2; // normal
          currSettings.aquaTemp = sensor.getTempC();
          _countWaiting = 0;
          if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
        }
        else if (++_countWaiting == 10) {
          currSettings.aquaTempStatus = 1; // read temp
          if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
        }
      }
      break;
  }

  return _needDisplay;
}
