#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "action.h"
#include "settings.h"
#include "states.h"

class StateMachine {
public:
  static void init();
  static void setState(State* newState);
  static void processAction(Action action);
};

#endif // STATE_MACHINE_H