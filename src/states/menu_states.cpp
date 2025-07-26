#include "states.h"
#include "display.h"
#include "clock.h"
#include "state_machine.h"
#include "settings.h"
#include "logging.h"

State MenuTime = {
  .OnEnter = []() { Display::getInstance().print("TIME"); },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { StateMachine::setState(&MenuSchedule); },
  .OnCounterClockwise = []() { StateMachine::setState(&MenuLock); },
  .OnSelect = []() { StateMachine::setState(&TimeSetHours); },
  .OnSelectHold = []() { /* Do nothing */ }
};

State MenuSchedule = {
  .OnEnter = []() { Display::getInstance().print("SCHD"); },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { StateMachine::setState(&MenuNap); },
  .OnCounterClockwise = []() { StateMachine::setState(&MenuTime); },
  .OnSelect = []() { 
    StateMachine::setState(&ScheduleSetSleepHours); 
  },
  .OnSelectHold = []() { /* Do nothing */ }
};

State MenuNap = {
  .OnEnter = []() { 
    // Check if nap is currently active
    if (Settings::isNapEnabled()) {
      Display::getInstance().print("STOP");
    } else {
      Display::getInstance().print("NAP");
    }
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { StateMachine::setState(&MenuLock); },
  .OnCounterClockwise = []() { StateMachine::setState(&MenuSchedule); },
  .OnSelect = []() { 
    if (Settings::isNapEnabled()) {
      Settings::stopNap();
      Clock::updateScheduleLED();
      Log::info("Nap stopped by user");
      StateMachine::setState(&Clock);
    } else {
      StateMachine::setState(&NapSetDuration);
    }
  },
  .OnSelectHold = []() { /* Do nothing */ }
};

State MenuLock = {
  .OnEnter = []() { 
    Display::getInstance().print("LOCK");
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { StateMachine::setState(&MenuBack); },
  .OnCounterClockwise = []() { StateMachine::setState(&MenuNap); },
  .OnSelect = []() { 
    Settings::setLocked(true);
    StateMachine::setState(&Locked);
  },
  .OnSelectHold = []() { /* Do nothing */ }
};

State MenuBack = {
  .OnEnter = []() { Display::getInstance().print("BACK"); },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { StateMachine::setState(&MenuTime); },
  .OnCounterClockwise = []() { StateMachine::setState(&MenuLock); },
  .OnSelect = []() { StateMachine::setState(&Clock); },
  .OnSelectHold = []() { /* Do nothing */ }
};
