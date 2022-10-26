// функция срабатывает на событие пузырька от BubbleCounter
void onTheBubble() {

  // индикация пролета пузырька
  if (CounterForBubbles.get_itsBubble()) Module.setLED(1, 7);
  else Module.setLED(0, 7);

  // обновляем меню если в нужном режиме
  if (currMode.main == 2) {
    printDisplay();
  }

}
