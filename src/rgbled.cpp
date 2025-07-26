#include "rgbled.h"
#include <schedule.h>
#include <Adafruit_NeoPixel.h>

#define DATA_PIN 44
#define NUMPIXELS 16

Adafruit_NeoPixel pixels(NUMPIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

void RgbLed::init() {
    pixels.begin();
    pixels.show(); // Initialize all pixels to 'off'
}

void RgbLed::indicateStatus(ScheduleBlock scheduleBlock) {
    switch (scheduleBlock) {
        case WIND_DOWN:
            setColor(0, 0, 255); // Blue
            break;
        case SLEEP:
            setColor(255, 0, 0); // Red
            break;
        case QUIET:
            setColor(255, 255, 0); // Yellow
            break;
        case WAKE:
            setColor(0, 255, 0); // Green
            break;
        case NO_BLOCK:
            turnOff();
            break;
    }
}

void RgbLed::turnOff() {
    pixels.clear();
    pixels.show();
}

void RgbLed::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
    }
    pixels.show();
}
