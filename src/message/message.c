
#include <arpa/inet.h>

#include "common.h"
#include "message.h"
#include "log.h"
#include "mqtt.h"

int remain_length_encode(uint32_t length, uint8_t encode[4], uint32_t *encode_len)
{
	int index = 0;
	uint8_t byte;
	do {
		byte = length % 128;
		length = length / 128;
		if (length > 0) {
			encode[index] = byte | (1<<7);
		} else {
			encode[index] = byte;
		}
		index += 1;
	} while (length > 0);

	*encode_len = index;
    if (*encode_len > MQTT_REMAIN_LEN_ENCODE_MAX_LENGTH) {
        return MQTT_INVALID_PARAM;
    }
	return MQTT_SUCCESS;
}

int remain_length_decode(uint8_t *encode, uint32_t *encode_len, uint32_t *length)
{
	uint32_t base = 1;
	int index = 0;
	uint8_t byte_value;
	*encode_len = *encode_len <= 4 ? *encode_len : 4;
    *length = 0;
	while (index < *encode_len) {
		byte_value = encode[index] & (~(1<<7));
		*length += (byte_value * base);
		if ((encode[index] & (1<<7)) == 0) {
			break;
		}
		base = base<<7;
		index += 1;
	}
    
    *encode_len = index + 1;

    return MQTT_SUCCESS;
}

int build_string_field(uint8_t *str, uint16_t str_len, uint8_t *buffer, uint32_t *buffer_len)
{
    /* 允许str为空（例如在connect报文中，允许clientid是一个零字节的字符串 */
    if (str == NULL && str_len != 0) {
        mqtt_log_error("invalid param");
        return MQTT_INVALID_PARAM;
    }
    
    if (buffer == NULL || buffer_len == NULL) {
        mqtt_log_error("invalid param");
        return MQTT_INVALID_PARAM;
    }
    
    // if (*buffer_len < str_len + 2) {
    //     mqtt_log_error("buffer is shorter than str_len + 2");
    //     return MQTT_INVALID_PARAM;
    // }

    uint16_t network_len = htons(str_len);  // 将字符串长度由主机序转换成大端序
    memcpy(buffer, &network_len, sizeof(uint16_t));

    memcpy(buffer + 2, str, str_len);
    *buffer_len = str_len + 2;
    
    return MQTT_SUCCESS;
}

/**
 * @brief utf-8字符串解析函数。当不确定字符长度时，可以设置str为NULL，能够根据返回值strlen动态申请空间。
 *        但如果str不为NULL，就一定确保其大小能够容纳解析后数据
 * 
 * @param buffer: 待解析的字符串指针
 * @param buffer_len: buffer的长度
 * @param str: 解析后的字符串指针
 * @param str_len: 解析后的字符串的长度指针
 * 
 * @return 0: 解析成功
*/
int parse_string_field(uint8_t *buffer, uint32_t buffer_len, uint8_t *str, uint16_t *str_len)
{
    if (buffer == NULL || str_len == NULL) {
        mqtt_log_error("invalid param");
        return MQTT_INVALID_PARAM;
    }
    
    uint16_t network_len = ntohs(*(uint16_t *)buffer);
    if (str != NULL) {
        memcpy(str, buffer + 2, network_len);
    }
    
    *str_len = network_len;
    
    return MQTT_SUCCESS;
}

int locate_string_field(uint8_t *buffer, uint32_t buffer_len, uint8_t **str, uint16_t *str_len)
{
    if (buffer == NULL || str_len == NULL || str == NULL) {
        mqtt_log_error("invalid param");
        return MQTT_INVALID_PARAM;
    }

    *str = buffer + sizeof(uint16_t);
    *str_len = ntohs(*(uint16_t *)buffer);
    
    return MQTT_SUCCESS;
}


int message_type_filter(uint8_t *message, uint32_t message_len, mqtt_message_type_t *message_type)
{
    uint8_t type = (message[0] & MESSAGE_TYPE_MASK) >> 4;
    
    if (type < MQTT_MESSAGE_TYPE_MAX && type > MQTT_MESSAGE_TYPE_MIN) {
        *message_type = type;
        return MQTT_SUCCESS;
    } else {
        *message_type = MQTT_MESSAGE_TYPE_MAX;
        return MQTT_MESSAGE_TYPE_ERROR;
    }
}
