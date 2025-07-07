#include <Arduino.h>
#include <Wire.h>
#include "state_machine.h"
#include "display.h"
#include "encoder.h"
#include "clock.h"
#include "settings.h"
#include "rgbled.h"
#include <Elog.h>
#include <logging.h>

#define SCL_PIN 14
#define SDA_PIN 27

#define RTC_SQW_PIN 34
#define RTC_32K_PIN 35

// 32KB EEPROM
#define EEPROM_I2C_ADDR 0x57
#define RTC_I2C_ADDR 0x68

void setup() {
  Serial.begin(115200);
  
  Logger.configure(100, true);
  Logger.registerSerial(MAIN_LOG, ELOG_LEVEL_INFO, "MAIN", Serial);

  Logger.info(MAIN_LOG, "Starting Wake Clock...");

  Wire.begin(SDA_PIN, SCL_PIN);
  Logger.info(MAIN_LOG, "I2C initialized");

  if (!Settings::init()) {
    Logger.error(MAIN_LOG, "Failed to initialize settings!");
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