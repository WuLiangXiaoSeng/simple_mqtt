#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "mqtt.h"
#include "message.h"
#include "msg_pubrel.h"

static const uint8_t pubrel_fixed_header[2] = {0x60, 0x02}; /* PUBREL 报文的固定头固定为 0x60 0x02 */

int pubrel_message_build(uint16_t packet_id, uint8_t *message, uint32_t *message_len)
{
    if (message == NULL || message_len == NULL || *message_len < PUBREL_MESSAGE_LENGTH) {
        return MQTT_INVALID_PARAM;
    }

    *message_len = 0;
    
    memcpy(message, pubrel_fixed_header, sizeof(pubrel_fixed_header));
    message += sizeof(pubrel_fixed_header);
    *message_len += sizeof(pubrel_fixed_header);
    
    memcpy(message, &packet_id, sizeof(uint16_t));
    message += sizeof(uint16_t);
    *message_len += sizeof(uint16_t);

    return MQTT_SUCCESS;
}

int pubrel_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id)
{
    if (message == NULL || message_len != PUBREL_MESSAGE_LENGTH || packet_id == NULL) {
        return MQTT_INVALID_PARAM;
    }

    if (*message != pubrel_fixed_header[0]) {
        mqtt_log_error("this is not pubrec message");
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    message += 1;
    
    if (*message != pubrel_fixed_header[1]) {
        mqtt_log_error("remain length error, now is %u, should be %u", *message, pubrel_fixed_header[1]);
        return MQTT_PUBREL_MESSAGE_ERROR;
    }
    message += 1;

    memcpy(packet_id, message, sizeof(uint16_t));

    return MQTT_SUCCESS;
}