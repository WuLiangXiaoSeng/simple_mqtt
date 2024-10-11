#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "log.h"
#include "mqtt.h"
#include "common.h"
#include "message.h"
#include "msg_unsubscribe.h"

/**
 * MQTT UNSUBSCRIBE Message
 * 
 * Byte1: Message type + Reserved bits (4 + 4)
 * Byte2~N: Remain length (N <= 5)
 * ByteN+1~N+2: Packet identifier (2 bytes)
 * ByteN+3~M: Topic filter (UTF-8 encoded string)
*/

int unsubscribe_message_build(uint16_t packet_id, topic_filter_t *filter_list, uint8_t *message, uint32_t *message_len)
{
    if (filter_list == NULL || message == NULL || message_len == NULL) {
        return MQTT_INVALID_PARAM;
    }

    int ret = 0;
    uint32_t remain_len = 0;
    uint8_t buffer[4] = {0};
    uint32_t field_len = 0;
    topic_filter_t *cur = filter_list;

    /* 计算剩余长度 */
    remain_len += sizeof(uint16_t);
    cur = filter_list;
    while (cur != NULL) {
        remain_len += cur->topic_filter_len + 2; /* 2 bytes for topic filter length */
        cur = cur->next;
    }

    ret = remain_length_encode(remain_len, buffer, &field_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain_length_encode failed, ret=%d", ret);
        return ret;
    }
    
    if (*message_len < (field_len + remain_len + 1)) {
        mqtt_log_error("message buffer is too small, need %d, but only %d", field_len + remain_len + 1, *message_len);
        return MQTT_INVALID_PARAM;
    }
    
    *message_len = 0;

    *message = message_type_encode(MQTT_MESSAGE_TYPE_UNSUBSCRIBE);
    message += 1;
    *message_len += 1;

    memcpy(message, buffer, field_len);
    message += field_len;
    *message_len += field_len;

    packet_id = htons(packet_id);
    memcpy(message, &packet_id, sizeof(uint16_t));
    message += sizeof(uint16_t);
    *message_len += sizeof(uint16_t);
    
    cur = filter_list;
    while (cur != NULL) {
        ret = build_string_field(cur->topic_filter, cur->topic_filter_len, message, &field_len);
        if (ret != MQTT_SUCCESS) {
            mqtt_log_error("build_string_field failed, ret=%d", ret);
            return ret;
        }
        message += field_len;
        *message_len += field_len;

        cur = cur->next;
    }

    return MQTT_SUCCESS;
}

int unsubscribe_message_parse(uint8_t *message, uint32_t message_len, uint16_t *packet_id, topic_filter_t **filter_list)
{
    if (message == NULL || message_len == 0 || packet_id == NULL || filter_list == NULL) {
        return MQTT_INVALID_PARAM;
    }
    
    int ret;
    uint32_t field_len, remain_len;
    topic_filter_t *cur, *pre;

    if (*message != message_type_encode(MQTT_MESSAGE_TYPE_UNSUBSCRIBE)) {
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
        return MQTT_SUBSCRIBE_MESSAGE_ERROR;
    }

    memcpy(packet_id, message, sizeof(uint16_t));
    *packet_id = ntohs(*packet_id);
    message += sizeof(uint16_t);
    message_len -= sizeof(uint16_t);
    
    *filter_list = (topic_filter_t *)malloc(sizeof(topic_filter_t));
    if (*filter_list == NULL) {
        mqtt_log_error("malloc failed");
        return MQTT_MEMORY_NOBUFFER;
    }
    
    pre = *filter_list;
    while (message_len > 0) {
        cur = (topic_filter_t *)malloc(sizeof(topic_filter_t));
        if (cur == NULL) {
            mqtt_log_error("malloc failed");
            ret = MQTT_MEMORY_NOBUFFER;
            goto _exit;
        }
        memset(cur, 0, sizeof(topic_filter_t));
        
        ret = locate_string_field(message, message_len, &cur->topic_filter, &cur->topic_filter_len);
        if (ret != MQTT_SUCCESS) {
            mqtt_log_error("locate_string_field failed");
            free(cur);
            goto _exit;
        }
        message += cur->topic_filter_len + 2;
        message_len -= cur->topic_filter_len + 2;
        
        cur->next = NULL;
        pre->next = cur;
        pre = cur;
    }
    
    /* 释放第一个元素，因为第一个元素中是没有数据的 */
    pre = *filter_list;
    *filter_list = (*filter_list)->next;
    free(pre);

    return MQTT_SUCCESS;
_exit:
    cur = *filter_list;
    while (cur != NULL) {
        pre = cur;
        cur = cur->next;
        free(pre);
    }

    return ret;
}