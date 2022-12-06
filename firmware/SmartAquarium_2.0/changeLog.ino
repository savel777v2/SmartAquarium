/*
  ===== Версия 1.1 =====
  25.10.2022 Разбил проект на вкладки. Провел рефакторинг кода
  26.10.2022 Реализовал новый класс счетчика пузырьков. Пока в меню вывод только одной цифры  
  27.10.2022 Добавил еще 2 подменю в меню счетчик пузырьков. 
    Добавил еще 1 подменю и сохранение настроек в EPROM для счетчика пузырьков. 
    Добавил разделители разрядов во всех меню и подменю.
  30.10.2022 Рефакторинг класса меню
  31.10.2022 Добавил поддержку шагового мотора 
    Добавил управление скоростью мотора
  01.11.2022 Рефакторинг StepMotor - теперь умеет смещаться к заданной точке 
  09.11.2022 Доработал BubbleCounter - теперь передает события во внешнюю функцию.
    Изменения по индикаторам. Рефакторинг меню.
    BubbleCounter - доработал ошибку при некорректных сигналах
  10.11.2022 Добавил индикацию регулярного пузырька, т.е. который капает со стабильной скоростью
  11.11.2022 Исправил ошибки StepMotor по смещению к заданной точке
  12.11.2022 Добавил настройку скорости пузырьков
  16.11.2022 Начал делать управление пузырьками
    Переделал на класс и объект BubbleControl
  17.11.2022 Новый функционал в BubbleControl
  19.11.2022 BubbleControl: переделал настройку bubblesIn10Second на bubblesIn100Second
    BubbleCounter: переделал настройку minBubbleLevel, исправил ошибки
    BubbleControl: новые функции в меню
  20.11.2022 Motor: индикация теперь без делителя
    BubbleCounter: рефакторинг _checkErrorBubble(), _checkErrorNoBubble()
    BubbleControl: теперь звук отключается, настройка нулевого уровня пузырька доступна
  21.11.2022 BubbleControl: новая настройка включать пузырек как днем за указанное число минут до рассвета
  21.11.2022 Новые настройки цикла кормежки рыб едой
  26.11.2022 Расписание кормежки рыб едой. Мелкие правки
  29.11.2022 BubbleControl: ожидает 5 пузырьков после движения мотора

  ===== Версия 2.0 =====
  02.12.2022 BubbleCounter: новая версия с новым принципом обнаружения пузырька. Фактически будет диагностироваться
    форма сигнала пузырька: скосроть подъема, падения, высота и его продолжительность. Будет работать без необходимости калибровки
    на любой мощьности лазера и внешнего освещения. Версия пока в тесте
  02.12.2022 BubbleCounter: отладил новую версию
  02.12.2022 Добавил блок индикации продолжительности кода
  03.12.2022 Temperature - оптимизировал, вынес в отдельную закладку (логика требует доработки)
  05.12.2022 Temperature - исправил ошибки
  06.12.2022 Eating - оптимизировал, вынес в отдельную закладку
  06.12.2022 Eating - добавил кнопку
    
 */ 
