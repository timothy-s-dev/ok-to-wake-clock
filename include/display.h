#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_Alphanumeric_Display.h>

class Display {
public:
    static void init();
    static HT16K33& getInstance();
};

#endif // DISPLAY_H