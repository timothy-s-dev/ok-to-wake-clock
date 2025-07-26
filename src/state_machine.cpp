#include "state_machine.h"
#include "display.h"
#include "clock.h"
#include "settings.h"
#include "schedule.h"
#include "logging.h"

State* currentState = nullptr;

void StateMachine::init() {
  if (Settings::isLocked()) {
    setState(&Locked);
  } else {
    setState(&Clock);
  }
}

void StateMachine::setState(State* newState) {
  if (currentState != nullptr && currentState->OnExit) {
    currentState->OnExit();
  }
  currentState = newState;
  if (currentState->OnEnter) {
    currentState->OnEnter();
  }
}

void StateMachine::processAction(Action action) {
  if (currentState == nullptr || action == NONE) return;
  if (action == CW && currentState->OnClockwise) currentState->OnClockwise();
  if (action == CCW && currentState->OnCounterClockwise) currentState->OnCounterClockwise();
  if (action == SELECT && currentState->OnSelect) currentState->OnSelect();
  if (action == SELECT_HOLD && currentState->OnSelectHold) currentState->OnSelectHold();
  if (action == TIME_CHANGE && currentState->OnTimeChange) currentState->OnTimeChange();
}
