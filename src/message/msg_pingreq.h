#ifndef __MSG_PINGREQ_H__
#define __MSG_PINGREQ_H__

#define PINGREQ_MESSAGE_LENGTH 2

int pingreq_message_build(uint8_t *message, uint32_t *message_len);

int pingreq_message_parse(uint8_t *message, uint32_t message_len);


#endif