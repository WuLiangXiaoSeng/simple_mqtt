#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "mqtt.h"
#include "message.h"
#include "msg_puback.h"

static const uint8_t puback_fixed_header[2] = {0x40, 0x02}; /* PUBACK 报文的固定头固定为 0x40 0x02 */

int puback_message_build(uint16_t packet_id, uint8_t *message, uint32_t *message_len)
{
    if (message == NULL || message_len == NULL || *message_len < PUBACK_MESSAGE_LENGTH) {
        return MQTT_INVALID_PARAM;
    }

    *message_len = 0;
    
    memcpy(message, puback_fixed_header, sizeof(puback_fixed_header));
    message += sizeof(puback_fixed_header);
    *message_len += sizeof(puback_fixed_header);
    
    memcpy(message, &packet_id, sizeof(uint16_t));
    message += sizeof(uint16_t);
    *message_len += sizeof(uint16_t);

    return MQTT_SUCCESS;
}

int puback_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id)
{
    if (message == NULL || message_len != PUBACK_MESSAGE_LENGTH || packet_id == NULL) {
        return MQTT_INVALID_PARAM;
    }

    if (*message != puback_fixed_header[0]) {
        mqtt_log_error("this is not puback message");
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    message += 1;
    
    if (*message != puback_fixed_header[1]) {
        mqtt_log_error("remain length error, now is %u, should be %u", *message, puback_fixed_header[1]);
        return MQTT_PUBACK_MESSAGE_ERROR;
    }
    message += 1;

    memcpy(packet_id, message + 1, sizeof(uint16_t));

    return MQTT_SUCCESS;
}