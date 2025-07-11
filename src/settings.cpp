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

bool Settings::saveSchedule(DayOfWeek day, const DaySchedule& schedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    String key = getDayKey(day);
    
    // Store the schedule as a byte array
    uint8_t scheduleData[10] = {
        schedule.winddownStartHour,
        schedule.winddownStartMinute,
        schedule.sleepStartHour,
        schedule.sleepStartMinute,
        schedule.quietStartHour,
        schedule.quietStartMinute,
        schedule.wakeStartHour,
        schedule.wakeStartMinute,
        schedule.wakeEndHour,
        schedule.wakeEndMinute
    };
    
    size_t bytesWritten = preferences.putBytes(key.c_str(), scheduleData, sizeof(scheduleData));
    
    if (bytesWritten != sizeof(scheduleData)) {
        Logger.error(MAIN_LOG, "Failed to save schedule for day %d", day);
        return false;
    }

    Logger.info(MAIN_LOG, "Schedule saved for day %d", day);
    return true;
}

bool Settings::loadSchedule(DayOfWeek day, DaySchedule& schedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    String key = getDayKey(day);
    uint8_t scheduleData[10];
    
    size_t bytesRead = preferences.getBytes(key.c_str(), scheduleData, sizeof(scheduleData));
    
    if (bytesRead != sizeof(scheduleData)) {
        Logger.error(MAIN_LOG, "Failed to load schedule for day %d, using defaults", day);
        schedule = getDefaultSchedule();
        return false;
    }
    
    // Unpack the data into the schedule structure
    schedule.winddownStartHour = scheduleData[0];
    schedule.winddownStartMinute = scheduleData[1];
    schedule.sleepStartHour = scheduleData[2];
    schedule.sleepStartMinute = scheduleData[3];
    schedule.quietStartHour = scheduleData[4];
    schedule.quietStartMinute = scheduleData[5];
    schedule.wakeStartHour = scheduleData[6];
    schedule.wakeStartMinute = scheduleData[7];
    schedule.wakeEndHour = scheduleData[8];
    schedule.wakeEndMinute = scheduleData[9];
    
    return true;
}

bool Settings::loadAllSchedules(DaySchedule schedules[7]) {
    bool allSuccess = true;
    
    for (int day = 0; day < 7; day++) {
        if (!loadSchedule(static_cast<DayOfWeek>(day), schedules[day])) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool Settings::saveAllSchedules(const DaySchedule schedules[7]) {
    bool allSuccess = true;
    
    for (int day = 0; day < 7; day++) {
        if (!saveSchedule(static_cast<DayOfWeek>(day), schedules[day])) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool Settings::resetSchedule(DayOfWeek day) {
    DaySchedule defaultSchedule = getDefaultSchedule();
    return saveSchedule(day, defaultSchedule);
}

bool Settings::resetAllSchedules() {
    bool allSuccess = true;
    DaySchedule defaultSchedule = getDefaultSchedule();
    
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

DaySchedule Settings::getDefaultSchedule() {
    DaySchedule defaultSchedule;
    
    // Default schedule: 
    // Winddown starts at 19:45 (7:45 PM)
    // Sleep starts at 20:00 (8:00 PM)
    // Quiet hours start at 07:15 (7:15 AM)
    // Wake starts at 7:30 (7:30 AM)
    // Wake ends at 7:45 (7:45 AM)
    defaultSchedule.winddownStartHour = 19;
    defaultSchedule.winddownStartMinute = 45;
    defaultSchedule.sleepStartHour = 20;
    defaultSchedule.sleepStartMinute = 0;
    defaultSchedule.quietStartHour = 7;
    defaultSchedule.quietStartMinute = 15;
    defaultSchedule.wakeStartHour = 7;
    defaultSchedule.wakeStartMinute = 30;
    defaultSchedule.wakeEndHour = 7;
    defaultSchedule.wakeEndMinute = 45;
    
    return defaultSchedule;
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
    DaySchedule defaultSchedule = getDefaultSchedule();
    
    // Initialize all days with the same default schedule
    for (int day = 0; day < 7; day++) {
        saveSchedule(static_cast<DayOfWeek>(day), defaultSchedule);
    }

    // Initialize nap schedule as inactive
    NapSchedule defaultNap;
    memset(&defaultNap, 0, sizeof(defaultNap));
    defaultNap.active = false;
    saveNapSchedule(defaultNap);

    Logger.info(MAIN_LOG, "Default schedules initialized for all days");
}

bool Settings::saveNapSchedule(const NapSchedule& napSchedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    // Store the nap schedule as a byte array (10 bytes for times + 1 byte for active flag)
    uint8_t napData[11] = {
        napSchedule.winddownStartHour,
        napSchedule.winddownStartMinute,
        napSchedule.sleepStartHour,
        napSchedule.sleepStartMinute,
        napSchedule.quietStartHour,
        napSchedule.quietStartMinute,
        napSchedule.wakeStartHour,
        napSchedule.wakeStartMinute,
        napSchedule.wakeEndHour,
        napSchedule.wakeEndMinute,
        napSchedule.active ? 1 : 0
    };
    
    size_t bytesWritten = preferences.putBytes("nap_schedule", napData, sizeof(napData));
    
    if (bytesWritten != sizeof(napData)) {
        Logger.error(MAIN_LOG, "Failed to save nap schedule");
        return false;
    }

    Logger.info(MAIN_LOG, "Nap schedule saved (active: %s)", napSchedule.active ? "true" : "false");
    return true;
}

bool Settings::loadNapSchedule(NapSchedule& napSchedule) {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    uint8_t napData[11];
    
    size_t bytesRead = preferences.getBytes("nap_schedule", napData, sizeof(napData));
    
    if (bytesRead != sizeof(napData)) {
        Logger.warning(MAIN_LOG, "Failed to load nap schedule, using defaults");
        memset(&napSchedule, 0, sizeof(napSchedule));
        napSchedule.active = false;
        return false;
    }
    
    // Unpack the data into the nap schedule structure
    napSchedule.winddownStartHour = napData[0];
    napSchedule.winddownStartMinute = napData[1];
    napSchedule.sleepStartHour = napData[2];
    napSchedule.sleepStartMinute = napData[3];
    napSchedule.quietStartHour = napData[4];
    napSchedule.quietStartMinute = napData[5];
    napSchedule.wakeStartHour = napData[6];
    napSchedule.wakeStartMinute = napData[7];
    napSchedule.wakeEndHour = napData[8];
    napSchedule.wakeEndMinute = napData[9];
    napSchedule.active = (napData[10] != 0);
    
    return true;
}

bool Settings::stopNap() {
    if (!initialized) {
        Logger.error(MAIN_LOG, "Settings not initialized");
        return false;
    }
    
    NapSchedule napSchedule;
    if (!loadNapSchedule(napSchedule)) {
        Logger.warning(MAIN_LOG, "Could not load nap schedule to stop");
        return false;
    }
    
    napSchedule.active = false;
    Logger.info(MAIN_LOG, "Stopping nap");
    
    return saveNapSchedule(napSchedule);
}

bool Settings::isNapActive() {
    if (!initialized) {
        return false;
    }
    
    NapSchedule napSchedule;
    if (!loadNapSchedule(napSchedule)) {
        return false;
    }
    
    return napSchedule.active;
}
