#ifndef __MSG_DISCONNECT_H__
#define __MSG_DISCONNECT_H__

#define DISCONNECT_MESSAGE_LENGTH 2

int disconnect_message_build(uint8_t *message, uint32_t *message_len);
int disconnect_message_parse(uint8_t *message, uint32_t message_len);

#endif