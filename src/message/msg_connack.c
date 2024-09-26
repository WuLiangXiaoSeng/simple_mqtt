#include <stdint.h>

#include "log.h"
#include "mqtt.h"
#include "msg_connack.h"
#include "common.h"

int connack_message_build(uint8_t sp, connack_code_t connack_code, 
                        uint8_t *message, uint32_t *message_len)
{
    connack_variable_header_t variable_header = {0};
    uint32_t remain_length, field_len;
    uint8_t message_type_with_servered;
    int ret;

    variable_header.connack_code = connack_code;
    if (sp) {
        variable_header.reserved_and_sp |= (0xFF & CONNACK_SP_MASK);
    }
    
    /* 构建 CONNACK 消息 */
    *message_len = 0;
    message_type_with_servered = message_type_encode(MQTT_MESSAGE_TYPE_CONNACK);
    memcpy(message, &message_type_with_servered, 1);
    message += 1;
    *message_len += 1;

    remain_length = sizeof(variable_header);
    ret = remain_length_encode(remain_length, message, &field_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain_length_encode failed, remain length %u", remain_length);
        return ret;
    }
    message += field_len;
    *message_len += field_len;
    
    memcpy(message, &variable_header, sizeof(variable_header));
    message += sizeof(variable_header);
    *message_len += sizeof(variable_header);

    return MQTT_SUCCESS;
}

int connack_message_parse(uint8_t *message, uint32_t message_len,
                          connack_code_t *connack_code, uint8_t *sp)
{
    if (!message || !connack_code || !sp) {
        mqtt_log_error("invalid param");
        return MQTT_INVALID_PARAM;
    }
    mqtt_log_debug("message_len %u", message_len);
    
    if (*message != message_type_encode(MQTT_MESSAGE_TYPE_CONNACK)) {
        mqtt_log_error("invalid message type %u, need %u", *message, MQTT_MESSAGE_TYPE_CONNACK);
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    message += 1;
    message_len -= 1;
    
    uint32_t remain_length;
    uint32_t field_len;
    int ret;

    ret = remain_length_decode(message, &field_len, &remain_length);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain_length_decode failed");
        return ret;
    }
    message += field_len;
    message_len -= field_len;

    if ((remain_length != sizeof(connack_variable_header_t)) || (message_len != remain_length)) {
        mqtt_log_error("invalid remain length %u, message len %u", remain_length, message_len);
        return MQTT_CONNACK_MESSAGE_ERROR;
    }

    connack_variable_header_t *variable_header = (connack_variable_header_t *)message;
    *connack_code = variable_header->connack_code;
    if ((variable_header->reserved_and_sp & (~CONNACK_SP_MASK)) != 0) {
        mqtt_log_error("invalid reserved_and_sp %#X", variable_header->reserved_and_sp);
        return MQTT_CONNACK_MESSAGE_ERROR;
    }
    *sp = (variable_header->reserved_and_sp & CONNACK_SP_MASK);
    
    return MQTT_SUCCESS;
}