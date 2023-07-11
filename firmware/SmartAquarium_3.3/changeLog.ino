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
    - MicroDS3231 - расскоментировал использование (гл.пер.\память 1581крит\27254)
  25.06.2023
    - Новый объект BubbleControl.h; bubbleSettings, bubbleDaySpeed, bubbleNightSpeed, beforeMorningStart, bubbleControlSound - новые меню submenu (гл.пер.\память 1611\29148)
  26.06.2023
    - bubbleControlValue - новый объект наследник MenuItem; bubblesInSecond - модифицировал меню submenu (гл.пер.\память 1653\29366)
    - bubbleControlSettings - новые меню submenu (гл.пер.\память 1651\29482)
    - morningFeeding, eveningFeeding, dayFeedingSettings, nightFeeding - новые настройки для Feeding submenu (гл.пер.\память 1641\30202)
    - Новый объект Feeding.h (гл.пер.\память 1655\30980) ПРЕВЫСИЛ ПАМЯТЬ 30720!!!
    - Оптимизация PPROM: убрал DateTime в MicroDS3231 - (гл.пер.\память 1650\30784)
    - Оптимизация PPROM: убрал пины в StepMotor - (гл.пер.\память 1646\30714)
    - Оптимизация PPROM: ControlTemp сменил библиотеку DS18B20.h на легкую microDS18B20.h - (гл.пер.\память 1624\29910)
    - Оптимизация PPROM: ControlTemp рефакторинг readTemperatureNeedDisplay (гл.пер.\память 1624\29874)
    - Оптимизация PPROM: убрал пин в Melody.h + рефакторинг - (гл.пер.\память 1624\29820)
  27.06.2023
    - Оптимизация PPROM: переместил currSettings внутрь наследников MenuItem (гл.пер.\память 1624\29652)
    - Оптимизация PPROM: объединил TimerMinute+TimerSecond=TimerValue внутри наследников MenuItem (гл.пер.\память 1602\29452)
    - Оптимизация PPROM: объединил CurHour+CurMinute=TimeValue внутри наследников MenuItem (гл.пер.\память 1580\29314)
    - Оптимизация PPROM: объединил DayFlag+TimerFlag=SettingsValue внутри наследников MenuItem (гл.пер.\память 1558\29284)
    - Оптимизация PPROM: удалил избыточные текстовые объекты в Menu (гл.пер.\память 1566\29018)
    - Новый наследник FeedingValue для MenuItem. submenu.feedingLoop - новое меню (гл.пер.\память 1594\29460)
    - TimeValue для MenuItem - добавил установку времени в MicroDS3231 (гл.пер.\память 1594\30450)
    - Оптимизация PPROM: StepMotor - digitalWrite вынес в функцию + мелочи (гл.пер.\память 1594\30398)
    - Оптимизация PPROM: вынес гл. функцию timeInMinutes (гл.пер.\память 1594\30376)
    
  ===== Версия 3.1 (тупиковая ветвь) =====
  28.06.2023
    - Оптимизация PPROM: наследник MenuItem.TextItem переделал String на Char* (гл.пер.\память 1594\29734)
  05.07.2023
    - Вывели приветственную надпись (гл.пер.\память 1602\29852)
    - Добавил вывод продолжительности (гл.пер.\память 1604\30324)
    - Убрал объект LoopTime в основной модуль за ненадобностью, разнес код по закладкам (гл.пер.\память 1584\29606)
    - Через выводи продолжительности диагностировал задержки menu.display() до 4 мс. - нужна оптимизация (гл.пер.\память 1584\29628)

  ===== Версия 3.2 =====
  07.07.2023
    - Восстановил с версии 3.0. Ветку 3.1 определил как тупиковую
    - Оптимизация PPROM: наследник MenuItem.TextItem переделал String на Char* (гл.пер.\память 1594\29734)
    - Melody - разделил интерфейс и реализацию (гл.пер.\память 1594\29736)
    - StepMotor - разделил интерфейс и реализацию (гл.пер.\память 1594\29816)
  10.07.2023
    - Timer - разделил интерфейс и реализацию (гл.пер.\память 1594\29816)
    - CurrSettings - разделил интерфейс и реализацию, переименовал на Global.h (гл.пер.\память 1594\29816)
    - Feeding - разделил интерфейс и реализацию (гл.пер.\память 1594\29792)
  11.07.2023
    - Lamps - разделил интерфейс и реализацию (гл.пер.\память 1594\29792) 
    - BubbleControl,BubbleCounter,ControlTemp,Menu,MenuItem,TM1638My - разделил интерфейс и реализацию (гл.пер.\память 1594\29788) 
    - LoopTime - разделил интерфейс и реализацию (гл.пер.\память 1594\29788) 

  ===== Версия 3.3 =====   
  11.07.2023
    - Lamps - переделал передачу переменной на гл. переменные в целях оптимизации и улучшения читабельности (гл.пер.\память 1592\29764)
    - Исправил все указатели на объекты на глобальные переменные объектов в целях оптимизации (гл.пер.\память 1550\28492)   

*/