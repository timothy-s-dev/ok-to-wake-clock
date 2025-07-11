#include "clock.h"
#include "state_machine.h"
#include "settings.h"
#include "rgbled.h"
#include "schedule.h"
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

uint16_t Clock::getMinutesSinceMidnight() {
    uint16_t hours = getCurrentHours();
    uint16_t minutes = getCurrentMinutes();
    return hours * 60 + minutes;
}

uint8_t Clock::getCurrentDayOfWeek() {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_DAY_OF_WEEK_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        uint8_t day = bcdToDecimal(Wire.read());
        return day;
    }
    return 0; // Default to Sunday
}

uint8_t Clock::getCurrentDate() {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_DATE_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        return bcdToDecimal(Wire.read());
    }
    return 1; // Default to 1st of month
}

uint8_t Clock::getCurrentMonth() {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_MONTH_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        uint8_t monthByte = Wire.read();
        // Mask out the century bit (bit 7) and convert BCD to decimal
        return bcdToDecimal(monthByte & 0x7F);
    }
    return 1; // Default to January
}

uint16_t Clock::getCurrentYear() {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(RTC_YEAR_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(RTC_ADDRESS, (uint8_t)1);
    if (Wire.available()) {
        uint8_t yearByte = bcdToDecimal(Wire.read());
        // DS3231 stores only 2-digit year, add 2000 to get full year
        return 2000 + yearByte;
    }
    return 2025; // Default to current year
}

// Helper function to check current schedule phase and update RGB LED
void Clock::updateScheduleLED() {
    uint8_t currentHour = getCurrentHours();
    uint8_t currentMinute = getCurrentMinutes();
    uint8_t dayOfWeek = getCurrentDayOfWeek();
    
    // Convert current time to minutes since midnight
    int currentMinutes = currentHour * 60 + currentMinute;
    
    // Check if there's an active nap first - naps take priority
    Schedule napSchedule;
    bool hasActiveNap = Settings::isNapEnabled() && Settings::loadNapSchedule(napSchedule);
    
    if (hasActiveNap) {
        Logger.info(MAIN_LOG, "Active nap detected, using nap schedule");
        
        // Use the Schedule class's getCurrentBlock method to determine phase
        ScheduleBlock currentBlock = napSchedule.getCurrentBlock();
        
        Logger.info(MAIN_LOG, "Nap Schedule: Current block = %d", currentBlock);
                    
        // Set LED color based on nap phase
        RgbLed::indicateStatus(currentBlock);
        if (currentBlock == NO_BLOCK) {
            // If nap is over, deactivate it
            Logger.info(MAIN_LOG, "Nap period ended, deactivating nap");
            Settings::stopNap();
            RgbLed::turnOff();
            return; // Exit early since nap is over
        }
        
        return; // Exit early since we're using nap schedule
    }
    
    // No active nap, use regular daily schedule
    Schedule todaySchedule;
    Settings::loadSchedule(static_cast<DayOfWeek>(dayOfWeek), todaySchedule);
    
    // Use the Schedule class's getCurrentBlock method
    ScheduleBlock currentBlock = todaySchedule.getCurrentBlock();

    Logger.info(MAIN_LOG, "Current time: %02d:%02d, Day: %d", currentHour, currentMinute, dayOfWeek);
    Logger.info(MAIN_LOG, "Daily Schedule: Current block = %d", currentBlock);
    
    RgbLed::indicateStatus(currentBlock);
}

bool Clock::startNap(uint16_t durationMinutes) {
    uint8_t currentHour = getCurrentHours();
    uint8_t currentMinute = getCurrentMinutes();
    
    Logger.info(MAIN_LOG, "Starting nap at %02d:%02d for %d minutes", currentHour, currentMinute, durationMinutes);
    
    // Use the Schedule class to create a nap schedule
    Schedule napSchedule = Schedule::getNap(durationMinutes);
    
    // Save the nap schedule and enable it
    bool success = Settings::saveNapSchedule(napSchedule) && Settings::setNapEnabled(true);
    
    if (success) {
        Logger.info(MAIN_LOG, "Nap schedule created and enabled for %d minutes", durationMinutes);
        
        // Update LED immediately
        updateScheduleLED();
    }
    
    return success;
}
