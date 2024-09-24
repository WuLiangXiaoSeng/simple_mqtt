#ifndef _LOG_H_
#define _LOG_H_

#include <stdint.h>
#include <string.h>

#define MQTT_LOG_LEVEL_DEBUG 0
#define MQTT_LOG_LEVEL_INFO 1
#define MQTT_LOG_LEVEL_WARN 2
#define MQTT_LOG_LEVEL_ERROR 3
#define MQTT_LOG_LEVEL_FATAL 4
#define MQTT_LOG_LEVEL_NONE 5

#ifndef MQTT_LOG_PREFIX
#define MQTT_LOG_PREFIX "[MQTT %s: %d]"
#endif

#if defined(_WIN32) || defined(_WIN64)
#define LOG_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define LOG_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif



extern uint32_t mqtt_log_level;

#define mqtt_log_printf(level, format, args...) \
    do { \
        if (level >= mqtt_log_level) { \
            printf(MQTT_LOG_PREFIX format "\n", LOG_FILENAME, __LINE__, ##args); \
        } \
    } while(0);


#define mqtt_log_debug(format, args...) \
    mqtt_log_printf(MQTT_LOG_LEVEL_DEBUG, format, ##args)

#define mqtt_log_info(format, args...) \
    mqtt_log_printf(MQTT_LOG_LEVEL_INFO, format, ##args)
    
#define mqtt_log_warn(format, args...) \
    mqtt_log_printf(MQTT_LOG_LEVEL_WARN, format, ##args)

#define mqtt_log_error(format, args...) \
    mqtt_log_printf(MQTT_LOG_LEVEL_ERROR, format, ##args)
    
#define mqtt_log_fatal(format, args...) \
    mqtt_log_printf(MQTT_LOG_LEVEL_FATAL, format, ##args)



#define mqtt_log_hexdump(data, len) \
    do { \
        int i; \
        for (i = 0; i < len; i++) { \
            printf("%02x ", data[i]); \
            if ((i + 1) % 16 == 0) printf("\n"); \
        } \
        printf("\n"); \
    } while(0);
    
#endif