/*
  Timer.h - Library for displaying menu in TM1638.


*/
#pragma once

class Timer {

  public:
    Timer (byte _minute, byte _second);    
    void restart(byte _minute, byte _second);
    byte getMinute();
    byte getSecond();
    int loopNeedDisplay();

  private:
    byte minute, second;
    unsigned long nextSecondTime;
};

Timer::Timer(byte _minute, byte _second) {
  restart(_minute, _second);  
};

void Timer::restart(byte _minute, byte _second) {
  minute = _minute;
  second = _second;
  nextSecondTime = millis() + 1000;
};

byte Timer::getMinute() {
  return minute;
};

byte Timer::getSecond() {
  return second;
};

int Timer::loopNeedDisplay() {
  if (second == 0 && minute == 0) return -1;
  if (millis() < nextSecondTime) return 0;

  if (second == 0) {
    second = 59;
    minute--;
  }
  else {
    second--;
    if (second == 0 && minute == 0) return -1;
  }

  nextSecondTime += 1000;
  return 1;
}
