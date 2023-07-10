/*
  Timer.h - Объект для отсчета таймера


*/

#ifndef Timer_h
#define Timer_h

#include <Arduino.h>

class Timer {

  public:  
    Timer (byte _minute, byte _second);    
    void restart(byte _minute, byte _second);
    byte getMinute();
    byte getSecond();
    // отсчитывает таймер, возвращает -1 если таймер кончился и больше не требуется,
    // 0 - если таймер не изменился, 1 - отсчитал секунду и нужно обновить внешние данные
    // к примеру, меню
    int loopNeedDisplay();

  private:
    byte minute, second;
    unsigned long nextSecondTime;
};

#endif
