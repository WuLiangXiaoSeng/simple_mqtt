#ifndef __MQTT_SUBSCRIBE_H__
#define __MQTT_SUBSCRIBE_H__

#include "common.h"


int topic_filter_release(topic_filter_t *head);

int subscribe_message_build(uint16_t packet_id, topic_filter_t *filter_list, uint8_t *message, uint32_t *message_len);

int subscribe_message_parse(uint8_t *message, uint32_t message_len,
                            uint16_t *packet_id, topic_filter_t **filter_list);

int subscribe_message_len_calc(topic_filter_t *filter_list, uint32_t *message_len);

#endif