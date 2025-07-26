#include "states.h"
#include "display.h"
#include "clock.h"
#include "state_machine.h"
#include "settings.h"
#include "logging.h"

static uint8_t tempSleepStartHour = 22;
static uint8_t tempSleepStartMinute = 0;
static uint8_t tempQuietStartHour = 23;
static uint8_t tempQuietStartMinute = 0;
static DayOfWeek currentScheduleDay = SUNDAY;

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
  Log::info("Schedule saved to all days of the week");
}

State ScheduleSetSleepHours = {
  .OnEnter = []() {
    Schedule currentSchedule;
    currentScheduleDay = SUNDAY;
    Settings::loadSchedule(currentScheduleDay, currentSchedule);

    tempSleepStartHour = currentSchedule.getSleepStartHour();
    tempSleepStartMinute = currentSchedule.getSleepStartMinute();
    tempQuietStartHour = currentSchedule.getQuietStartHour();
    tempQuietStartMinute = currentSchedule.getQuietStartMinute();
    
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