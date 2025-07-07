#ifndef RGBLED_H
#define RGBLED_H

#include <Arduino.h>

class RgbLed {
public:
  static void init();
  static void setColor(uint8_t red, uint8_t green, uint8_t blue);
  static void turnOff();
};

#endif // RGBLED_H