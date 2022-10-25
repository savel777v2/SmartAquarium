#if (DEBUG_MODE == 1)

void debugTemp() {
  Serial.print("aquaTemp: ");
  Serial.print(currSettings.aquaTemp);
  Serial.print(",aquaTempErr: ");
  Serial.println(currSettings.aquaTempErr);
}

void debugEditingMenuItemPart() {
  Serial.print(EditingMenuItemPart.get_typeOfPart());
  Serial.print(",isNull: ");
  Serial.print(EditingMenuItemPart.get_isNull());
  Serial.print(",value: ");
  Serial.println(EditingMenuItemPart.get_value());
}

void debugManualLamp() {
  Serial.print("manualLamp: ");
  Serial.print(currSettings.manualLamp);
  Serial.print(",nowDay: ");
  Serial.println(currSettings.nowDay);
}

void debugOnOffLamps(char _char[10], int _i, int _nowInMinutes, int _minutesLamp) {
  Serial.print(_char);
  Serial.print(" _i: ");
  Serial.print(_i);
  Serial.print(" _nowInMinutes: ");
  Serial.print(_nowInMinutes);
  Serial.print(" _minutesLamp: ");
  Serial.println(_minutesLamp);
}

void debugOnOffLamps(char _char[10], int _i, int _nowInMinutes, int _minutesLamp) {
  Serial.print(_char);
  Serial.print(" _i: ");
  Serial.print(_i);
  Serial.print(" _nowInMinutes: ");
  Serial.print(_nowInMinutes);
  Serial.print(" _minutesLamp: ");
  Serial.println(_minutesLamp);
}

#endif
