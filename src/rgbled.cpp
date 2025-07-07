#include "rgbled.h"

#define RGB_RED_PIN 33
#define RGB_GREEN_PIN 32
#define RGB_BLUE_PIN 25

void RgbLed::init() {
    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);
    pinMode(RGB_BLUE_PIN, OUTPUT);
    analogWrite(RGB_RED_PIN, 0);
    analogWrite(RGB_GREEN_PIN, 0);
    analogWrite(RGB_BLUE_PIN, 0);
}

void RgbLed::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    analogWrite(RGB_RED_PIN, red);
    analogWrite(RGB_GREEN_PIN, green);
    analogWrite(RGB_BLUE_PIN, blue);
}

void RgbLed::turnOff() {
    analogWrite(RGB_RED_PIN, 0);
    analogWrite(RGB_GREEN_PIN, 0);
    analogWrite(RGB_BLUE_PIN, 0);
}
