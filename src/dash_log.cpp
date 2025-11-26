#include <Arduino.h>
#include "dash_log.h"
#include <stdarg.h>

// Include ESP32 logging
#if defined(ESP32) || defined(ESP_PLATFORM)
    #include "esp_log.h"
#endif

// Global log level (default to INFO)
static dash_log_level_t current_log_level = DASH_LOG_LEVEL_INFO;

extern "C" {
    // Set the current log level
    void dash_log_set_level(dash_log_level_t level) {
        current_log_level = level;
    }
    
    // Get the current log level
    dash_log_level_t dash_log_get_level(void) {
        return current_log_level;
    }
    
    #if defined(ESP32) || defined(ESP_PLATFORM)
    // Sync with ESP32 Arduino core log level
    void dash_log_sync_with_esp32(void) {
        // Read the ESP32 core log level
        // ARDUHAL_LOG_LEVEL is set by Arduino core
        #ifdef ARDUHAL_LOG_LEVEL
            switch(ARDUHAL_LOG_LEVEL) {
                case ARDUHAL_LOG_LEVEL_NONE:
                    current_log_level = DASH_LOG_LEVEL_NONE;
                    break;
                case ARDUHAL_LOG_LEVEL_ERROR:
                    current_log_level = DASH_LOG_LEVEL_ERROR;
                    break;
                case ARDUHAL_LOG_LEVEL_WARN:
                    current_log_level = DASH_LOG_LEVEL_WARN;
                    break;
                case ARDUHAL_LOG_LEVEL_INFO:
                    current_log_level = DASH_LOG_LEVEL_INFO;
                    break;
                case ARDUHAL_LOG_LEVEL_DEBUG:
                    current_log_level = DASH_LOG_LEVEL_DEBUG;
                    break;
                case ARDUHAL_LOG_LEVEL_VERBOSE:
                    current_log_level = DASH_LOG_LEVEL_VERBOSE;
                    break;
                default:
                    current_log_level = DASH_LOG_LEVEL_INFO;
            }
        #else
            // Default if not defined
            current_log_level = DASH_LOG_LEVEL_INFO;
        #endif
    }
    
    // Get ESP32 log level as string
    const char* dash_log_get_esp32_level_string(void) {
        #ifdef ARDUHAL_LOG_LEVEL
            switch(ARDUHAL_LOG_LEVEL) {
                case ARDUHAL_LOG_LEVEL_NONE: return "NONE";
                case ARDUHAL_LOG_LEVEL_ERROR: return "ERROR";
                case ARDUHAL_LOG_LEVEL_WARN: return "WARN";
                case ARDUHAL_LOG_LEVEL_INFO: return "INFO";
                case ARDUHAL_LOG_LEVEL_DEBUG: return "DEBUG";
                case ARDUHAL_LOG_LEVEL_VERBOSE: return "VERBOSE";
                default: return "UNKNOWN";
            }
        #else
            return "NOT_DEFINED";
        #endif
    }
    #endif
    
    // Output log message with level checking
    void dash_log_output(dash_log_level_t level, const char* level_str, const char* tag, const char* format, ...) {
            if (level > current_log_level) return;

        char buffer[256];
        int len = 0;

        // Compose the prefix
        len += snprintf(buffer + len, sizeof(buffer) - len, "[%s]", level_str);
        if (tag && tag[0] != '\0') {
            len += snprintf(buffer + len, sizeof(buffer) - len, " [%s]", tag);
        }

        len += snprintf(buffer + len, sizeof(buffer) - len, " ");

        // Append formatted message
        va_list args;
        va_start(args, format);
        len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
        va_end(args);

        // Append newline
        snprintf(buffer + len, sizeof(buffer) - len, "\r\n");

        Serial.print(buffer);
    }
}