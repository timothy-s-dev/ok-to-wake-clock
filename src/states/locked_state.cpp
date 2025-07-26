#include "states.h"
#include "display.h"
#include "clock.h"
#include "state_machine.h"
#include "settings.h"
#include "logging.h"

void showLockMessage() {
  String currentDisplay = Clock::getTimeString();
  Display::getInstance().clear();
  Display::getInstance().print("LOCK");
  delay(1000);
  Display::getInstance().clear();
  Display::getInstance().print(currentDisplay);
  delay(10);
  Display::getInstance().colonOn();
}

State Locked = {
  .OnEnter = []() {
    Display::getInstance().print(Clock::getTimeString());
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().colonOff(); Display::getInstance().clear(); },
  .OnClockwise = []() { showLockMessage(); },
  .OnCounterClockwise = []() { showLockMessage(); },
  .OnSelect = []() { showLockMessage(); },
  .OnSelectHold = []() {
    Settings::setLocked(false);
    Display::getInstance().clear();
    Display::getInstance().print("UNLK");
    delay(1000);
    StateMachine::setState(&Clock);
  },
  .OnTimeChange = []() {
    Display::getInstance().print(Clock::getTimeString());
    delay(10);
    Display::getInstance().colonOn();
  }
};