/*
  Melody.h - Library for displaying menu in TM1638.


*/
#pragma once

#define MELODY_DURATION 60000
#define PIEZO_PIN 3 // + ground

class Melody {

  public:
    Melody();
    ~Melody();
    void restart();
    bool loopNeedLoop();

  private:
    byte note;
    unsigned long nextNoteTime, melodyEndTime;
    unsigned int notes[10][2] = {{2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 3000}};
};

Melody::Melody () {
  restart();
};

Melody::~Melody () {
  noTone(PIEZO_PIN);
};

void Melody::restart() {
  note = 0;
  nextNoteTime = millis();
  melodyEndTime = millis() + MELODY_DURATION;
};

bool Melody::loopNeedLoop() {

  if (millis() > melodyEndTime) return false;
  
  if (millis() < nextNoteTime) return true;

  if (notes[note][0] == 0) noTone(PIEZO_PIN);
  else tone(PIEZO_PIN, notes[note][0]);

  nextNoteTime += notes[note][1];
  if (++note == sizeof(notes) / sizeof(notes[0])) note = 0;

  return true;
    
}
