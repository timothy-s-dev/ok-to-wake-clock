#include "settings.h"
#include <Elog.h>
#include <logging.h>
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
        Logger.error(MAIN_LOG, "Failed to initialize preferences");
        return false;
    }
    
    // Check if this is the first time initialization
    if (!preferences.getBool("initialized", false)) {
        Logger.info(MAIN_LOG, "First time setup - initializing default schedules");
        initializeDefaults();
        preferences.putBool("initialized", true);
    }
    
    initialized = true;
    Logger.info(MAIN_LOG, "Settings initialized successfully");
    return true;
}

bool Settings::saveSchedule(DayOfWeek day, const Schedule& schedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    String key = getDayKey(day);
    
    // Convert schedule to byte array
    std::array<uint8_t, 10> scheduleData = schedule.convertToByteArray();
    
    size_t bytesWritten = preferences.putBytes(key.c_str(), scheduleData.data(), scheduleData.size());
    
    if (bytesWritten != scheduleData.size()) {
        Logger.error(MAIN_LOG, "Failed to save schedule for day %d", day);
        return false;
    }

    Logger.info(MAIN_LOG, "Schedule saved for day %d", day);
    return true;
}

bool Settings::loadSchedule(DayOfWeek day, Schedule& schedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    String key = getDayKey(day);
    uint8_t scheduleData[10];
    
    size_t bytesRead = preferences.getBytes(key.c_str(), scheduleData, sizeof(scheduleData));
    
    if (bytesRead != sizeof(scheduleData)) {
        Logger.error(MAIN_LOG, "Failed to load schedule for day %d, using defaults", day);
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
        Logger.info(MAIN_LOG, "Settings closed");
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

    Logger.info(MAIN_LOG, "Default schedules initialized for all days");
}

bool Settings::saveNapSchedule(const Schedule& napSchedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    // Convert schedule to byte array
    std::array<uint8_t, 10> napData = napSchedule.convertToByteArray();
    
    size_t bytesWritten = preferences.putBytes("nap_schedule", napData.data(), napData.size());
    
    if (bytesWritten != napData.size()) {
        Logger.error(MAIN_LOG, "Failed to save nap schedule");
        return false;
    }

    Logger.info(MAIN_LOG, "Nap schedule saved");
    return true;
}

bool Settings::loadNapSchedule(Schedule& napSchedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    uint8_t napData[10];
    
    size_t bytesRead = preferences.getBytes("nap_schedule", napData, sizeof(napData));
    
    if (bytesRead != sizeof(napData)) {
        Logger.warning(MAIN_LOG, "Failed to load nap schedule, using defaults");
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
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    preferences.putBool("nap_schedule_enabled", enabled);
    Logger.info(MAIN_LOG, "Nap enabled state set to: %s", enabled ? "true" : "false");
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
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    // This function is now deprecated in favor of Clock::startNap
    // which has access to the current time
    Logger.warning(MAIN_LOG, "Settings::startNap is deprecated, use Clock::startNap instead");
    return false;
}

bool Settings::stopNap() {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    Logger.info(MAIN_LOG, "Stopping nap");
    return setNapEnabled(false);
}
