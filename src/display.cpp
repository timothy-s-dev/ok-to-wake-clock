#include "display.h"
#include "settings.h"
#include "logging.h"

#define DISPLAY_I2C_ADDR 0x70

HT16K33 display;

void Display::init() {
  if (display.begin() == false)
  {
    Log::error("Device did not acknowledge! Freezing.");
    while(1);
  }
  Log::info("Display acknowledged.");

  // Load saved brightness from settings
  uint8_t savedBrightness = Settings::getDisplayBrightness();
  display.setBrightness(savedBrightness);
}

HT16K33& Display::getInstance() {
    return display;
}

void Display::setBrightness(uint8_t brightness) {
    if (brightness > 15) brightness = 15; // Clamp to valid range
    display.setBrightness(brightness);
    Settings::setDisplayBrightness(brightness);
}