#include "encoder.h"
#include <Elog.h>
#include <logging.h>

#define DIAL_CLK_PIN 18
#define DIAL_DT_PIN 19
#define DIAL_SW_PIN 21

ESP32Encoder encoder;

volatile bool Encoder::buttonPressed;
volatile unsigned long Encoder::lastButtonTime;
volatile bool Encoder::lastButtonState;
volatile bool Encoder::buttonStateStable;
unsigned long Encoder::buttonPressStartTime = 0;
bool Encoder::buttonHoldDetected = false;
const unsigned long BUTTON_DEBOUNCE_MS = 50;
const unsigned long BUTTON_HOLD_MS = 3000; // 3 seconds

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
      
      // Button pressed (transition from HIGH to LOW)
      if (buttonStateStable == LOW) {
        buttonPressStartTime = millis();
        buttonHoldDetected = false;
        // Trigger SELECT immediately on button press
        action = SELECT;
      }
      // Button released (transition from LOW to HIGH)
      else if (buttonStateStable == HIGH) {
        unsigned long pressDuration = millis() - buttonPressStartTime;
        
        // Trigger SELECT_HOLD on release if held for 3+ seconds
        if (pressDuration >= BUTTON_HOLD_MS) {
          action = SELECT_HOLD;
        }
        
        buttonHoldDetected = false;
        buttonPressStartTime = 0;
      }
    }
  }
  
  lastButtonState = currentButtonState;
  
  return action;
}