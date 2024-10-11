#ifndef __MSG_UNSUBSCRIBE_H__
#define __MSG_UNSUBSCRIBE_H__

#include "common.h"

int unsubscribe_message_build(uint16_t packet_id, topic_filter_t *filter_list, uint8_t *message, uint32_t *message_len);
int unsubscribe_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id, topic_filter_t **filter_list);

#endif