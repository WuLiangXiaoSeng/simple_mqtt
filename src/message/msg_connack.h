#ifndef __MQTT_MSG_CONNACK_H__
#define __MQTT_MSG_CONNACK_H__

#include "message.h"

typedef enum {
    MQTT_CONNACK_ACCEPTED = 0x00,
    MQTT_CONNACK_UNACCEPTABLE_PROTOCOL_VERSION= 0x01,
    MQTT_CONNACK_IDENTIFIER_REJECTED = 0x02,
    MQTT_CONNACK_SERVER_UNAVAILABLE = 0x03,
    MQTT_CONNACK_BAD_USERNAME_OR_PASSWORD = 0x04,
    MQTT_CONNACK_NOT_AUTHORIZED = 0x05,
    MQTT_CONNACK_MAX = 0x06,
} connack_code_t;


#define CONNACK_SP_MASK 0b00000001

typedef struct connack_variable_header_ {
    uint8_t reserved_and_sp;  /* reserved(7 bit) and session present flag(1 bit) */
    uint8_t connack_code;
}__attribute__((packed)) connack_variable_header_t;

int connack_message_build(uint8_t sp, connack_code_t connack_code, 
                        uint8_t *message, uint32_t *message_len);


int connack_message_parse(uint8_t *message, uint32_t message_len,
                          connack_code_t *connack_code, uint8_t *sp);

#endif