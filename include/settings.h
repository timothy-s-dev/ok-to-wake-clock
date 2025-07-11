#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

// Schedule structure for each day of the week
struct DaySchedule {
    uint8_t winddownStartHour;      // 0-23
    uint8_t winddownStartMinute;    // 0-59
    uint8_t sleepStartHour;         // 0-23
    uint8_t sleepStartMinute;       // 0-59
    uint8_t quietStartHour;         // 0-23
    uint8_t quietStartMinute;       // 0-59
    uint8_t wakeStartHour;          // 0-23
    uint8_t wakeStartMinute;        // 0-59
    uint8_t wakeEndHour;            // 0-23
    uint8_t wakeEndMinute;          // 0-59
};

// Nap schedule structure (uses same format as day schedules)
struct NapSchedule {
    uint8_t winddownStartHour;      // 0-23
    uint8_t winddownStartMinute;    // 0-59
    uint8_t sleepStartHour;         // 0-23
    uint8_t sleepStartMinute;       // 0-59
    uint8_t quietStartHour;         // 0-23
    uint8_t quietStartMinute;       // 0-59
    uint8_t wakeStartHour;          // 0-23
    uint8_t wakeStartMinute;        // 0-59
    uint8_t wakeEndHour;            // 0-23
    uint8_t wakeEndMinute;          // 0-59
    bool active;                    // Whether nap is currently active
};

// Days of the week enum
enum DayOfWeek {
    SUNDAY = 0,
    MONDAY = 1,
    TUESDAY = 2,
    WEDNESDAY = 3,
    THURSDAY = 4,
    FRIDAY = 5,
    SATURDAY = 6
};

class Settings {
public:
    // Initialize the settings module
    static bool init();
    
    // Save a schedule for a specific day
    static bool saveSchedule(DayOfWeek day, const DaySchedule& schedule);
    
    // Load a schedule for a specific day
    static bool loadSchedule(DayOfWeek day, DaySchedule& schedule);
    
    // Load all schedules into an array
    static bool loadAllSchedules(DaySchedule schedules[7]);
    
    // Save all schedules from an array
    static bool saveAllSchedules(const DaySchedule schedules[7]);
    
    // Reset a specific day's schedule to defaults
    static bool resetSchedule(DayOfWeek day);
    
    // Reset all schedules to defaults
    static bool resetAllSchedules();
    
    // Check if settings have been initialized (first time setup)
    static bool isInitialized();
    
    // Get default schedule (can be used for initial setup)
    static DaySchedule getDefaultSchedule();
    
    // Nap-related functions
    static bool saveNapSchedule(const NapSchedule& napSchedule);
    static bool loadNapSchedule(NapSchedule& napSchedule);
    static bool startNap(uint16_t durationMinutes); // Start a nap with given duration
    static bool stopNap(); // Stop the current nap
    static bool isNapActive(); // Check if a nap is currently active
    
    // Close preferences (call when shutting down)
    static void close();

private:
    static Preferences preferences;
    static bool initialized;
    
    // Helper function to get the key name for a specific day
    static String getDayKey(DayOfWeek day);
    
    // Initialize with default schedules if first time
    static void initializeDefaults();
};

#endif // SETTINGS_H
