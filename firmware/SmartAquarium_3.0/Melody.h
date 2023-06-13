/*
  Melody.h - Library for displaying menu in TM1638.


*/
#pragma once

class Melody {

  public:
    Melody (int _piezo_pin);
    ~Melody();
    unsigned long getMelodyStartTime();
    void loop();

  private:
    int piezo_pin;
    byte note;
    unsigned long melodyStartTime;
    unsigned long nextNoteTime;
    unsigned int notes[10][2] = {{2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 100}, {2500, 50}, {0, 3000}};
};

Melody::Melody (int _piezo_pin) {
  piezo_pin = _piezo_pin;
  note = 0;
  nextNoteTime = 0;
  melodyStartTime = millis();
  pinMode(piezo_pin, OUTPUT);
};

unsigned long Melody::getMelodyStartTime() {
  return melodyStartTime;
};

Melody::~Melody () {
  noTone(piezo_pin);
};

void Melody::loop() {
  if (millis() < nextNoteTime) return;

  if (nextNoteTime == 0) melodyStartTime = millis();

  if (notes[note][0] == 0) noTone(piezo_pin);
  else tone(piezo_pin, notes[note][0]);

  nextNoteTime = millis() + notes[note][1];
  if (++note == sizeof(notes) / sizeof(notes[0])) note = 0;
}
