/*
  MenuItem.h - Реализация элементов меню для меню
*/

#ifndef MenuItem_h
#define MenuItem_h

#include <EEPROM.h>
#include <Arduino.h>
#include <microDS3231.h>
#include "Global.h"
#include "TM1638My.h"
#include "Feeding.h"
#include "BubbleControl.h"
#include "BubbleCounter.h"
#include "ControlTemp.h"
#include "StepMotor.h"

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
  dayNight, timerOn
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
    SettingsValue (const global::CurrSettings* _currSettings, const typeSettingsValue _valueType);
    String display();
  private:
    typeSettingsValue valueType;
    global::CurrSettings* currSettings;

};

// выводит флаг будильника из EEPROM
class AlarmFlag: public MenuItem {

  public:
    String display();

};

// выводит текущее время часы или минуты, редактирует и сохраняет его в MicroDS3231
class TimeValue: public MenuItem {

  public:
    TimeValue (const global::CurrSettings* _currSettings, const byte _valueIndex, const MicroDS3231* _rtc);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    byte editValue;
    byte valueIndex;
    global::CurrSettings* currSettings;
    MicroDS3231* rtc;

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
    //MotorPosition (const StepMotor* _stepMotor) : byteEEPROMvalue (EEPROM_MOTOR_POSITION, 0, 250, 4, 3);
    MotorPosition (const StepMotor* _stepMotor);
    String display();
    void saveEditing();
  private:
    StepMotor * stepMotor;

};

// выводит текущие минуты или секунды таймера, если он тикает,
// иначе выводит и редактирует параметры по умолчанию таймера и сохраняет их в EEPROM
class TimerValue: public MenuItem {

  public:
    TimerValue (const global::CurrSettings * _currSettings, const byte _valueIndex);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    global::CurrSettings * currSettings;
    bool valueIndex;
    byte editValue;

};

// выводит и редактирует текущее состояние таймера - если меняем то перезапускаем или запускаем таймер
class TimerStart: public MenuItem {

  public:
    TimerStart (const global::CurrSettings * _currSettings);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    byte editValue;
    global::CurrSettings * currSettings;

};

// выводит текущую температуру модуля MicroDS3231
class RtsTemp: public MenuItem {

  public:
    RtsTemp (const MicroDS3231 * _rtc);
    String display();
  private:
    MicroDS3231 * rtc;

};

// выводит текущую температуру ворды аквариума через объект ControlTemp
class AquaTemp: public MenuItem {

  public:
    AquaTemp (const ControlTemp * _controlTemp);
    String display();
  private:
    ControlTemp * controlTemp;

};

// выводит логи температуру воды аквариума через объект ControlTemp
// в режиме редактирования - просматриваем историю логов
class TempLog: public MenuItem {

  public:
    TempLog (const global::CurrSettings * _currSettings, const ControlTemp * _controlTemp);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
  private:
    global::CurrSettings* currSettings;
    ControlTemp* controlTemp;
    byte editValue;
    String logToString(byte _index);

};

// выводит заданный показатель счетчика CO2 через объект BubbleCounter
class bubbleCounterValue: public MenuItem {

  public:
    bubbleCounterValue(const BubbleCounter * _bubbleCounter, const typeBubbleCounterValue _valueType);
    String display();
  private:
    BubbleCounter * bubbleCounter;
    typeBubbleCounterValue valueType;

};

// выводит заданный показатель управления расходом CO2 через объект BubbleControl
class bubbleControlValue: public MenuItem {

  public:
    bubbleControlValue (const BubbleControl * _bubbleControl, const typeBubbleControlValue _valueType);
    String display();
  private:
    BubbleControl * bubbleControl;
    typeBubbleControlValue valueType;

};

// выводит редактирует счетчик кормления через объект Feeding
class FeedingValue: public MenuItem {

  public:
    FeedingValue (const Feeding * _feeding);
    String display();
    boolean editing();
    void enterEditing();
    void downValue();
    void upValue();
    void saveEditing();
  private:
    byte editValue;
    Feeding * feeding;

};

#endif
