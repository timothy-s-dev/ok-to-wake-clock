#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>
#include "schedule.h"

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
    static bool saveSchedule(DayOfWeek day, const Schedule& schedule);
    
    // Load a schedule for a specific day
    static bool loadSchedule(DayOfWeek day, Schedule& schedule);
    
    // Load all schedules into an array
    static bool loadAllSchedules(Schedule schedules[7]);
    
    // Save all schedules from an array
    static bool saveAllSchedules(const Schedule schedules[7]);
    
    // Reset a specific day's schedule to defaults
    static bool resetSchedule(DayOfWeek day);
    
    // Reset all schedules to defaults
    static bool resetAllSchedules();
    
    // Check if settings have been initialized (first time setup)
    static bool isInitialized();
    
    // Get default schedule (can be used for initial setup)
    static Schedule getDefaultSchedule();
    
    // Nap-related functions
    static bool saveNapSchedule(const Schedule& napSchedule);
    static bool loadNapSchedule(Schedule& napSchedule);
    static bool setNapEnabled(bool enabled); // Enable/disable nap
    static bool isNapEnabled(); // Check if nap is enabled
    static bool startNap(uint16_t durationMinutes); // Start a nap with given duration
    static bool stopNap(); // Stop the current nap
    
    // Lock mode functions
    static bool setLocked(bool locked); // Set lock mode state
    static bool isLocked(); // Check if device is locked
    
    // Brightness functions
    static bool setDisplayBrightness(uint8_t brightness); // Set display brightness (0-15)
    static uint8_t getDisplayBrightness(); // Get display brightness
    static bool setLedBrightness(uint8_t brightness); // Set RGB LED brightness (0-255)
    static uint8_t getLedBrightness(); // Get RGB LED brightness
    
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
