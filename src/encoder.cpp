#include "encoder.h"

#define DIAL_CLK_PIN 18
#define DIAL_DT_PIN 19
#define DIAL_SW_PIN 21

ESP32Encoder encoder;

volatile bool Encoder::buttonPressed;
volatile unsigned long Encoder::lastButtonTime;
volatile bool Encoder::lastButtonState;
volatile bool Encoder::buttonStateStable;
const unsigned long BUTTON_DEBOUNCE_MS = 50;

void IRAM_ATTR Encoder::buttonISR() {
  lastButtonTime = millis();
}

void Encoder::init() {
  encoder.attachSingleEdge(DIAL_DT_PIN, DIAL_CLK_PIN);
  encoder.setCount(0);

  pinMode(DIAL_SW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(DIAL_SW_PIN), buttonISR, CHANGE);
}

Action Encoder::getAction() {
  static int64_t lastEncoderCount = 0;
  int64_t currentCount = encoder.getCount();
  
  Action action = NONE;
  
  // Check for rotation
  if (currentCount > lastEncoderCount) {
    action = CW;
    lastEncoderCount = currentCount;
  } else if (currentCount < lastEncoderCount) {
    action = CCW;
    lastEncoderCount = currentCount;
  }
  
  // Software debouncing for button
  static unsigned long lastDebounceTime = 0;
  bool currentButtonState = digitalRead(DIAL_SW_PIN);
  
  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (currentButtonState != buttonStateStable) {
      buttonStateStable = currentButtonState;
      
      // Only register a press on the transition from HIGH to LOW
      if (buttonStateStable == LOW) {
        action = SELECT;
      }
    }
  }
  
  lastButtonState = currentButtonState;
  
  return action;
}