/*
  Melody.h - Объект для управления PIEZO_PIN в целях воспроизведения мелодии
*/

#ifndef Melody_h
#define Melody_h

#include <Arduino.h>

#define MELODY_DURATION 60000 // общая продолжительность мелодии в мс, играет по нотам циклично
#define PIEZO_PIN 3 // вывод на микросхеме, подтягивать к ground

class Melody {

  public:
    Melody();
    ~Melody();
    // возобновляет мелодию с текущего момента,
    void restart();
    // проигрывает мелодию, возвращает true пока она идет, иначе false
    bool loopNeedLoop();

  protected:
    // частота ноты (пауза - 0), продолжительность ноты в мс.
    const unsigned int notes[10][2] = {{2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 3000}};

  private:
    byte note;
    unsigned long nextNoteTime, melodyEndTime;
};

#endif
