/*
  MenuItem.h - Реализация элементов меню для меню
*/

#ifndef MenuItem_h
#define MenuItem_h

#include <EEPROM.h>
#include <Arduino.h>
#include <microDS3231.h>
extern MicroDS3231 globDS3231;

#include "Global.h"
extern global::CurrSettings globCurrSettings;

#include "TM1638My.h"
extern TM1638My globModule1638;

#include "Feeding.h"
extern Feeding globFeeding;

#include "BubbleControl.h"
extern BubbleControl globBubbleControl;

#include "BubbleCounter.h"
extern BubbleCounter globBubbleCounter;

#include "ControlTemp.h"
extern ControlTemp globControlTemp;

#include "StepMotor.h"
extern StepMotor globStepMotor;

enum typeBubbleCounterValue
{
  bubbleIn100Second, minLevel, maxLevel
};

enum typeBubbleControlValue
{
  controlCondition, minBubblesIn100Second, maxBubblesIn100Second
};

enum typeSettingsValue
{
  dayNight, timerOn, dur
};

// реализация объекта родоначальника
class MenuItem {

  public:
    MenuItem();
    virtual ~MenuItem();
    virtual String display() = 0;
    virtual boolean editing();
    virtual void enterEditing();
    virtual void downValue();
    virtual void upValue();
    virtual void exitEditing();
    virtual void saveEditing();
    void changeBlink();

  protected:
    struct CurrMode {
      boolean editing: 1;
      boolean blinkOn: 1;
    } currMode;

    String emptyString(int len);
    void changeValue(byte& editValue, const int delta, const byte minValue, const byte maxValue, boolean circleEdit = true);
};

// текстовый элемент меню - тупо текст выводит
class TextItem: public MenuItem {

  public:
    TextItem (const char* _s);
    String display();

  private:
    char s[9];
};

// выводит настройку из currSettings
class SettingsValue: public MenuItem {

  public:
    SettingsValue (const typeSettingsValue _valueType);
    String display();
  private:
    typeSettingsValue valueType;

};

// выводит флаг будильника из EEPROM
class AlarmFlag: public MenuItem {

  public:
    String display();

};

// выводит текущее время часы или минуты, редактирует и сохраняет его в MicroDS3231
class TimeValue: public MenuItem {

  public:
    TimeValue (const byte _valueIndex);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    byte editValue;
    byte valueIndex;

};

// универсально выводит значение из EEPROM, редактирует и сохраняет его в EEPROM
class byteEEPROMvalue: public MenuItem {

  public:
    byteEEPROMvalue (const int _adressEEPROM, const byte _minValue, const byte _maxValue, const byte _len, const byte _leadingSpaces = 0, const byte _multiplier = 1);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    byte minValue, maxValue;
  protected:
    int adressEEPROM;
    byte editValue;
    byte len, leadingSpaces;
    byte multiplier;
};

// наследник от byteEEPROMvalue, дополнительно передает значение в шаговый мотор для его движения
class MotorPosition: public byteEEPROMvalue {

  public:
    MotorPosition ();
    String display();
    void saveEditing();
    
};

// выводит текущие минуты или секунды таймера, если он тикает,
// иначе выводит и редактирует параметры по умолчанию таймера и сохраняет их в EEPROM
class TimerValue: public MenuItem {

  public:
    TimerValue (const byte _valueIndex);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    bool valueIndex;
    byte editValue;

};

// выводит и редактирует текущее состояние таймера - если меняем то перезапускаем или запускаем таймер
class TimerStart: public MenuItem {

  public:
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    byte editValue;

};

// выводит текущую температуру модуля MicroDS3231
class RtsTemp: public MenuItem {

  public:
    String display();

};

// выводит текущую температуру ворды аквариума через объект ControlTemp
class AquaTemp: public MenuItem {

  public:
    String display();

};

// выводит логи температуру воды аквариума через объект ControlTemp
// в режиме редактирования - просматриваем историю логов
class TempLog: public MenuItem {

  public:
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
  private:
    byte editValue;
    String logToString(byte _index);

};

// выводит заданный показатель счетчика CO2 через объект BubbleCounter
class bubbleCounterValue: public MenuItem {

  public:
    bubbleCounterValue(const typeBubbleCounterValue _valueType);
    String display();
  private:
    typeBubbleCounterValue valueType;

};

// выводит заданный показатель управления расходом CO2 через объект BubbleControl
class bubbleControlValue: public MenuItem {

  public:
    bubbleControlValue (const typeBubbleControlValue _valueType);
    String display();
  private:
    typeBubbleControlValue valueType;

};

// выводит редактирует счетчик кормления через объект Feeding
class FeedingValue: public MenuItem {

  public:
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    byte editValue;

};

#endif
