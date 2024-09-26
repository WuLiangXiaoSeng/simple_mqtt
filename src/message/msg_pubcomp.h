#ifndef __MQTT_MSG_PUBREL_H__
#define __MQTT_MSG_PUBREL_H__

#define PUBCOMP_MESSAGE_LENGTH 4

int pubcomp_message_build(uint16_t packet_id, uint8_t *message, uint32_t *message_len);

int pubcomp_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id);

#endif