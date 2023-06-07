#include <TM1638.h>
#include "MENU.h"
#include "CurrSettings.h"

enum tempStatus {notBegin, readTemp, normal};

int main() {

  TM1638 Module(4, 5, 6); // DIO, CLK, STB
  CurrSettings currSettings;
  currSettings.nowDay = false;
  currSettings.now.hour = 18;
  currSettings.now.minute = 25;
  Menu Menu(&Module, &currSettings);
  Menu.display();
}


/*void setup() {
  // put your setup code here, to run once:
  Module.setDisplayToString("Check");

  }

  void loop() {
  // put your main code here, to run repeatedly:

  }*/
