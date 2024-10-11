#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h> 

#include "log.h"
#include "mqtt.h"
#include "message.h"
#include "common.h"
#include "msg_unsuback.h"

static const uint8_t unsuback_fixed_header[2] = {0xB0, 0x02}; /* UNSUBACK 报文的固定头固定为 0xB0 0x02 */
int unsuback_message_build(uint16_t packet_id, uint8_t *message, uint32_t *message_len)
{
    if (message == NULL || message_len == NULL || *message_len < UNSUBACK_MESSAGE_LENGTH) {
        return MQTT_INVALID_PARAM;
    }

    *message_len = 0;
    
    memcpy(message, unsuback_fixed_header, sizeof(unsuback_fixed_header));
    message += sizeof(unsuback_fixed_header);
    *message_len += sizeof(unsuback_fixed_header);
    
    packet_id = htons(packet_id);
    memcpy(message, &packet_id, sizeof(uint16_t));
    message += sizeof(uint16_t);
    *message_len += sizeof(uint16_t);

    return MQTT_SUCCESS;
}

int unsuback_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id)
{
    if (message == NULL || message_len < UNSUBACK_MESSAGE_LENGTH || packet_id == NULL) {
        return MQTT_INVALID_PARAM;
    }
    
    if (*message != unsuback_fixed_header[0]) {
        mqtt_log_error("not a unsuback message");
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    message++;

    if (*message != unsuback_fixed_header[1]) {
        mqtt_log_error("remain length error, now is %u, should be %u", *message, unsuback_fixed_header[1]);
        return MQTT_UNSUBSCRIBE_MESSAGE_ERROR;
    }
    message += 1;

    memcpy(packet_id, message, sizeof(uint16_t));
    *packet_id = ntohs(*packet_id);

    return MQTT_SUCCESS;
}
