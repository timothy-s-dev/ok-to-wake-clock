#ifndef RGBLED_H
#define RGBLED_H

#include "schedule.h"
#include <Arduino.h>

class RgbLed {
public:
  static void init();
  static void indicateStatus(ScheduleBlock scheduleBlock);
  static void turnOff();
  static void setBrightness(uint8_t brightness);

private:
  static void setColor(uint8_t red, uint8_t green, uint8_t blue);
  static uint8_t currentBrightness;
};

#endif // RGBLED_H