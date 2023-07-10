/*
  StepMotor.h - Объект для управления шаговым двигателем (4-ре контакта)
    реализует управление шаговым мотором с учетом инерциальности. Например
    была абсалютная точка мотора 2500, задачи точку 5000. Мотор разгонится и начнет крутиться
    с максимальной скоростью. Неожиданно сменили точку на 500. Мотор затормозится, встанет в 0,
    затем разгонится в обратном направлении. Перед нужной точкой мотор затормозится и встанет в 0 ровно в нужной точке.
    Одно значение - четверть шага мотора (полный шаг - один оборот)
*/

#ifndef StepMotor_h
#define StepMotor_h

#include "Arduino.h"

#define MIN_DELAY 4 // минимальная задержка между шагом мотора в мс., соответсвует MAX_USER_SPEED
#define MAX_USER_SPEED 32 // максимально допустимое значение линейной пользовательской скорости
#define MAX_DELAY 128 // максимальная задержка шага мотора при скорости около 0 = MIN_DELAY * MAX_USER_SPEED

#define MOTOR_PIN_1 11
#define MOTOR_PIN_2 10
#define MOTOR_PIN_3 9
#define MOTOR_PIN_4 8

class StepMotor {
  public:

    StepMotor();
    // функции постоянной скорости мотора
    void set_userSpeed(int userSpeed);
    int get_userSpeed();
    // куда подвинуть мотор в абсалютном значении, т.е. относительно 0
    void set_positionMotor(long positionMotor);
    long get_positionMotor();
    // куда подвинуть мотор в относительном значении, т.е. относительно начальном точки перед движением
    void set_positionMove(long positionMove);
    long get_positionMove();
    // если сейчас выполняется движение мотора
    bool get_isActive();
    // время последнего шага мотора
    unsigned long get_lastSeekMotor();
    // функция движения мотора, вызывать как можно чаще, возвращает:
    //  255 - мотор ничего не делает
    //  -1 - мотор движется назад
    // 0 - мотор остановился
    // +1 - мотор движется вперед
    int loopDirection();

  private:

    // функция расчета задержки шага мотора от пользовательской скорости мотора
    int getStepDelay(int _UserSpeed);
    // задержку приблизим к нужной в соответствии с нужной скоростью
    void stepDelayBringCloser(int& stepDelay, int needStepDelay);
    // двигаемся к нужной позиции сначала ускоряясь, затем замедляясь
    void stepDelayMovePosition(int& stepDelay);
    // шаг мотора вверх
    void motorPositionUp(byte& phaseMotor, int& ans);
    // шаг мотора вниз
    void motorPositionDown(byte& phaseMotor, int& ans);
    // прописывает значения в пины
    void writePins(byte pins);

    unsigned long _lastSeekMotor = 0; // время последнего шага мотора
    bool _isActive = false; // мотор активен
    long _positionMotor = 0; // относительная позиция мотора
    long _positionMove = 0; // изменение позиции мотора
    int _userSpeed = 0; // нужное направление и скорость текущего движения мотора
    int _directionMotor = 0; // текущее направление мотора
    int _userDelayMotor = MIN_DELAY * MAX_USER_SPEED; // нужная задержка мотора (зависит от userSpeed)
    int _stepDelay = MAX_DELAY; // текущая задержка шага мотора
    byte _phaseMotor = 0; // фаза мотора 0-3
    // кэш расчета как нужно тормозить с заданным ускорением
    byte _brakingRouteDelay[16] = {124, 99, 79, 63, 50, 40, 31, 25, 20, 16, 13, 10, 8, 6, 5, 4};

};

#endif
