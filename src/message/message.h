#ifndef _MQTT_MESSAGE_H_
#define _MQTT_MESSAGE_H_

#include <stdint.h>

#include "common.h"

#define MESSAGE_TYPE_MASK 0xF0

#define message_type_encode(message_type) (((message_type << 4) & MESSAGE_TYPE_MASK))

typedef struct message_fixed_header_ {
    uint8_t packet_type_with_reserved;    /* 4bit 报文类型 + 4bit 保留位，但由于保留位暂未定义 */
    uint8_t remain_length[0]; /* 剩余长度（不包括当前字段）*/
} message_fixed_header_t;

int remain_length_encode(uint32_t length, uint8_t encode[4], uint32_t *encode_len);

int remain_length_decode(uint8_t *encode, uint32_t *encode_len, uint32_t *length);

int build_string_field(uint8_t *str, uint16_t str_len, uint8_t *buffer, uint32_t *buffer_len);

int parse_string_field(uint8_t *buffer, uint32_t buffer_len, uint8_t *str, uint16_t *str_len);

int locate_string_field(uint8_t *buffer, uint32_t buffer_len, uint8_t **str, uint16_t *str_len);

int message_type_filter(uint8_t *message, uint32_t message_len, mqtt_message_type_t *message_type);

#endif
