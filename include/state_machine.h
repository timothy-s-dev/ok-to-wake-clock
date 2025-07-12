#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "action.h"
#include "settings.h"

struct State {
  void (*OnEnter)();
  void (*OnExit)();

  void (*OnClockwise)();
  void (*OnCounterClockwise)();
  void (*OnSelect)();
  void (*OnSelectHold)();
  void (*OnTimeChange)();
};

class StateMachine {
public:
  static void init();
  static void setState(State* newState);
  static void processAction(Action action);
};

#endif // STATE_MACHINE_H