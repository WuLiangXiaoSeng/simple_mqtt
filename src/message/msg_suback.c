#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h> 

#include "log.h"
#include "mqtt.h"
#include "message.h"
#include "common.h"
#include "msg_suback.h"

#define SUBACK_TOPIC_NUM_DEFAULT 10
#define SUBACK_TOPIC_NUM_THRESHOLD 40

int suback_data_init(suback_data_t *data, uint16_t capacity)
{
    if (data == NULL) {
        return MQTT_INVALID_PARAM;
    }
    if (capacity == 0) {
        data->capacity = SUBACK_TOPIC_NUM_DEFAULT;
    } else {
        data->capacity = capacity;
    }
    
    data->qos = (uint8_t *)malloc(data->capacity);
    if (data->qos == NULL) {
        return MQTT_MEMORY_NOBUFFER;
    }
    data->size = 0;
    
    return MQTT_SUCCESS;
}

int suback_data_append(suback_data_t *data, uint8_t qos)
{
    if (data == NULL) {
        return MQTT_INVALID_PARAM;
    }
    
    /* 当size等于capcity时，需要对qos进行拓展，采用先倍增再线性的拓展策略(正常情况下数量不会太多) */
    if (data->size >= data->capacity) {
        uint16_t new_capacity = data->capacity;
        if (data->capacity < SUBACK_TOPIC_NUM_THRESHOLD) {
            new_capacity *= 2;
        } else {
            new_capacity += SUBACK_TOPIC_NUM_DEFAULT;
        }

        uint8_t *new_qos = (uint8_t *)realloc(data->qos, new_capacity);
        if (new_qos == NULL) {
            return MQTT_MEMORY_NOBUFFER;
        }
        data->qos = new_qos;
        data->capacity = new_capacity;
    }
    data->qos[data->size++] = qos;

    return MQTT_SUCCESS;
}

int suback_data_release(suback_data_t *data) 
{
    if (data == NULL) {
        return MQTT_INVALID_PARAM;
    }
    
    free(data->qos);
    data->qos = NULL;
    
    return MQTT_SUCCESS;
}

/**
 * MQTT SUBACK 报文
 * 
 * byte1: Message Type + Reversed Bits (4 + 4)
 * byte2~N: Remain Length (N <= 5)
 * byteN+1~N+2: Packet ID
 * byteN+3~N+3+M: Payload, Topic Filter QoS
*/


/**
 * @brief 构建 SUBACK 消息
 * 
 * @param packet_id SUBACK 消息包ID
 * @param data SUBACK 消息数据 (订阅主题对应的QoS)
 * @param message 存储构建的消息
 * @param message_len 存储构建的消息长度
 * 
 * @return 0 success
*/
int suback_message_build(uint16_t packet_id, suback_data_t *data,
                            uint8_t *message, uint32_t *message_len)
{
    if (data == NULL || message == NULL || message_len == NULL) {
        return MQTT_INVALID_PARAM;
    }

    uint32_t remain_len = 0;
    int ret = 0;
    uint8_t remain_len_encode[4];
    uint32_t encode_len;
    
    remain_len += sizeof(uint16_t);
    remain_len += data->size;
    
    ret = remain_length_encode(remain_len, remain_len_encode, &encode_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain_length_encode failed");
        return ret;
    }
    
    if (*message_len < (remain_len + encode_len + 1)) {
        mqtt_log_error("message buffer is too small, need %u bytes, but only %u bytes", (remain_len + encode_len + 1), *message_len);
        return MQTT_INVALID_PARAM;
    }
    
    *message_len = 0;
    *message = message_type_encode(MQTT_MESSAGE_TYPE_SUBACK);
    message += 1;
    *message_len += 1;

    memcpy(message, remain_len_encode, encode_len);
    message += encode_len;
    *message_len += encode_len;

    /* Packet ID, 在报文传输中需要是大端序（即网络字节序）*/
    packet_id = htons(packet_id);  
    memcpy(message, &packet_id, sizeof(uint16_t));
    message += sizeof(uint16_t);
    *message_len += sizeof(uint16_t);
    
    for (uint16_t i = 0; i < data->size; i++) {
        if (data->qos[i] >= MQTT_QOS_MAX && data->qos[i] != MQTT_QOS_REJECT) {
            return MQTT_SUBACK_QOS_ERROR;
        }
        memcpy(message, &data->qos[i], sizeof(uint8_t));
        message += sizeof(uint8_t);
        *message_len += sizeof(uint8_t);
    }
    
    return MQTT_SUCCESS;
}

/**
 * @brief 解析 SUBACK 消息
 * 
 * @param message SUBACK 消息
 * @param message_len SUBACK 消息长度
 * @param packet_id SUBACK 消息包ID
 * @param data SUBACK 消息数据(订阅主题对应的QoS)
 * 
 * @return 0 success
*/
int suback_message_parse(uint8_t *message, uint32_t message_len,
                            uint16_t *packet_id, suback_data_t *data)
{
    if (message == NULL || message_len == 0 || packet_id == NULL || data == NULL) {
        return MQTT_INVALID_PARAM;
    }
    
    int ret = 0;
    uint32_t remain_len, field_len;
    
    if (*message != message_type_encode(MQTT_MESSAGE_TYPE_SUBACK)) {
        mqtt_log_error("message type error");
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    message += 1;
    message_len -= 1;

    field_len = message_len;
    ret = remain_length_decode(message, &field_len, &remain_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain_length_decode failed");
        return ret;
    }
    message += field_len;
    message_len -= field_len;

    if (remain_len != message_len) {
        mqtt_log_error("remain_len error");
        return MQTT_SUBACK_MESSAGE_ERROR;
    }

    memcpy(packet_id, message, sizeof(uint16_t));
    *packet_id = ntohs(*packet_id);
    message += sizeof(uint16_t);
    message_len -= sizeof(uint16_t);

    /* 剩下报文中全部为订阅主题的QoS，且一个QoS只有一个字节 */
    ret = suback_data_init(data, message_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("suback_data_init failed");
        return ret;
    }
    for (uint32_t i = 0; i < message_len; i++) {
        if (message[i] >= MQTT_QOS_MAX && message[i] != MQTT_QOS_REJECT) {
            return MQTT_SUBACK_QOS_ERROR;
        }
        ret = suback_data_append(data, message[i]);
        if (ret != MQTT_SUCCESS) {
            mqtt_log_error("suback_data_append failed");
            suback_data_release(data);
            return ret;
        }
    }
    
    return MQTT_SUCCESS;
}