#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include <ESP32Encoder.h>
#include "action.h"

class Encoder {
private:
  static volatile bool buttonPressed;
  static volatile unsigned long lastButtonTime;
  static volatile bool lastButtonState;
  static volatile bool buttonStateStable;

  static void IRAM_ATTR buttonISR();
public:
  static void init();
  static Action getAction();
};

#endif // ENCODER_H