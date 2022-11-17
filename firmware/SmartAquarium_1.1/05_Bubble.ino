// функция срабатывает на событие пузырька от BubbleCounter
void onTheBubble(byte _events) {
  bool _needDisplay = false; // есть необходимость обновить дисплей

  if ((_events & 0b00000100) == 0b00000100) Module.setLED(1, 7); // начало пузырька
  if ((_events & 0b00001000) == 0b00001000) Module.setLED(0, 7); // конец пузырька

  // регулярность пузырьков
  if (_events >= 0b00001000 && !StepMotorBubbles.get_isActive()) {
    if (CounterForBubbles.get_itsRegularBubbles()) {
      // пузырьки регулярны - значит их скорость нужно проконтролировать
      BubbleSpeedControl.control(CounterForBubbles.get_lastDuration());
      Module.setLED(0, 6);
    }
    else {
      Module.setLED(1, 6);
    }
  }

  // обновляем меню если в нужном режиме
  if (currMode.main == 2 && currMode.secondary == 2) {
    // обсчет Min\Max
    if ((_events & 0b00000001) == 0b00000001) _needDisplay = true;
  }
  else if (currMode.main == 3 && currMode.secondary == 3) {
    // считываний сенсора в секунду
    if ((_events & 0b00000010) == 0b00000010) _needDisplay = true;
  }
  else if ((currMode.main == 2 && currMode.secondary <= 1) || (currMode.main == 3 && currMode.secondary != 0 && currMode.secondary != 5)) {
    // прочие режимы от пузырька в составе 2 и 3 на конец пузырька или ошибка
    if (_events >= 0b00001000) _needDisplay = true;
  }

  if (_needDisplay) printDisplay();

}
