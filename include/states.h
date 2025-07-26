#ifndef STATES_H
#define STATES_H

struct State {
  void (*OnEnter)();
  void (*OnExit)();

  void (*OnClockwise)();
  void (*OnCounterClockwise)();
  void (*OnSelect)();
  void (*OnSelectHold)();
  void (*OnTimeChange)();
};

extern State Clock;
extern State Locked;
extern State MenuTime;
extern State MenuSchedule;
extern State MenuNap;
extern State MenuLock;
extern State MenuBack;
extern State TimeSetHours;
extern State TimeSetMinutes;
extern State ScheduleSetSleepHours;
extern State ScheduleSetSleepMinutes;
extern State ScheduleSetQuietHours;
extern State ScheduleSetQuietMinutes;
extern State NapSetDuration;

#endif
