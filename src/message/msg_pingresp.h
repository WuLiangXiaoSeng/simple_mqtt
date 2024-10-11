#ifndef __MSG_PINGRESP_H__
#define __MSG_PINGRESP_H__

#define PINGREQ_MESSAGE_LENGTH 2

int pingresp_message_build(uint8_t *message, uint32_t *message_len);

int pingresp_message_parse(uint8_t *message, uint32_t message_len);

#endif