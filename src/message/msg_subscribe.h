#ifndef __MQTT_SUBSCRIBE_H__
#define __MQTT_SUBSCRIBE_H__

#include "common.h"

typedef struct topic_filter_ {
    uint8_t *topic_filter;
    uint16_t topic_filter_len;
    uint8_t qos;
    struct topic_filter_ *next;
} topic_filter_t;

int topic_filter_release(topic_filter_t *head);

int subscribe_message_build(uint16_t packet_id, topic_filter_t *filter_list, uint8_t *message, uint32_t *message_len);

int subscribe_message_parse(uint8_t *message, uint32_t message_len,
                            uint16_t *packet_id, topic_filter_t **filter_list);

#endif