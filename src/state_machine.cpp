#include "state_machine.h"
#include "display.h"
#include "clock.h"
#include "settings.h"
#include "schedule.h"
#include <Elog.h>
#include <logging.h>

State* currentState = nullptr;

// Lock state flag
static bool isDeviceLocked = false;

// Helper function to show lock message
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

// Schedule setting variables
static uint8_t tempSleepStartHour = 22;
static uint8_t tempSleepStartMinute = 0;
static uint8_t tempQuietStartHour = 23;
static uint8_t tempQuietStartMinute = 0;
static DayOfWeek currentScheduleDay = SUNDAY;

// Nap setting variables
static uint16_t tempNapDuration = 60; // Default 60 minutes

// Forward declarations of states
extern State Clock;
extern State MenuTime;
extern State MenuSchedule;
extern State MenuNap;
extern State MenuLock;
extern State TimeSetHours;
extern State TimeSetMinutes;
extern State ScheduleSetSleepHours;
extern State ScheduleSetSleepMinutes;
extern State ScheduleSetQuietHours;
extern State ScheduleSetQuietMinutes;
extern State NapSetDuration;

void StateMachine::init() {
  // Check if device is locked on startup
  isDeviceLocked = Settings::isLocked();
  setState(&Clock);
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

// State definitions
State Clock = {
  .OnEnter = []() {
    Display::getInstance().print(Clock::getTimeString());
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().colonOff(); Display::getInstance().clear(); },
  .OnClockwise = []() { 
    if (isDeviceLocked) {
      showLockMessage();
    } else {
      StateMachine::setState(&MenuTime);
    }
  },
  .OnCounterClockwise = []() { 
    if (isDeviceLocked) {
      showLockMessage();
    } else {
      StateMachine::setState(&MenuTime);
    }
  },
  .OnSelect = []() { 
    if (isDeviceLocked) {
      showLockMessage();
    } else {
      StateMachine::setState(&MenuTime);
    }
  },
  .OnSelectHold = []() { 
    if (isDeviceLocked) {
      // Unlock the device
      isDeviceLocked = false;
      Settings::setLocked(false);
      Display::getInstance().clear();
      Display::getInstance().print("UNLK");
      delay(1000);
      Display::getInstance().clear();
      Display::getInstance().print(Clock::getTimeString());
      delay(10);
      Display::getInstance().colonOn();
    }
  },
  .OnTimeChange = []() {
    Display::getInstance().print(Clock::getTimeString());
    delay(10);
    Display::getInstance().colonOn();
  }
};

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
    Schedule currentSchedule;
    currentScheduleDay = SUNDAY;
    Settings::loadSchedule(currentScheduleDay, currentSchedule);
    
    tempSleepStartHour = currentSchedule.getSleepStartHour();
    tempSleepStartMinute = currentSchedule.getSleepStartMinute();
    tempQuietStartHour = currentSchedule.getQuietStartHour();
    tempQuietStartMinute = currentSchedule.getQuietStartMinute();
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
      Logger.info(MAIN_LOG, "Nap stopped by user");
      StateMachine::setState(&Clock);
    } else {
      tempNapDuration = 60;
      StateMachine::setState(&NapSetDuration);
    }
  },
  .OnSelectHold = []() { /* Do nothing */ }
};

State MenuLock = {
  .OnEnter = []() { 
    // Show current lock status
    if (isDeviceLocked) {
      Display::getInstance().print("ULCK");  // Show unlock option
    } else {
      Display::getInstance().print("LOCK");  // Show lock option
    }
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { StateMachine::setState(&MenuTime); },
  .OnCounterClockwise = []() { StateMachine::setState(&MenuNap); },
  .OnSelect = []() { 
    // Toggle lock state
    if (isDeviceLocked) {
      // Unlock the device
      isDeviceLocked = false;
      Settings::setLocked(false);
      Display::getInstance().clear();
      Display::getInstance().print("UNLK");
      delay(1000);
    } else {
      // Lock the device
      isDeviceLocked = true;
      Settings::setLocked(true);
      Display::getInstance().clear();
      Display::getInstance().print("LOCK");
      delay(1000);
    }
    StateMachine::setState(&Clock);
  },
  .OnSelectHold = []() { /* Do nothing */ }
};

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

// Helper function to calculate and save complete schedule
void saveCompleteSchedule() {
  Schedule schedule;
  
  // Set the user-configured sleep and quiet start times
  schedule.setSleepStart(tempSleepStartHour, tempSleepStartMinute);
  schedule.setQuietStart(tempQuietStartHour, tempQuietStartMinute);
  
  // Calculate winddown start (sleep start - 30 minutes)
  int winddownMinutes = (tempSleepStartHour * 60 + tempSleepStartMinute) - 30;
  if (winddownMinutes < 0) winddownMinutes += 24 * 60; // Handle day wrap
  uint8_t winddownHour = (winddownMinutes / 60) % 24;
  uint8_t winddownMinute = winddownMinutes % 60;
  schedule.setWinddownStart(winddownHour, winddownMinute);
  
  // Calculate wake start (quiet start + 15 minutes)
  int wakeStartMinutes = (tempQuietStartHour * 60 + tempQuietStartMinute) + 15;
  if (wakeStartMinutes >= 24 * 60) wakeStartMinutes -= 24 * 60; // Handle day wrap
  uint8_t wakeStartHour = (wakeStartMinutes / 60) % 24;
  uint8_t wakeStartMin = wakeStartMinutes % 60;
  schedule.setWakeStart(wakeStartHour, wakeStartMin);
  
  // Calculate wake end (quiet start + 30 minutes)
  int wakeEndMinutes = (tempQuietStartHour * 60 + tempQuietStartMinute) + 30;
  if (wakeEndMinutes >= 24 * 60) wakeEndMinutes -= 24 * 60; // Handle day wrap
  uint8_t wakeEndHour = (wakeEndMinutes / 60) % 24;
  uint8_t wakeEndMin = wakeEndMinutes % 60;
  schedule.setWakeEnd(wakeEndHour, wakeEndMin);
  
  // Save the schedule to all 7 days of the week
  Schedule allSchedules[7];
  for (int i = 0; i < 7; i++) {
    allSchedules[i] = schedule;
  }
  Settings::saveAllSchedules(allSchedules);
  Logger.info(MAIN_LOG, "Schedule saved to all days of the week");
}

State ScheduleSetSleepHours = {
  .OnEnter = []() {
    Display::getInstance().print("STRT");
    delay(10);
    Display::getInstance().colonOff();
    delay(1000);
    String AMPM = tempSleepStartHour < 12 ? "AM" : "PM";
    uint8_t displayHour = tempSleepStartHour;
    if (displayHour == 0) displayHour = 12;
    else displayHour = displayHour % 12;
    Display::getInstance().print((displayHour < 10 ? "0" : "") + String(displayHour) + AMPM);
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    tempSleepStartHour = (tempSleepStartHour + 1) % 24;
    String AMPM = tempSleepStartHour < 12 ? "AM" : "PM";
    uint8_t displayHour = tempSleepStartHour;
    if (displayHour == 0) displayHour = 12;
    else displayHour = displayHour % 12;
    Display::getInstance().print((displayHour < 10 ? "0" : "") + String(displayHour) + AMPM);
  },
  .OnCounterClockwise = []() { 
    tempSleepStartHour = (tempSleepStartHour == 0) ? 23 : tempSleepStartHour - 1;
    String AMPM = tempSleepStartHour < 12 ? "AM" : "PM";
    uint8_t displayHour = tempSleepStartHour;
    if (displayHour == 0) displayHour = 12;
    else displayHour = displayHour % 12;
    Display::getInstance().print((displayHour < 10 ? "0" : "") + String(displayHour) + AMPM);
  },
  .OnSelect = []() { StateMachine::setState(&ScheduleSetSleepMinutes); },
  .OnSelectHold = []() { /* Do nothing */ }
};

State ScheduleSetSleepMinutes = {
  .OnEnter = []() {
    String minuteStr = (tempSleepStartMinute < 10) ? "0" + String(tempSleepStartMinute) : String(tempSleepStartMinute);
    Display::getInstance().print("M " + minuteStr);
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    tempSleepStartMinute = (tempSleepStartMinute + 1) % 60;
    String minuteStr = (tempSleepStartMinute < 10) ? "0" + String(tempSleepStartMinute) : String(tempSleepStartMinute);
    Display::getInstance().print("M " + minuteStr);
  },
  .OnCounterClockwise = []() { 
    tempSleepStartMinute = (tempSleepStartMinute == 0) ? 59 : tempSleepStartMinute - 1;
    String minuteStr = (tempSleepStartMinute < 10) ? "0" + String(tempSleepStartMinute) : String(tempSleepStartMinute);
    Display::getInstance().print("M " + minuteStr);
  },
  .OnSelect = []() { StateMachine::setState(&ScheduleSetQuietHours); },
  .OnSelectHold = []() { /* Do nothing */ }
};

State ScheduleSetQuietHours = {
  .OnEnter = []() {
    Display::getInstance().print("STOP");
    delay(10);
    Display::getInstance().colonOff();
    delay(1000);
    String AMPM = tempQuietStartHour < 12 ? "AM" : "PM";
    uint8_t displayHour = tempQuietStartHour;
    if (displayHour == 0) displayHour = 12;
    else displayHour = displayHour % 12;
    Display::getInstance().print((displayHour < 10 ? "0" : "") + String(displayHour) + AMPM);
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    tempQuietStartHour = (tempQuietStartHour + 1) % 24;
    String AMPM = tempQuietStartHour < 12 ? "AM" : "PM";
    uint8_t displayHour = tempQuietStartHour;
    if (displayHour == 0) displayHour = 12;
    else displayHour = displayHour % 12;
    Display::getInstance().print((displayHour < 10 ? "0" : "") + String(displayHour) + AMPM);
  },
  .OnCounterClockwise = []() { 
    tempQuietStartHour = (tempQuietStartHour == 0) ? 23 : tempQuietStartHour - 1;
    String AMPM = tempQuietStartHour < 12 ? "AM" : "PM";
    uint8_t displayHour = tempQuietStartHour;
    if (displayHour == 0) displayHour = 12;
    else displayHour = displayHour % 12;
    Display::getInstance().print((displayHour < 10 ? "0" : "") + String(displayHour) + AMPM);
  },
  .OnSelect = []() { StateMachine::setState(&ScheduleSetQuietMinutes); },
  .OnSelectHold = []() { /* Do nothing */ }
};

State ScheduleSetQuietMinutes = {
  .OnEnter = []() {
    String minuteStr = (tempQuietStartMinute < 10) ? "0" + String(tempQuietStartMinute) : String(tempQuietStartMinute);
    Display::getInstance().print("M " + minuteStr);
    delay(10);
    Display::getInstance().colonOn();
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    tempQuietStartMinute = (tempQuietStartMinute + 1) % 60;
    String minuteStr = (tempQuietStartMinute < 10) ? "0" + String(tempQuietStartMinute) : String(tempQuietStartMinute);
    Display::getInstance().print("M " + minuteStr);
  },
  .OnCounterClockwise = []() { 
    tempQuietStartMinute = (tempQuietStartMinute == 0) ? 59 : tempQuietStartMinute - 1;
    String minuteStr = (tempQuietStartMinute < 10) ? "0" + String(tempQuietStartMinute) : String(tempQuietStartMinute);
    Display::getInstance().print("M " + minuteStr);
  },
  .OnSelect = []() { 
    // Save the complete schedule with calculated values
    saveCompleteSchedule();
    // Update the RGB LED based on the new schedule
    Clock::updateScheduleLED();
    StateMachine::setState(&Clock); 
  },
  .OnSelectHold = []() { /* Do nothing */ }
};

State NapSetDuration = {
  .OnEnter = []() {
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
      Logger.info(MAIN_LOG, "Nap started with duration %d minutes", tempNapDuration);
    } else {
      Logger.error(MAIN_LOG, "Failed to start nap");
    }
    StateMachine::setState(&Clock); 
  },
  .OnSelectHold = []() { /* Do nothing */ }
};