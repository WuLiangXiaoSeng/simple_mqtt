#include <stdlib.h>

#include "mqtt.h"
#include "message.h"
#include "common.h"
#include "msg_pingresp.h"


int pingresp_message_build(uint8_t *message, uint32_t *message_len)
{
    if (message == NULL || message_len == NULL || *message_len < PINGREQ_MESSAGE_LENGTH)
    {
        return MQTT_INVALID_PARAM;
    }
    
    message[0] = message_type_encode(MQTT_MESSAGE_TYPE_PINGREQ);
    message[1] = 0x00;
    
    *message_len = PINGREQ_MESSAGE_LENGTH;
    
    return MQTT_SUCCESS;
}

int pingresp_message_parse(uint8_t *message, uint32_t message_len)
{
    return MQTT_SUCCESS;
}