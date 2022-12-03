
// read temperature from sensor in asynchronous mode
// ATTENTION read once getTempC() is longer then 8 ms
bool readTemperatureNeedDisplay() {
  static unsigned long _lastTime = 0;
  static byte _countWaiting = 0;

  bool _needDisplay = false;
  unsigned long _currentTime = millis();

  switch (_countWaiting) {
    case 0:
      // request temperature once in minute
      if (_lastTime == 0 || (_currentTime - _lastTime) > 60000) {
        _lastTime = _currentTime;
        sensor.requestTemperatures();
        _countWaiting = 1;
      }
      break;
    case 10:
      // waiting second after error
      if ((_currentTime - _lastTime) > 60000) {
        _lastTime = _currentTime;
        _countWaiting = 1;
      }
      break;
    default:
      // try read Temperature each second
      if ((_currentTime - _lastTime) > 1000) {
        _lastTime = _currentTime;
        if (sensor.isConversionComplete()) {
          currSettings.aquaTempErr = false;
          currSettings.aquaTemp = sensor.getTempC();
          _countWaiting = 0;
          if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
        }
        else if (++_countWaiting == 10) {
          currSettings.aquaTempErr = true;
          if (currMode.main == 1 && currMode.secondary == 0) _needDisplay = true;
        }
      }
      break;
  }

  return _needDisplay;
}
