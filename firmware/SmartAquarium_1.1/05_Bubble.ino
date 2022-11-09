// функция срабатывает на событие пузырька от BubbleCounter
void onTheBubble(byte _events) {
  bool _needDisplay = false; // есть необходимость обновить дисплей

  if ((_events & 0b00000100) == 0b00000100) Module.setLED(1, 7); // начало пузырька
  if ((_events & 0b00001000) == 0b00001000) Module.setLED(0, 7); // конец пузырька
  
  // обновляем меню если в нужном режиме
  if (currMode.main == 2) {    
    if (currMode.secondary == 3) {
      if ((_events & 0b00000010) == 0b00000010) _needDisplay = true; //считываний сенсора в секунду
    }
    else if (currMode.secondary == 4) {
      if ((_events & 0b00000001) == 0b00000001) _needDisplay = true; //обсчет Min\Max
    }
    else {
      if (_events >= 0b00001000) _needDisplay = true; // конец пузырька или ошибка
    }
  }

  if (_needDisplay) printDisplay();  

}
