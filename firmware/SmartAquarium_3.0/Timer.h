/*
  Timer.h - Library for displaying menu in TM1638.


*/
#pragma once

enum status {
  on, off, display
};

class Timer {

  public:
    Timer (byte _minute, byte _second);
    status loop();
    void restart(byte _minute, byte _second);
    byte getMinute();
    byte getSecond();

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

status Timer::loop() {
  if (second == 0 && minute == 0) return off;
  if (millis() < nextSecondTime) return on;

  if (second == 0) {
    second = 59;
    minute--;
  }
  else {
    second--;
    if (second == 0 && minute == 0) return off;
  }

  nextSecondTime += 1000;
  return display;
}
