#ifndef __MQTT_MSG_SUBACK_H__
#define __MQTT_MSG_SUBACK_H__

typedef struct suback_data_ {
    uint8_t *qos;
    uint16_t size;
    uint16_t capacity;
} suback_data_t;

int suback_data_init(suback_data_t *data, uint16_t capacity);
int suback_data_append(suback_data_t *data, uint8_t qos);
int suback_data_release(suback_data_t *data);

int suback_message_build(uint16_t packet_id, suback_data_t *data, uint8_t *message, uint32_t *message_len);
int suback_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id, suback_data_t *data);
#endif