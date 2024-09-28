/**
 * @file msg_pubrec.c
 * @brief PUBREC 报文解析和构建函数实现文件
 * @author wuliangxiaoseng
 * @date 2024-9-26
*/

#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "mqtt.h"
#include "message.h"
#include "msg_pubrec.h"

static const uint8_t pubrec_fixed_header[2] = {0x50, 0x02}; /* PUBREC 报文的固定头固定为 0x50 0x02 */

int pubrec_message_build(uint16_t packet_id, uint8_t *message, uint32_t *message_len)
{
    if (message == NULL || message_len == NULL || *message_len < PUBREC_MESSAGE_LENGTH) {
        return MQTT_INVALID_PARAM;
    }

    *message_len = 0;
    
    memcpy(message, pubrec_fixed_header, sizeof(pubrec_fixed_header));
    message += sizeof(pubrec_fixed_header);
    *message_len += sizeof(pubrec_fixed_header);
    
    memcpy(message, &packet_id, sizeof(uint16_t));
    message += sizeof(uint16_t);
    *message_len += sizeof(uint16_t);

    return MQTT_SUCCESS;
}

int pubrec_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id)
{
    if (message == NULL || message_len != PUBREC_MESSAGE_LENGTH || packet_id == NULL) {
        return MQTT_INVALID_PARAM;
    }

    if (*message != pubrec_fixed_header[0]) {
        mqtt_log_error("this is not pubrec message");
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    message += 1;
    
    if (*message != pubrec_fixed_header[1]) {
        mqtt_log_error("remain length error, now is %u, should be %u", *message, pubrec_fixed_header[1]);
        return MQTT_PUBREC_MESSAGE_ERROR;
    }
    message += 1;

    memcpy(packet_id, message, sizeof(uint16_t));

    return MQTT_SUCCESS;
}