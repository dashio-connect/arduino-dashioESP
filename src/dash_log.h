#ifndef SERIAL_WRAPPER_H
#define SERIAL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Log levels matching Arduino/ESP32 convention
typedef enum {
    DASH_LOG_LEVEL_NONE = 0,
    DASH_LOG_LEVEL_ERROR = 1,
    DASH_LOG_LEVEL_WARN = 2,
    DASH_LOG_LEVEL_INFO = 3,
    DASH_LOG_LEVEL_DEBUG = 4,
    DASH_LOG_LEVEL_VERBOSE = 5
} dash_log_level_t;

// Log level control
void dash_log_set_level(dash_log_level_t level);
dash_log_level_t dash_log_get_level(void);
void dash_log_output(dash_log_level_t level, const char* level_str, const char* tag, const char* format, ...);

// ESP32 specific functions
#if defined(ESP32) || defined(ESP_PLATFORM)
void dash_log_sync_with_esp32(void);
const char* dash_log_get_esp32_level_string(void);
#endif

#ifdef __cplusplus
}
#endif

// Your logger macro with level checking
#define DASH_LOG(level_enum, level_str, tag, msg, ...)  \
    do {                              \
        if (level_enum <= dash_log_get_level()) { \
            dash_log_output(level_enum, level_str, tag, msg, ##__VA_ARGS__); \
        } \
    } while (0)

// Convenience macros for different log levels
#define DASH_LOGE(tag, msg, ...)   DASH_LOG(DASH_LOG_LEVEL_ERROR, "ERROR", tag, msg, ##__VA_ARGS__)
#define DASH_LOGW(tag, msg, ...)    DASH_LOG(DASH_LOG_LEVEL_WARN, "WARN", tag, msg, ##__VA_ARGS__)
#define DASH_LOGI(tag, msg, ...)    DASH_LOG(DASH_LOG_LEVEL_INFO, "INFO", tag, msg, ##__VA_ARGS__)
#define DASH_LOGD(tag, msg, ...)   DASH_LOG(DASH_LOG_LEVEL_DEBUG, "DEBUG", tag, msg, ##__VA_ARGS__)
#define DASH_LOGV(tag, msg, ...) DASH_LOG(DASH_LOG_LEVEL_VERBOSE, "VERBOSE", tag, msg, ##__VA_ARGS__)

#endif