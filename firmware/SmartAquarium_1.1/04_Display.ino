// общая функция вывода дисплея
void printDisplay() {
  char _toDisplay[9];
  MenuItemPart _PrintMenuItemPart;

  _toDisplay[0] = '\0';

  int _indexSetting = 0;
  for (int _i = 0; _i < 8; _i++) if (partsOfMenuItem[_i][0] != '\0') {
      _PrintMenuItemPart.initialize(partsOfMenuItem[_i], &currSettings);
      bool _findSetting = false;
      if (_PrintMenuItemPart.get_edited()) {
        _indexSetting++;
        if (currSettings.setting == _indexSetting) _findSetting = true;
      }
      if (!_findSetting) _PrintMenuItemPart.readValue(&currSettings);
      else _PrintMenuItemPart.set_value(EditingMenuItemPart.get_value());
      _PrintMenuItemPart.valueToDisplay(_toDisplay, &currSettings, _findSetting && settMode.blinkOff);

    }

  Module.setDisplayToString(_toDisplay, 0, false);

}
