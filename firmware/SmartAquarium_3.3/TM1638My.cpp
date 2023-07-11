/*
  TM1638My.cpp - Программная реализация управления модулем Дисплея и клавиатуры TM1638
*/

#include "TM1638My.h"

TM1638My::TM1638My(int _DIO, int _CLK, int _STB) : TM1638(_DIO, _CLK, _STB) {  
  for (auto& keyTime : keyTimePressed) keyTime = 0;
  for (auto& count : countPressed) count = 0;
};

boolean TM1638My::keyPressed(const byte _key, const byte _keys) {
  byte keyFlag = 1 << _key;
  return (_keys & keyFlag) == keyFlag;
};

// Функция анализа нажатия клавиш модуля
//  _keys - маска анализируемых клавиш, например B11110000 - первые четыре
//  _mode - режим анализируемых клавиш, где разряд для клавиши это режим:
//    0 - обычное нажатие
//    1 - режим удержания более 1 сек - быстрыее, 5 сек - еще быстрее
//
byte TM1638My::keysPressed(const byte _keys, const byte _mode) {

  byte ans = 0; 
  byte curKeys = getButtons();
  if (curKeys == 255) return ans;
    
  for (int ind = 0; ind < 8; ind++) {
    byte keyFlag = 1 << ind;
    if ((_keys & keyFlag) != keyFlag) continue;
    if ((curKeys & keyFlag) == keyFlag) {
      if (keyTimePressed[ind] == 0) {
        keyTimePressed[ind] = millis();
        ans |= keyFlag;
      }
      else if ((_mode & keyFlag) == keyFlag) {
        unsigned long _timeLeft = millis() - keyTimePressed[ind];
        byte _countPressed = 0;
        if (_timeLeft > 5000) {
          _timeLeft = _timeLeft - 5000;
          _countPressed = 25 + _timeLeft * 0.01;
        }
        else if (_timeLeft > 1000) {
          _timeLeft = _timeLeft - 1000;
          _countPressed = _timeLeft * 0.005;
        }
        if (_countPressed != countPressed[ind]) {
          countPressed[ind] = _countPressed;
          ans |= keyFlag;
        }
      }
    }
    else {
      keyTimePressed[ind] = 0;
      countPressed[ind] = 0;
    }
  }

  return ans;

};
