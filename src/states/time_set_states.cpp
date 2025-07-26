#include "states.h"
#include "display.h"
#include "clock.h"
#include "state_machine.h"
#include "settings.h"
#include "logging.h"

State TimeSetHours = {
  .OnEnter = []() {
    uint8_t currentHours = Clock::getCurrentHours();
    String AMPM = currentHours < 12 ? "AM" : "PM";
    Display::getInstance().print(Clock::getTimeString().substring(0, 2) + AMPM);
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    uint8_t currentHours = Clock::getCurrentHours();
    uint8_t currentMinutes = Clock::getCurrentMinutes();
    
    // Increment hours (24-hour format, wraps from 23 to 0)
    currentHours = (currentHours + 1) % 24;
    
    Clock::setTime(currentHours, currentMinutes, 0);
    String AMPM = currentHours < 12 ? "AM" : "PM";
    Display::getInstance().print(Clock::getTimeString().substring(0, 2) + AMPM);
  },
  .OnCounterClockwise = []() { 
    uint8_t currentHours = Clock::getCurrentHours();
    uint8_t currentMinutes = Clock::getCurrentMinutes();
    
    // Decrement hours (24-hour format, wraps from 0 to 23)
    currentHours = (currentHours == 0) ? 23 : currentHours - 1;
    
    Clock::setTime(currentHours, currentMinutes, 0);
    String AMPM = currentHours < 12 ? "AM" : "PM";
    Display::getInstance().print(Clock::getTimeString().substring(0, 2) + AMPM);
  },
  .OnSelect = []() { StateMachine::setState(&TimeSetMinutes); },
  .OnSelectHold = []() { /* Do nothing */ }
};

State TimeSetMinutes = {
  .OnEnter = []() {
    Display::getInstance().print("M " + Clock::getTimeString().substring(2));
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    uint8_t currentHours = Clock::getCurrentHours();
    uint8_t currentMinutes = Clock::getCurrentMinutes();
    
    // Increment minutes (wraps from 59 to 0)
    currentMinutes = (currentMinutes + 1) % 60;
    
    Clock::setTime(currentHours, currentMinutes, 0);
    Display::getInstance().print("M " + Clock::getTimeString().substring(2));
  },
  .OnCounterClockwise = []() { 
    uint8_t currentHours = Clock::getCurrentHours();
    uint8_t currentMinutes = Clock::getCurrentMinutes();
    
    // Decrement minutes (wraps from 0 to 59)
    currentMinutes = (currentMinutes == 0) ? 59 : currentMinutes - 1;
    
    Clock::setTime(currentHours, currentMinutes, 0);
    Display::getInstance().print("M " + Clock::getTimeString().substring(2));
  },
  .OnSelect = []() { StateMachine::setState(&Clock); },
  .OnSelectHold = []() { /* Do nothing */ }
};