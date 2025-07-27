#include "settings.h"
#include "schedule.h"
#include "logging.h"
#include <Preferences.h>
#include <cstring>

// Static member definitions
Preferences Settings::preferences;
bool Settings::initialized = false;

bool Settings::init() {
    if (initialized) {
        return true;
    }
    
    // Open preferences in read-write mode
    if (!preferences.begin("wake-clock", false)) {
        Log::error("Failed to initialize preferences");
        return false;
    }
    
    // Check if this is the first time initialization
    if (!preferences.getBool("initialized", false)) {
        Log::info("First time setup - initializing default schedules");
        initializeDefaults();
        preferences.putBool("initialized", true);
    }
    
    initialized = true;
    Log::info("Settings initialized successfully");
    return true;
}

bool Settings::saveSchedule(DayOfWeek day, const Schedule& schedule) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    String key = getDayKey(day);
    
    // Convert schedule to byte array
    std::array<uint8_t, 10> scheduleData = schedule.convertToByteArray();
    
    size_t bytesWritten = preferences.putBytes(key.c_str(), scheduleData.data(), scheduleData.size());
    
    if (bytesWritten != scheduleData.size()) {
        Log::error("Failed to save schedule for day %d", day);
        return false;
    }

    Log::info("Schedule saved for day %d", day);
    return true;
}

bool Settings::loadSchedule(DayOfWeek day, Schedule& schedule) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    String key = getDayKey(day);
    uint8_t scheduleData[10];
    
    size_t bytesRead = preferences.getBytes(key.c_str(), scheduleData, sizeof(scheduleData));
    
    if (bytesRead != sizeof(scheduleData)) {
        Log::error("Failed to load schedule for day %d, using defaults", day);
        schedule = Schedule();
        return false;
    }
    
    // Convert byte array to Schedule object
    std::array<uint8_t, 10> dataArray;
    std::copy(scheduleData, scheduleData + 10, dataArray.begin());
    schedule = Schedule::fromByteArray(dataArray);
    
    return true;
}

bool Settings::loadAllSchedules(Schedule schedules[7]) {
    bool allSuccess = true;
    
    for (int day = 0; day < 7; day++) {
        if (!loadSchedule(static_cast<DayOfWeek>(day), schedules[day])) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool Settings::saveAllSchedules(const Schedule schedules[7]) {
    bool allSuccess = true;
    
    for (int day = 0; day < 7; day++) {
        if (!saveSchedule(static_cast<DayOfWeek>(day), schedules[day])) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool Settings::resetSchedule(DayOfWeek day) {
    Schedule defaultSchedule = getDefaultSchedule();
    return saveSchedule(day, defaultSchedule);
}

bool Settings::resetAllSchedules() {
    bool allSuccess = true;
    Schedule defaultSchedule = getDefaultSchedule();
    
    for (int day = 0; day < 7; day++) {
        if (!saveSchedule(static_cast<DayOfWeek>(day), defaultSchedule)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool Settings::isInitialized() {
    return initialized && preferences.getBool("initialized", false);
}

void Settings::close() {
    if (initialized) {
        preferences.end();
        initialized = false;
        Log::info("Settings closed");
    }
}

String Settings::getDayKey(DayOfWeek day) {
    const char* dayNames[] = {
        "sunday", "monday", "tuesday", "wednesday", 
        "thursday", "friday", "saturday"
    };
    
    if (day >= 0 && day <= 6) {
        return String("sched_") + dayNames[day];
    }
    
    return String("sched_invalid");
}

void Settings::initializeDefaults() {
    Schedule defaultSchedule;
    
    // Initialize all days with the same default schedule
    for (int day = 0; day < 7; day++) {
        saveSchedule(static_cast<DayOfWeek>(day), defaultSchedule);
    }

    // Initialize nap schedule as inactive
    Schedule defaultNap = Schedule::getNap(30); // Default nap duration of 30 minutes
    saveNapSchedule(defaultSchedule);
    setNapEnabled(false);

    Log::info("Default schedules initialized for all days");
}

bool Settings::saveNapSchedule(const Schedule& napSchedule) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    // Convert schedule to byte array
    std::array<uint8_t, 10> napData = napSchedule.convertToByteArray();
    
    size_t bytesWritten = preferences.putBytes("nap_schedule", napData.data(), napData.size());
    
    if (bytesWritten != napData.size()) {
        Log::error("Failed to save nap schedule");
        return false;
    }

    Log::info("Nap schedule saved");
    return true;
}

bool Settings::loadNapSchedule(Schedule& napSchedule) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    uint8_t napData[10];
    
    size_t bytesRead = preferences.getBytes("nap_schedule", napData, sizeof(napData));
    
    if (bytesRead != sizeof(napData)) {
        Log::warning("Failed to load nap schedule, using defaults");
        napSchedule = Schedule(); // Default schedule
        return false;
    }
    
    // Convert byte array to Schedule object
    std::array<uint8_t, 10> dataArray;
    std::copy(napData, napData + 10, dataArray.begin());
    napSchedule = Schedule::fromByteArray(dataArray);
    
    return true;
}

bool Settings::setNapEnabled(bool enabled) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    preferences.putBool("nap_schedule_enabled", enabled);
    Log::info("Nap enabled state set to: %s", enabled ? "true" : "false");
    return true;
}

bool Settings::isNapEnabled() {
    if (!initialized) {
        return false;
    }
    
    return preferences.getBool("nap_schedule_enabled", false);
}

bool Settings::startNap(uint16_t durationMinutes) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    // This function is now deprecated in favor of Clock::startNap
    // which has access to the current time
    Log::warning("Settings::startNap is deprecated, use Clock::startNap instead");
    return false;
}

bool Settings::stopNap() {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    Log::info("Stopping nap");
    return setNapEnabled(false);
}

bool Settings::setLocked(bool locked) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    preferences.putBool("device_locked", locked);
    Log::info("Device lock state set to: %s", locked ? "true" : "false");
    return true;
}

bool Settings::isLocked() {
    if (!initialized) {
        return false;
    }
    
    return preferences.getBool("device_locked", false);
}

bool Settings::setDisplayBrightness(uint8_t brightness) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    // Clamp brightness to valid range (0-15 for HT16K33)
    if (brightness > 15) brightness = 15;
    
    preferences.putUChar("display_lvl", brightness);
    Log::info("Display brightness set to: %d", brightness);
    return true;
}

uint8_t Settings::getDisplayBrightness() {
    if (!initialized) {
        return 3; // Default brightness
    }
    
    return preferences.getUChar("display_lvl", 3);
}

bool Settings::setLedBrightness(uint8_t brightness) {
    if (!initialized) {
        Log::error("Settings not initialized");
        return false;
    }
    
    preferences.putUChar("led_lvl", brightness);
    Log::info("LED brightness set to: %d", brightness);
    return true;
}

uint8_t Settings::getLedBrightness() {
    if (!initialized) {
        return 128; // Default brightness (50% of 255)
    }
    
    return preferences.getUChar("led_lvl", 128);
}
