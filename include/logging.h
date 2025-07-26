#ifndef LOG_H
#define LOG_H

#define MAIN_LOG 0

#include <Arduino.h>

class Log {
public:
    // Configuration
    static void init(bool useElog = true);
    static void setUseElog(bool enable);
    
    // Logging methods
    static void info(const char* format, ...);
    static void error(const char* format, ...);
    static void warning(const char* format, ...);
    static void debug(const char* format, ...);
    static void notice(const char* format, ...);
    
private:
    static bool useElog;
    static bool initialized;
    static void logMessage(const char* level, const char* format, va_list args);
    static void logWithElog(uint8_t elogLevel, const char* format, va_list args);
    static void logWithSerial(const char* level, const char* format, va_list args);
};

#endif
