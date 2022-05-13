#include <TM1638.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#include <microDS3231.h>
MicroDS3231 Rtc;

#define DISPLAY_INTERVAL 0
#define KEYBOARD_INTERVAL 10
#define PIEZO_PIN 8

struct {
  byte main;
  byte secondary;
  byte setting;
} currMode;

struct {
  DateTime now;  
} timeSettings;

void setup() {
  pinMode(PIEZO_PIN, OUTPUT); // настраиваем вывод 2 на выход
  tone(PIEZO_PIN, 200);
  delay(500);
  noTone(PIEZO_PIN);  
  //Serial.begin(9600);           //  setup serial
  //Serial.println(currMode.main); // отладка  
}

void loop() {

  loopTime();
  readKeyboard();  
  
}

void readKeyboard() {  

  
}

// общая функция изменения времени
void loopTime() {
  static unsigned long _lastLoopTime = 0; // последнее время обработки изменения времени
  byte _needDisplay = 0; // есть необходимость обновить дисплей

  if (_lastLoopTime == 0) {
    timeSettings.now = Rtc.getTime();
    _lastLoopTime  = millis();
    _needDisplay = 1;
  }
  
  if ((millis() - _lastLoopTime) > 1000) {
    // секунда оттикала
    _lastLoopTime  = millis();
    timeSettings.now.second++;
    if ((currMode.main == 0)&&(currMode.secondary == 0)) _needDisplay = 1;
    if (timeSettings.now.second == 60) {
      timeSettings.now.second = 0;
      timeSettings.now.minute++;
      if ((currMode.main == 0)&&(currMode.secondary == 0)) _needDisplay = 1;
      if (timeSettings.now.minute == 60) {
        // синхронизация раз в час
        timeSettings.now = Rtc.getTime();
      }      
    }
  }

  if (_needDisplay == 1) printDisplay();
    
}

// общая функция вывода дисплея
void printDisplay() {  
  
  switch (currMode.main) {
    case 0:
      printDisplay0_Time();
      break;
    case 1:
      break;
    default:
      break;
  }  
}

// функция вывода дисплея в режиме 0: вывод времени
void printDisplay0_Time() {  
  char s[8];
  
  switch (currMode.secondary) {
    case 0:
      sprintf(s, "  %02d%02d%02d", timeSettings.now.hour % 99, timeSettings.now.minute % 99, timeSettings.now.second % 99);
      Module.setDisplayToString(s, 0, false);
      break;
    case 1:
      break;
    default:
      break;
    }
}
