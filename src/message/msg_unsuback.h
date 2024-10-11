#ifndef __MSG_UNSUBACK_H__
#define __MSG_UNSUBACK_H__

#define UNSUBACK_MESSAGE_LENGTH 4

int unsuback_message_build(uint16_t packet_id, uint8_t *message, uint32_t *message_len);
int unsuback_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id);

#endif