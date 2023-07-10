/*
  Global.cpp - Программная реализация глобальных определний и функций
*/

#include "Global.h"

int global::timeInMinutes(const int _hour, const int _minutes) {
  return _hour * 60 + _minutes;
};

String global::valToString(const int val, const byte len, byte leadingSpaces = 0) {
  String ans(val);
  int lenPref = len - ans.length();
  if (lenPref < 0) return ans.substring(-lenPref);
  String pref = "";
  while (lenPref-- > 0) {
    if (leadingSpaces > 0) {
      leadingSpaces--;
      pref += ' ';
    }
    else pref += '0';
  }
  return pref + ans;
};
