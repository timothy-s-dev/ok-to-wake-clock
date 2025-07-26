#include "logging.h"
#include <Elog.h>
#include <logging.h>

// Static member definitions
bool Log::useElog = true;
bool Log::initialized = false;

void Log::init(bool useElogFlag) {
    useElog = useElogFlag;
    
    if (useElog) {
        // Initialize Elog with conservative settings
        Logger.configure(500, true);  // Large buffer, blocking mode
        delay(100);
        Logger.registerSerial(MAIN_LOG, ELOG_LEVEL_INFO, "LOG", Serial);
        delay(100);
    }
    
    initialized = true;
    
    if (useElog) {
        Logger.info(MAIN_LOG, "Log module initialized with Elog");
    } else {
        Serial.println("[LOG] Log module initialized with Serial fallback");
        Serial.flush();
    }
}

void Log::setUseElog(bool enable) {
    useElog = enable;
    if (initialized) {
        if (useElog) {
            Serial.println("[LOG] Switched to Elog mode");
        } else {
            Serial.println("[LOG] Switched to Serial fallback mode");
        }
        Serial.flush();
    }
}

void Log::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (useElog && initialized) {
        logWithElog(ELOG_LEVEL_INFO, format, args);
    } else {
        logWithSerial("INFO", format, args);
    }
    
    va_end(args);
}

void Log::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (useElog && initialized) {
        logWithElog(ELOG_LEVEL_ERROR, format, args);
    } else {
        logWithSerial("ERROR", format, args);
    }
    
    va_end(args);
}

void Log::warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (useElog && initialized) {
        logWithElog(ELOG_LEVEL_WARNING, format, args);
    } else {
        logWithSerial("WARN", format, args);
    }
    
    va_end(args);
}

void Log::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (useElog && initialized) {
        logWithElog(ELOG_LEVEL_DEBUG, format, args);
    } else {
        logWithSerial("DEBUG", format, args);
    }
    
    va_end(args);
}

void Log::notice(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (useElog && initialized) {
        logWithElog(ELOG_LEVEL_NOTICE, format, args);
    } else {
        logWithSerial("NOTICE", format, args);
    }
    
    va_end(args);
}

void Log::logWithElog(uint8_t elogLevel, const char* format, va_list args) {
    // Use Elog's variadic logging
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    switch(elogLevel) {
        case ELOG_LEVEL_INFO:
            Logger.info(MAIN_LOG, "%s", buffer);
            break;
        case ELOG_LEVEL_ERROR:
            Logger.error(MAIN_LOG, "%s", buffer);
            break;
        case ELOG_LEVEL_WARNING:
            Logger.warning(MAIN_LOG, "%s", buffer);
            break;
        case ELOG_LEVEL_DEBUG:
            Logger.debug(MAIN_LOG, "%s", buffer);
            break;
        case ELOG_LEVEL_NOTICE:
            Logger.notice(MAIN_LOG, "%s", buffer);
            break;
        default:
            Logger.info(MAIN_LOG, "%s", buffer);
            break;
    }
}

void Log::logWithSerial(const char* level, const char* format, va_list args) {
    // Get timestamp
    unsigned long timestamp = millis();
    
    // Format the message
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    // Print with timestamp and level
    Serial.printf("[%08lu] [%s] %s\n", timestamp, level, buffer);
    Serial.flush();  // Ensure immediate output
}
