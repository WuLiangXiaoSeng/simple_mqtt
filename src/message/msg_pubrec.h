#ifndef __MQTT_MSG_PUBREC_H__
#define __MQTT_MSG_PUBREC_H__

#define PUBREC_MESSAGE_LENGTH 4

int pubrec_message_build(uint16_t packet_id, uint8_t *message, uint32_t *message_len);

int pubrec_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id);

#endif