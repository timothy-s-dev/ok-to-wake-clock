#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <Wire.h>

class Clock {
private:
    static const uint8_t RTC_ADDRESS = 0x68;
    static const uint8_t RTC_SECONDS_REG = 0x00;
    static const uint8_t RTC_MINUTES_REG = 0x01;
    static const uint8_t RTC_HOURS_REG = 0x02;
    static const uint8_t RTC_DAY_OF_WEEK_REG = 0x03;
    static const uint8_t RTC_DATE_REG = 0x04;
    static const uint8_t RTC_MONTH_REG = 0x05;
    static const uint8_t RTC_YEAR_REG = 0x06;
    static const uint8_t RTC_CONTROL_REG = 0x0E;
    
    static String timeString;
    static volatile bool timeCheckFlag;
    static int sqwPin;
    
    static uint8_t bcdToDecimal(uint8_t bcd);
    static uint8_t decimalToBcd(uint8_t decimal);
    static void checkAndUpdateTime();
    static void IRAM_ATTR sqwInterrupt();
    
public:
    static void init(int sqwPin);
    static void enableSQWInterrupt();
    static void disableSQWInterrupt();
    static String getTimeString();
    static void setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    static void update(); // Call this in main loop to check for time changes
    static uint8_t getCurrentHours();
    static uint8_t getCurrentMinutes();
    static uint16_t getMinutesSinceMidnight();
    static uint8_t getCurrentDate();
    static uint8_t getCurrentMonth();
    static uint16_t getCurrentYear();
    static uint8_t getCurrentDayOfWeek(); // 0=Sunday, 1=Monday, ..., 6=Saturday
    static void updateScheduleLED(); // Update RGB LED based on current schedule
    static bool startNap(uint16_t durationMinutes); // Start a nap with specified duration
};

#endif
