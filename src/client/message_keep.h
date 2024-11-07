#ifndef __MQTT_PACKET_ID_H__
#define __MQTT_PACKET_ID_H__

#include <stdint.h>
#include <sys/time.h>

typedef struct message_send_{
    uint16_t packet_id;
    uint8_t *message;
    uint32_t message_len;
    uint8_t qos;
    time_t timestamp;
    struct message_send_ *next;
} message_send_t;

int message_send_add(message_send_t **list, uint16_t packet_id, uint8_t *message, uint32_t message_len, uint8_t qos);

int message_send_find(message_send_t *list, uint16_t packet_id, message_send_t *node);

int message_send_remove(message_send_t **list, uint16_t packet_id);

#endif