#include "schedule.h"
#include "clock.h"
#include <array>

Schedule::Schedule() {
    // Default schedule times converted to minutes since midnight
    winddownStart = 19 * 60 + 45;  // 19:45 = 1185 minutes
    sleepStart = 20 * 60 + 0;      // 20:00 = 1200 minutes
    quietStart = 7 * 60 + 15;      // 07:15 = 435 minutes
    wakeStart = 7 * 60 + 30;       // 07:30 = 450 minutes
    wakeEnd = 7 * 60 + 45;         // 07:45 = 465 minutes
}

Schedule Schedule::getNap(uint8_t duration) {
    Schedule napSchedule;
    
    uint16_t currentMinutes = Clock::getMinutesSinceMidnight();

    napSchedule.winddownStart = currentMinutes - 15;
    napSchedule.sleepStart = currentMinutes;
    napSchedule.quietStart = currentMinutes + duration;
    napSchedule.wakeStart = currentMinutes + duration + 15;
    napSchedule.wakeEnd = currentMinutes + duration + 30;

    return napSchedule;
}

Schedule Schedule::fromByteArray(const std::array<uint8_t, 10>& data) {
    Schedule schedule;
    
    // Convert hour/minute pairs back to minutes since midnight
    schedule.winddownStart = data[0] * 60 + data[1];
    schedule.sleepStart = data[2] * 60 + data[3];
    schedule.quietStart = data[4] * 60 + data[5];
    schedule.wakeStart = data[6] * 60 + data[7];
    schedule.wakeEnd = data[8] * 60 + data[9];
    
    return schedule;
}

std::array<uint8_t, 10> Schedule::convertToByteArray() const {
    std::array<uint8_t, 10> data = {
        static_cast<uint8_t>(winddownStart / 60),   // hour
        static_cast<uint8_t>(winddownStart % 60),   // minute
        static_cast<uint8_t>(sleepStart / 60),      // hour
        static_cast<uint8_t>(sleepStart % 60),      // minute
        static_cast<uint8_t>(quietStart / 60),      // hour
        static_cast<uint8_t>(quietStart % 60),      // minute
        static_cast<uint8_t>(wakeStart / 60),       // hour
        static_cast<uint8_t>(wakeStart % 60),       // minute
        static_cast<uint8_t>(wakeEnd / 60),         // hour
        static_cast<uint8_t>(wakeEnd % 60)          // minute
    };
    
    return data;
}

void Schedule::setWinddownStart(uint8_t hour, uint8_t minute) {
    winddownStart = hour * 60 + minute;
}

void Schedule::setSleepStart(uint8_t hour, uint8_t minute) {
    sleepStart = hour * 60 + minute;
}

void Schedule::setQuietStart(uint8_t hour, uint8_t minute) {
    quietStart = hour * 60 + minute;
}

void Schedule::setWakeStart(uint8_t hour, uint8_t minute) {
    wakeStart = hour * 60 + minute;
}

void Schedule::setWakeEnd(uint8_t hour, uint8_t minute) {
    wakeEnd = hour * 60 + minute;
}

bool Schedule::isActive() const {
    uint16_t currentMinutes = Clock::getMinutesSinceMidnight();

    if (winddownStart <= wakeEnd) {
        return (currentMinutes >= winddownStart && currentMinutes < wakeEnd);
    } else {
        if (currentMinutes < wakeEnd) {
            return (currentMinutes >= winddownStart || currentMinutes < wakeEnd);
        }
    }
    return (currentMinutes >= winddownStart && currentMinutes < wakeEnd);
}

ScheduleBlock Schedule::getCurrentBlock() const {
    uint16_t currentMinutes = Clock::getMinutesSinceMidnight();

    // Helper function to check if current time is within a range, handling day wrapping
    auto isInRange = [](uint16_t current, uint16_t start, uint16_t end) -> bool {
        if (start <= end) {
            // Normal case: no day wrapping
            return current >= start && current < end;
        } else {
            // Day wrapping case: range crosses midnight
            return current >= start || current < end;
        }
    };
    
    // Check each schedule block in order
    if (isInRange(currentMinutes, winddownStart, sleepStart)) {
        return WIND_DOWN;
    } else if (isInRange(currentMinutes, sleepStart, quietStart)) {
        return SLEEP;
    } else if (isInRange(currentMinutes, quietStart, wakeStart)) {
        return QUIET;
    } else if (isInRange(currentMinutes, wakeStart, wakeEnd)) {
        return WAKE;
    }

    // If not in any schedule block, return NO_BLOCK as default
    // (this would be outside the wake period)
    return NO_BLOCK;
}
