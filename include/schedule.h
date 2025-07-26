#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>

enum ScheduleBlock {
  WIND_DOWN,
  SLEEP,
  QUIET,
  WAKE,
  NO_BLOCK,
};

class Schedule {
public:
  Schedule();
  static Schedule getNap(uint8_t duration);

  static Schedule fromByteArray(const std::array<uint8_t, 10>& data);
  std::array<uint8_t, 10> convertToByteArray() const;

  void setWinddownStart(uint8_t hour, uint8_t minute);
  void setSleepStart(uint8_t hour, uint8_t minute);
  void setQuietStart(uint8_t hour, uint8_t minute);
  void setWakeStart(uint8_t hour, uint8_t minute);
  void setWakeEnd(uint8_t hour, uint8_t minute);

  uint8_t getWinddownStartHour() const { return winddownStart / 60; }
  uint8_t getWinddownStartMinute() const { return winddownStart % 60; }
  uint8_t getSleepStartHour() const { return sleepStart / 60; }
  uint8_t getSleepStartMinute() const { return sleepStart % 60; }
  uint8_t getQuietStartHour() const { return quietStart / 60; }
  uint8_t getQuietStartMinute() const { return quietStart % 60; }
  uint8_t getWakeStartHour() const { return wakeStart / 60; }
  uint8_t getWakeStartMinute() const { return wakeStart % 60; }
  uint8_t getWakeEndHour() const { return wakeEnd / 60; }
  uint8_t getWakeEndMinute() const { return wakeEnd % 60; }

  bool isActive() const;
  ScheduleBlock getCurrentBlock() const;

private:
  uint16_t winddownStart;
  uint16_t sleepStart;
  uint16_t quietStart;
  uint16_t wakeStart;
  uint16_t wakeEnd;
};

#endif // SCHEDULE_H