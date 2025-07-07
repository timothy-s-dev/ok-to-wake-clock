#include "clock.h"
#include "state_machine.h"
#include "settings.h"
#include "rgbled.h"
#include <Elog.h>
#include <logging.h>

// Static member definitions
String Clock::timeString = "0000";
volatile bool Clock::timeCheckFlag = false;
int Clock::sqwPin = -1;

void Clock::init(int pin) {
    sqwPin = pin;
    pinMode(sqwPin, INPUT_PULLUP);
    
    // Configure RTC to output 1Hz square wave on SQW pin
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_CONTROL_REG);
    Wire.write(0x10); // Enable SQW, 1Hz output
    Wire.endTransmission();
    
    // Read initial time
    checkAndUpdateTime();

    Logger.info(MAIN_LOG, "Clock initialized");
}

void Clock::enableSQWInterrupt() {
    if (sqwPin != -1) {
        attachInterrupt(digitalPinToInterrupt(sqwPin), sqwInterrupt, FALLING);
        Logger.info(MAIN_LOG, "SQW interrupt enabled");
    }
}

void Clock::disableSQWInterrupt() {
    if (sqwPin != -1) {
        detachInterrupt(digitalPinToInterrupt(sqwPin));
        Logger.info(MAIN_LOG, "SQW interrupt disabled");
    }
}

void IRAM_ATTR Clock::sqwInterrupt() {
    timeCheckFlag = true;
}

void Clock::update() {
    if (timeCheckFlag) {
        timeCheckFlag = false;
        checkAndUpdateTime();
    }
}

String Clock::getTimeString() {
    return timeString;
}

void Clock::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_SECONDS_REG);
    Wire.write(decimalToBcd(seconds));
    Wire.write(decimalToBcd(minutes));
    Wire.write(decimalToBcd(hours));
    Wire.endTransmission();
    
    // Update our local time string immediately
    checkAndUpdateTime();

    Logger.info(MAIN_LOG, "Time set to %02d:%02d:%02d", hours, minutes, seconds);
}

void Clock::checkAndUpdateTime() {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_SECONDS_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)3);
    
    if (Wire.available() >= 3) {
        uint8_t seconds = bcdToDecimal(Wire.read());
        uint8_t minutes = bcdToDecimal(Wire.read());
        uint8_t hours24 = bcdToDecimal(Wire.read());
        
        // Convert to 12-hour format
        uint8_t hours12 = hours24;
        if (hours12 == 0) {
            hours12 = 12; // Midnight case (00:xx becomes 12:xx AM)
        } else if (hours12 > 12) {
            hours12 -= 12; // PM case (13:xx becomes 1:xx PM, etc.)
        }
        
        // Format as HHMM string in 12-hour format
        char timeBuffer[4];
        sprintf(timeBuffer, "%02d%02d", hours12, minutes);
        if (timeBuffer[0] == '0') {
            timeBuffer[0] = ' ';
        }
        String newTimeString = String(timeBuffer);
        
        // Only update if the time string has changed
        if (newTimeString != timeString) {
            timeString = newTimeString;
            StateMachine::processAction(TIME_CHANGE);
            
            // Update RGB LED based on schedule when minute changes
            updateScheduleLED();
            
            // Logging
            const char* ampm = (hours24 < 12) ? "AM" : "PM";
            Logger.info(MAIN_LOG, "Time updated: %02d:%02d:%02d %s (String: %s)", 
                          hours12, minutes, seconds, ampm, timeString.c_str());
        }
    } else {
        Logger.error(MAIN_LOG, "Error reading from RTC");
    }
}

uint8_t Clock::bcdToDecimal(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t Clock::decimalToBcd(uint8_t decimal) {
    return ((decimal / 10) << 4) + (decimal % 10);
}

uint8_t Clock::getCurrentHours() {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_HOURS_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        return bcdToDecimal(Wire.read());
    }
    return 0;
}

uint8_t Clock::getCurrentMinutes() {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_MINUTES_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        return bcdToDecimal(Wire.read());
    }
    return 0;
}

uint8_t Clock::getCurrentDayOfWeek() {
    // Day of week is stored in register 0x03
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(0x03);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        uint8_t day = bcdToDecimal(Wire.read());
        // DS3231 uses 1=Sunday, 2=Monday, ... 7=Saturday
        // Convert to 0=Sunday, 1=Monday, ... 6=Saturday
        return (day == 0) ? 0 : day - 1;
    }
    return 0; // Default to Sunday
}

// Helper function to check current schedule phase and update RGB LED
void Clock::updateScheduleLED() {
    uint8_t currentHour = getCurrentHours();
    uint8_t currentMinute = getCurrentMinutes();
    uint8_t dayOfWeek = getCurrentDayOfWeek();
    
    // Load today's schedule
    DaySchedule todaySchedule;
    Settings::loadSchedule(static_cast<DayOfWeek>(dayOfWeek), todaySchedule);
    
    // Convert current time to minutes since midnight
    int currentMinutes = currentHour * 60 + currentMinute;
    
    // Convert schedule times to minutes since midnight
    int winddownStart = todaySchedule.winddownStartHour * 60 + todaySchedule.winddownStartMinute;
    int sleepStart = todaySchedule.sleepStartHour * 60 + todaySchedule.sleepStartMinute;
    int quietStart = todaySchedule.quietStartHour * 60 + todaySchedule.quietStartMinute;
    int wakeStart = todaySchedule.wakeStartHour * 60 + todaySchedule.wakeStartMinute;
    int wakeEnd = todaySchedule.wakeEndHour * 60 + todaySchedule.wakeEndMinute;

    Logger.info(MAIN_LOG, "Current time: %02d:%02d, Day: %d", currentHour, currentMinute, dayOfWeek);
    Logger.info(MAIN_LOG, "Schedule: Winddown %02d:%02d, Sleep %02d:%02d, Quiet %02d:%02d, Wake %02d:%02d - %02d:%02d",
                todaySchedule.winddownStartHour, todaySchedule.winddownStartMinute,
                todaySchedule.sleepStartHour, todaySchedule.sleepStartMinute,
                todaySchedule.quietStartHour, todaySchedule.quietStartMinute,
                todaySchedule.wakeStartHour, todaySchedule.wakeStartMinute,
                todaySchedule.wakeEndHour, todaySchedule.wakeEndMinute);
    
    // Handle day wrapping (e.g., if quiet time goes past midnight)
    bool inWinddown = false;
    bool inSleep = false;
    bool inQuiet = false;
    bool inWake = false;
    
    // Check winddown phase (between winddown start and sleep start)
    if (winddownStart <= sleepStart) {
        inWinddown = (currentMinutes >= winddownStart && currentMinutes < sleepStart);
    } else {
        // Wraps around midnight
        inWinddown = (currentMinutes >= winddownStart || currentMinutes < sleepStart);
    }
    
    // Check sleep phase (between sleep start and quiet start)
    if (sleepStart <= quietStart) {
        inSleep = (currentMinutes >= sleepStart && currentMinutes < quietStart);
    } else {
        // Wraps around midnight
        inSleep = (currentMinutes >= sleepStart || currentMinutes < quietStart);
    }
    
    // Check quiet phase (between quiet start and wake start)
    if (quietStart <= wakeStart) {
        inQuiet = (currentMinutes >= quietStart && currentMinutes < wakeStart);
    } else {
        // Wraps around midnight
        inQuiet = (currentMinutes >= quietStart || currentMinutes < wakeStart);
    }
    
    // Check wake phase (between wake start and wake end)
    if (wakeStart <= wakeEnd) {
        inWake = (currentMinutes >= wakeStart && currentMinutes < wakeEnd);
    } else {
        // Wraps around midnight
        inWake = (currentMinutes >= wakeStart || currentMinutes < wakeEnd);
    }

    Logger.info(MAIN_LOG, "Phases: Winddown=%d, Sleep=%d, Quiet=%d, Wake=%d\n", inWinddown, inSleep, inQuiet, inWake);
    
    // Set LED color based on current phase
    if (inWinddown) {
        RgbLed::setColor(0, 0, 255);
    } else if (inSleep) {
        RgbLed::setColor(255, 0, 0);
    } else if (inQuiet) {
        RgbLed::setColor(255, 255, 0);
    } else if (inWake) {
        RgbLed::setColor(0, 255, 0);
    } else {
        RgbLed::turnOff();
    }
}
