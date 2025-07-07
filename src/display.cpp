#include "display.h"
#include <Elog.h>
#include <logging.h>

#define DISPLAY_I2C_ADDR 0x70

HT16K33 display;

void Display::init() {
  if (display.begin() == false)
  {
    Logger.error(MAIN_LOG, "Device did not acknowledge! Freezing.");
    while(1);
  }
  Logger.info(MAIN_LOG, "Display acknowledged.");

  display.setBrightness(3);
}

HT16K33& Display::getInstance() {
    return display;
}