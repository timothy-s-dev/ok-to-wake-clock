#include "states.h"
#include "display.h"
#include "clock.h"
#include "state_machine.h"

State Clock = {
  .OnEnter = []() {
    Display::getInstance().print(Clock::getTimeString());
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().colonOff(); Display::getInstance().clear(); },
  .OnClockwise = []() { StateMachine::setState(&MenuTime); },
  .OnCounterClockwise = []() { StateMachine::setState(&MenuTime); },
  .OnSelect = []() { StateMachine::setState(&MenuTime); },
  .OnTimeChange = []() {
    Display::getInstance().print(Clock::getTimeString());
    delay(10);
    Display::getInstance().colonOn();
  }
};