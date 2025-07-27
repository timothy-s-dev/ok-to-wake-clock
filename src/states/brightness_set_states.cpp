#include "states.h"
#include "state_machine.h"
#include "display.h"
#include "rgbled.h"
#include "settings.h"
#include "clock.h"
#include "logging.h"

// Static variables to store temporary brightness values
static uint8_t tempDisplayBrightness = 3;
static uint8_t tempColorBrightness = 128;

State SetDisplayBrightness = {
  .OnEnter = []() {
    tempDisplayBrightness = Settings::getDisplayBrightness();
    Display::getInstance().print("DISP");
    delay(1000);
    // Show current brightness level (0-15 mapped to 00-15)
    String brightnessStr = (tempDisplayBrightness < 10) ? "0" + String(tempDisplayBrightness) : String(tempDisplayBrightness);
    Display::getInstance().print(brightnessStr);
  },
  .OnExit = []() { Display::getInstance().clear(); },
  .OnClockwise = []() { 
    if (tempDisplayBrightness < 15) {
      tempDisplayBrightness++;
      Display::setBrightness(tempDisplayBrightness);
      String brightnessStr = (tempDisplayBrightness < 10) ? "0" + String(tempDisplayBrightness) : String(tempDisplayBrightness);
      Display::getInstance().print(brightnessStr);
    }
  },
  .OnCounterClockwise = []() { 
    if (tempDisplayBrightness > 0) {
      tempDisplayBrightness--;
      Display::setBrightness(tempDisplayBrightness);
      String brightnessStr = (tempDisplayBrightness < 10) ? "0" + String(tempDisplayBrightness) : String(tempDisplayBrightness);
      Display::getInstance().print(brightnessStr);
    }
  },
  .OnSelect = []() { 
    // Brightness is already saved in real-time, just move to color brightness
    StateMachine::setState(&SetColorBrightness);
  },
  .OnSelectHold = []() { /* Do nothing */ }
};

State SetColorBrightness = {
  .OnEnter = []() {
     // Convert to percentage, rounded down to nearest 5
    tempColorBrightness = (Settings::getLedBrightness() * 100 / 255) / 5 * 5;
    Display::getInstance().print("LED");
    delay(1000);
    String brightnessStr = (tempColorBrightness < 10) ? "  " + String(tempColorBrightness) : (tempColorBrightness < 100) ? " " + String(tempColorBrightness) : String(tempColorBrightness);
    Display::getInstance().print(brightnessStr + "%");
    RgbLed::indicateStatus(WAKE);
  },
  .OnExit = []() { 
    Display::getInstance().clear();
  },
  .OnClockwise = []() { 
    if (tempColorBrightness < 100) {
      tempColorBrightness = (tempColorBrightness + 5 > 100) ? 100 : tempColorBrightness + 5;
      RgbLed::setBrightness(tempColorBrightness * 255 / 100); // Convert back to 0-255
      String brightnessStr = (tempColorBrightness < 10) ? "  " + String(tempColorBrightness) : (tempColorBrightness < 100) ? " " + String(tempColorBrightness) : String(tempColorBrightness);
      Display::getInstance().print(brightnessStr + "%");
      RgbLed::indicateStatus(WAKE);
    }
  },
  .OnCounterClockwise = []() { 
    if (tempColorBrightness > 0) {
      tempColorBrightness = (tempColorBrightness < 5) ? 0 : tempColorBrightness - 5;
      RgbLed::setBrightness(tempColorBrightness * 255 / 100);
      String brightnessStr = (tempColorBrightness < 10) ? "  " + String(tempColorBrightness) : (tempColorBrightness < 100) ? " " + String(tempColorBrightness) : String(tempColorBrightness);
      Display::getInstance().print(brightnessStr + "%");
      RgbLed::indicateStatus(WAKE);
    }
  },
  .OnSelect = []() { 
    Clock::updateScheduleLED();
    StateMachine::setState(&Clock);
  },
  .OnSelectHold = []() { /* Do nothing */ }
};
