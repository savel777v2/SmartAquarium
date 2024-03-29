/*
  TM1638My.h - Объект для управления модулем Дисплея и клавиатуры TM1638
    наследник библиотечного модлуя TM1638.h.
    Дополняет реализацию функциями нажатия клавиш с учетом их удержания (т.е. факт начала нажатия
    а не "нажата\не нажата" в текущий момент в стандартной библиотеке)
*/

#ifndef TM1638My_h
#define TM1638My_h

#include <Arduino.h>
#include <TM1638.h>

class TM1638My : public TM1638 {

  public:
    TM1638My(int _DIO, int _CLK, int _STB);
    // Возвращает побитово какие клавиши нажаты, где:
    // _keys - побитовая маска анализа клавиш (не все могут анализироваться)
    // _mode - побитовый режим анализа клавиш (0 - без удержания, 1 - с удержанием)
    byte keysPressed(const byte _keys, const byte _mode);
    // Вычленяет конкретную клавишу из побитовой что нажато
    // _key - номер клавиши (бита)
    // _keys - побитово, что нажато (из keysPressed)
    boolean keyPressed(const byte _key, const byte _keys);

  private:
    // кэш времени последнего нажатия
    unsigned long keyTimePressed[8];
    // счетчики удержания нажатия (для реализации повторения нажатия при удержаниях)
    byte countPressed[8];

};

#endif
