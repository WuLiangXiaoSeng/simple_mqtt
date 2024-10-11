#include <stdlib.h>

#include "log.h"
#include "mqtt.h"
#include "message.h"
#include "common.h"
#include "msg_disconnect.h"

int disconnect_message_build(uint8_t *message, uint32_t *message_len)
{
    if(message == NULL || message_len == NULL || *message_len < DISCONNECT_MESSAGE_LENGTH) {
        return MQTT_INVALID_PARAM;
    }

    message[0] = message_type_encode(MQTT_MESSAGE_TYPE_DISCONNECT);
    message[1] = 0x00;

    *message_len = DISCONNECT_MESSAGE_LENGTH;

    return MQTT_SUCCESS;
}

int disconnect_message_parse(uint8_t *message, uint32_t message_len)
{
    if(message == NULL || message_len < DISCONNECT_MESSAGE_LENGTH) {
        return MQTT_INVALID_PARAM;
    }
    
    if ((message[0] & MESSAGE_TYPE_MASK) != message_type_encode(MQTT_MESSAGE_TYPE_DISCONNECT)) {
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    
    /* check reserved bits */
    if ((message[1] & ~MESSAGE_TYPE_MASK) != 0x00) {
        return MQTT_FIXED_HEADER_RESERVED_ERROR;
    }
    
    return MQTT_SUCCESS;
}