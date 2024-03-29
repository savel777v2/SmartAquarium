/*
  Lamps.h - Library for displaying menu in TM1638.


*/
#pragma once

// Lamps from left to right
#define LAMP1_PIN A1
#define LAMP2_PIN A2
#define LAMP3_PIN A3
#define MANUAL_LAMP_DURATION 600000 // at least on change 10 minutes

class Lamps {

  public:
    Lamps(CurrSettings* _currSettings);
    void scheduler();
    void changeManualLamp();

  private:
    CurrSettings* currSettings;
    byte manualLamp;
    unsigned long manualLampTimeOff;
};

Lamps::Lamps(CurrSettings* _currSettings) {
  pinMode(LAMP1_PIN, OUTPUT);
  pinMode(LAMP2_PIN, OUTPUT);
  pinMode(LAMP3_PIN, OUTPUT);
  currSettings = _currSettings;
  manualLamp = 0; // 0 - not, 1 - on, 2 - off
  manualLampTimeOff = 0;
};

void Lamps::scheduler() {

  byte lampPinsLevel[3];

  // turn off manual lamp
  if (manualLamp > 0 && millis() >= manualLampTimeOff) manualLamp = 0;

  // tone for turn off manual lamp
  if (manualLamp == 0 && manualLampTimeOff > 0) {
    manualLampTimeOff = 0;
    tone(PIEZO_PIN, 2500, 100);
  }

  if (manualLamp == 2 || !currSettings->nowDay) {
    for (int i = 0; i < 3; i++) lampPinsLevel[i] = 0;
  }
  else if (manualLamp == 1) {
    for (int i = 0; i < 3; i++) lampPinsLevel[i] = 1;
  }
  else {
    int minutesBetweenLamps = EEPROM.read(EEPROM_LAMP_INTERVAL);
    int nowInMinutes = timeInMinutes(currSettings->nowHour, currSettings->nowMinute);
    int morningInMinutes = timeInMinutes(EEPROM.read(EEPROM_MORNING_HOUR), EEPROM.read(EEPROM_MORNING_MINUTE));
    int eveningInMinutes = timeInMinutes(EEPROM.read(EEPROM_EVENING_HOUR), EEPROM.read(EEPROM_EVENING_MINUTE));
    for (int i = 0; i < 3; i++) {
      // after morning
      int minutesLamp = morningInMinutes + minutesBetweenLamps * i;
      lampPinsLevel[i] = 0;
      if (nowInMinutes >= morningInMinutes) {
        if (nowInMinutes >= minutesLamp) lampPinsLevel[i] = 1;
      }
      else if (minutesLamp >= 1440) {
        minutesLamp = minutesLamp - 1440;
        if (nowInMinutes >= minutesLamp) lampPinsLevel[i] = 1;
      }
      else lampPinsLevel[i] = 1;
      // before evening
      minutesLamp = eveningInMinutes + minutesBetweenLamps * (i - 2);
      if (nowInMinutes < eveningInMinutes) {
        if (nowInMinutes >= minutesLamp) lampPinsLevel[i] = 0;
      }
      else if (minutesLamp < 0) {
        minutesLamp = minutesLamp + 1440;
        if (nowInMinutes >= minutesLamp) lampPinsLevel[i] = 0;
      }
    }
  }

  digitalWrite(LAMP1_PIN, lampPinsLevel[0] ? LOW : HIGH);
  digitalWrite(LAMP2_PIN, lampPinsLevel[1] ? LOW : HIGH);
  digitalWrite(LAMP3_PIN, lampPinsLevel[2] ? LOW : HIGH);

}

void Lamps::changeManualLamp() {

  if (manualLamp == 0) manualLamp = (currSettings->nowDay ? 2 : 1);
  else if (manualLamp == (currSettings->nowDay ? 2 : 1)) manualLamp = (manualLamp == 2 ? 1 : 2);
  else manualLamp = 0;

  manualLampTimeOff = millis() + MANUAL_LAMP_DURATION;
  scheduler();

}
