#include <Arduino.h>
#include <Wire.h>
#include "state_machine.h"
#include "display.h"
#include "encoder.h"
#include "clock.h"
#include "settings.h"
#include "rgbled.h"
#include "logging.h"

#define SCL_PIN 6
#define SDA_PIN 5

#define RTC_SQW_PIN 43

// 32KB EEPROM
#define EEPROM_I2C_ADDR 0x57
#define RTC_I2C_ADDR 0x68

void setup() {
  Serial.begin(115200);

  delay(5000);
  
  // Initialize custom Log module - set to false to use Serial fallback
  Log::init(false);  
  Log::info("Starting Wake Clock...");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setBufferSize(512);
  Log::info("I2C initialized");

  // SCAN ALL I2C DEVICES
  Log::info("Checking for RTC");
  Wire.beginTransmission(RTC_I2C_ADDR);
  if (Wire.endTransmission() != 0) {
    Log::error("RTC not found!");
  } else {
    Log::info("RTC found!");
  }
  Log::info("Checking for Display");
  Wire.beginTransmission(0x70);
  if (Wire.endTransmission() != 0) {
    Log::error("Display not found!");
  } else {
    Log::info("Display found!");
  }

  if (!Settings::init()) {
    Log::error("Failed to initialize settings!");
  }

  Clock::init(RTC_SQW_PIN);
  Clock::enableSQWInterrupt();
  RgbLed::init();
  Display::init();
  Encoder::init();
  StateMachine::init();

  Clock::updateScheduleLED();
}

void loop() {
  Clock::update();  
  Action action = Encoder::getAction();
  StateMachine::processAction(action);
  delay(10);
}