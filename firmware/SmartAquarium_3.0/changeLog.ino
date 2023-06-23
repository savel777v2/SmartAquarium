/*
  ===== Версия 3.0 =====
  06.06.2023 Первая версия меню с полиформизмом элементов меню. Выполнил 0.0 часы неполностью
  07.06.2023 Убрал main на схему Ардуино через setup() и loop(), иначе не работает millis().
    Добавил объект LoopTime - анализирует события времени
  08.06.2023
    - Расширил возможности анализа клавиш TM1638.h через наследование в TM1638My.h. Теперь использую его.
    - Menu Реализовал переход между submenu по клавиатуре.
  09.06.2023
    - Реализовал логику редактирования MenuItem через Menu
    - Новый наследник byteEEPROMvalue для MenuItem
  13.06.2023
    - Menu - реализовал отражение точек на дисплее
    - submenu.time - выполнил секунднsй ритм точки (гл.пер.\память 841\12000)
    - Melody.h - новый объект Melody.h для мелодии звонка, submenu.alarm - подменю звонка (гл.пер.\память 902\14052)
  14.06.2023
    - Timer.h - новый объект для таймера, submenu.timer - подменю таймера (гл.пер.\память 968\15486)
    - контроль дня\ночи выполнил (гл.пер.\память 968\15652)
    - Lamps.h - новый объект для управления освещением (гл.пер.\память 983\16610)
  20.06.2023
    - ControlTemp.h - новый объект для съема темепратуры и управления нагревателем
    - RtsTemp, AquaTemp - новые объекты наследники MenuItem
    - Menu - выполнил режими curTemp,dayTemp,nightTemp,deltaTemp для submenu (гл.пер.\память 1127\20148)
  21.06.2023
    - Новый наследник TempLog для MenuItem. submenu.logTemp - новое меню (гл.пер.\память 1159\20582)
    - Эскизы меню для bubbleControl (гл.пер.\память 1193\20652)
  22.06.2023
    - Новый объект BubbleCounter (гл.пер.\память 1380\23092)
    - bubbleCounterValue - новый объект наследник MenuItem; bubbleSpeed,sensorValue - новые меню submenu (гл.пер.\память 1408\23312)
  23.06.2023
    - Выполнил шаблоны всех меню для контроля расхода памяти (гл.пер.\память 1460\23478)
    - Новый объект StepMotor (гл.пер.\память 1529\25212)
    - MotorPosition - новый объект наследник MenuItem.byteEEPROMvalue; Menu - режим motorPosition для submenu (гл.пер.\память 1547крит\25530)
    - MicroDS3231 - расскоментировал использавоние (гл.пер.\память 1581крит\27254)

*/
