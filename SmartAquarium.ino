#include <TM1638.h>
TM1638 Module(4, 5, 6); // DIO, CLK, STB

#define DISPLAY_INTERVAL 100
#define KEYBOARD_INTERVAL 10
#define PIEZO_PIN 8
byte mainMode;
byte secondaryMode;
byte settingMode;

void setup() {
  pinMode(PIEZO_PIN, OUTPUT); // настраиваем вывод 2 на выход
  tone(PIEZO_PIN, 200);
  delay(500);
  noTone(PIEZO_PIN);
}

void loop() {

  readKeyboard();
  printDisplay();
  
}

void readKeyboard() {  

  
}

// общая функция вывода дисплея
void printDisplay() {  
  static unsigned long lastDisplay = 0; // последнее время вывода дисплея  

  if ((millis() - lastDisplay) > DISPLAY_INTERVAL) {
    lastDisplay = millis();
    switch (mainMode) {
      case 0:
        printDisplay0_Time();
        break;
      case 1:
        break;
      default:
        break;
    }
  }
}

// функция вывода дисплея в режиме 0: вывод времени
void printDisplay0_Time() {  
  char s[8];
  
  switch (mainMode) {
    case 0:
      sprintf(s, "  %2d%2d  ", mainMode % 99, secondaryMode % 99);
      Module.setDisplayToString(s, 0, false);
      break;
    case 1:
      break;
    default:
      break;
    }
}
