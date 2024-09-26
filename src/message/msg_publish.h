#ifndef _MQTT_MSG_PUBLISH_H_
#define _MQTT_MSG_PUBLISH_H_


#define PUBLISH_FLAG_INDEX_RETAIN 0
#define PUBLISH_FLAG_INDEX_QOS_LOW 1
#define PUBLISH_FLAG_INDEX_QOS_HIGH 2
#define PUBLISH_FLAG_INDEX_DUP 3


int publish_message_build(uint8_t dup, mqtt_qos_t qos, uint8_t retain, 
                         uint8_t *topic, uint16_t topic_len,
                         uint16_t *packet_id,
                         uint8_t *payload, uint32_t payload_len,
                         uint8_t *message, uint32_t *message_len);
                         
int publish_message_parse(uint8_t *message, uint32_t message_len,
                          uint8_t *dup, mqtt_qos_t *qos, uint8_t *retain,
                          uint8_t **topic, uint16_t *topic_len, 
                          uint16_t *packet_id,
                          uint8_t **payload, uint32_t *payload_len);

#endif