#ifndef _MQTT_COMMON_H_
#define _MQTT_COMMON_H_

typedef enum {
    MQTT_MESSAGE_TYPE_MIN = 0,
    MQTT_MESSAGE_TYPE_CONNECT = 1,
    MQTT_MESSAGE_TYPE_CONNACK = 2,
    MQTT_MESSAGE_TYPE_PUBLISH = 3,
    MQTT_MESSAGE_TYPE_PUBACK = 4,
    MQTT_MESSAGE_TYPE_PUBREC = 5,
    MQTT_MESSAGE_TYPE_PUBREL = 6,
    MQTT_MESSAGE_TYPE_PUBCOMP = 7,
    MQTT_MESSAGE_TYPE_SUBSCRIBE = 8,
    MQTT_MESSAGE_TYPE_SUBACK = 9,
    MQTT_MESSAGE_TYPE_UNSUBSCRIBE = 10,
    MQTT_MESSAGE_TYPE_UNSUBACK = 11,
    MQTT_MESSAGE_TYPE_PINGREQ = 12,
    MQTT_MESSAGE_TYPE_PINGRESP = 13,
    MQTT_MESSAGE_TYPE_DISCONNECT = 14,
    MQTT_MESSAGE_TYPE_MAX = 15,
} mqtt_message_type_t;

typedef enum {
    MQTT_QOS_0 = 0b00,
    MQTT_QOS_1 = 0b01,
    MQTT_QOS_2 = 0b10,
    MQTT_QOS_MAX = 0b11,
    MQTT_QOS_REJECT = 0x80,
} mqtt_qos_t;

typedef struct topic_filter_ {
    uint8_t *topic_filter;
    uint16_t topic_filter_len;
    uint8_t qos;
    struct topic_filter_ *next;
} topic_filter_t;

#define MQTT_REMAIN_LEN_ENCODE_MAX_LENGTH 4

#endif