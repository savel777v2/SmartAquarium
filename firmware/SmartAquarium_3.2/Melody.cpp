/*
  Melody.cpp - Программная реализация модуля мелодии
*/

#include "Arduino.h"
#include "Melody.h"

Melody::Melody() {
  restart();
};

Melody::~Melody() {
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
