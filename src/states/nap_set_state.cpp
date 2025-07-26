#include "states.h"
#include "display.h"
#include "clock.h"
#include "state_machine.h"
#include "settings.h"
#include "logging.h"

static uint16_t tempNapDuration = 60; // Default 60 minutes

State NapSetDuration = {
  .OnEnter = []() {
    tempNapDuration = 60;
    Display::getInstance().print(String(tempNapDuration));
    delay(10);
    Display::getInstance().colonOff();
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    // Increment duration by 5 minutes, max 300 (5 hours)
    tempNapDuration += 5;
    if (tempNapDuration > 300) {
      tempNapDuration = 300;
    }
    Display::getInstance().print(String(tempNapDuration));
  },
  .OnCounterClockwise = []() { 
    // Decrement duration by 5 minutes, min 5
    if (tempNapDuration > 5) {
      tempNapDuration -= 5;
    }
    Display::getInstance().print(String(tempNapDuration));
  },
  .OnSelect = []() { 
    // Start the nap with the selected duration
    if (Clock::startNap(tempNapDuration)) {
      Log::info("Nap started with duration %d minutes", tempNapDuration);
    } else {
      Log::error("Failed to start nap");
    }
    StateMachine::setState(&Clock); 
  },
  .OnSelectHold = []() { /* Do nothing */ }
};