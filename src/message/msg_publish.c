#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "message.h"
#include "common.h"
#include "msg_publish.h"
#include "mqtt.h"
#include "log.h"


int publish_message_build(uint8_t dup, uint8_t qos, uint8_t retain, 
                         uint8_t *topic, uint16_t topic_len,
                         uint16_t *packet_id,
                         uint8_t *payload, uint32_t payload_len,
                         uint8_t *message, uint32_t *message_len)
{
    if (!message || !message_len || !topic || !topic_len || (qos >= MQTT_QOS_MAX)) {
        return MQTT_INVALID_PARAM;
    }
    /* qos > 0 need packet_id */
    if (qos > MQTT_QOS_0 && (packet_id == NULL)) {
        return MQTT_INVALID_PARAM;
    }
    /* 0字节载荷长度是允许的 */
    if (payload_len != 0 && payload == NULL) {
        return MQTT_INVALID_PARAM;
    }
    
    uint8_t type_with_flags = 0;
    uint16_t remain_len = 0;
    int ret;
    uint8_t buffer[4];
    uint32_t feild_len;
    
    type_with_flags = message_type_encode(MQTT_MESSAGE_TYPE_PUBLISH);
    if (dup) {
        type_with_flags |= (1 << PUBLISH_FLAG_INDEX_DUP);
    } 
    if (retain) {
        type_with_flags |= (1 << PUBLISH_FLAG_INDEX_RETAIN);
    }
    switch (qos) {
        case MQTT_QOS_0:
            break;
        case MQTT_QOS_1:
            type_with_flags |= (1 << PUBLISH_FLAG_INDEX_QOS_LOW);
            break;
        case MQTT_QOS_2:
            type_with_flags |= (1 << PUBLISH_FLAG_INDEX_QOS_HIGH);
            break;
        default:
            return MQTT_INVALID_PARAM;
    }

    /* calc reamin length */
    remain_len = topic_len + 2;
    if (qos > MQTT_QOS_0) {
        remain_len += sizeof(uint16_t);
    }
    remain_len += payload_len;
    
    ret = remain_length_encode(remain_len, buffer, &feild_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain_length_encode %u failed", remain_len);
        return ret;
    }
    
    /* 检查message 长度是否足够 */
    if (*message_len < (1 + feild_len + remain_len)) {
        mqtt_log_error("message_len %u < %u", *message_len, 1 + feild_len + remain_len);
        return MQTT_MEMORY_NOBUFFER;
    }

    *message_len = 0;
    
    memcpy(message, &type_with_flags, 1);
    message += 1;
    *message_len += 1;

    memcpy(message, buffer, feild_len);
    message += feild_len;
    *message_len += feild_len;

    ret = build_string_field(topic, topic_len, message, &feild_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("build_string_field failed");
        return ret;
    }
    message += feild_len;
    *message_len += feild_len;
    
    if (qos > MQTT_QOS_0) {
        *packet_id = htons(*packet_id);
        memcpy(message, packet_id, sizeof(uint16_t));
        message += sizeof(uint16_t);
        *message_len += sizeof(uint16_t);
    }
    
    memcpy(message, payload, payload_len);
    message += payload_len;
    *message_len += payload_len;

    return MQTT_SUCCESS;
}

/**
 * @brief publish message parse
 *        NOTE：出参中主题和载荷是其在原始报文中的起始位置，函数中不重新申请空间
 *              因此，在这两部分使用结束前，原始报文都不应该被释放。
 * 
 * @param message       原始报文
 * @param message_len   原始报文长度
 * @param dup           dup
 * @param qos           qos
 * @param retain        retain
 * @param topic         主题指针
 * @param topic_len     主题长度
 * @param packet_id     packet_id
 * @param payload       载荷指针
 * @param payload_len   载荷长度
*/
int publish_message_parse(uint8_t *message, uint32_t message_len,
                          uint8_t *dup, uint8_t *qos, uint8_t *retain,
                          uint8_t **topic, uint16_t *topic_len, 
                          uint16_t *packet_id,
                          uint8_t **payload, uint32_t *payload_len)
{
    if (message == NULL || message_len == 0 || \
            dup == NULL || qos == NULL || retain == NULL || \
            topic == NULL || topic_len == NULL || \
            packet_id == NULL || \
            payload == NULL || payload_len == NULL) {
        return MQTT_INVALID_PARAM;
    }

    if ((*message & MESSAGE_TYPE_MASK) != message_type_encode(MQTT_MESSAGE_TYPE_PUBLISH)) {
        mqtt_log_error("this message is not PUBLISH.");
        return MQTT_MESSAGE_TYPE_ERROR;
    }

    *dup = (*message & (1 << PUBLISH_FLAG_INDEX_DUP)) ? 1 : 0;
    *retain = (*message & (1 << PUBLISH_FLAG_INDEX_RETAIN)) ? 1 : 0;
    if ((*message & (1 << PUBLISH_FLAG_INDEX_QOS_LOW)) && (*message & (1 << PUBLISH_FLAG_INDEX_QOS_HIGH))) {
        mqtt_log_error("parse qos error.");
        return MQTT_PUBLISH_QOS_ERROR;
    } else if (*message & (1 << PUBLISH_FLAG_INDEX_QOS_HIGH)) {
        *qos = MQTT_QOS_2;
    } else if (*message & (1 << PUBLISH_FLAG_INDEX_QOS_LOW)) {
        *qos = MQTT_QOS_1;
    } else {
        *qos = MQTT_QOS_0;
    }
    message += 1;
    message_len -= 1;

    int ret = 0;
    uint32_t feild_len, remain_len;
    
    feild_len = message_len;
    ret = remain_length_decode(message, &feild_len, &remain_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain_length_decode failed");
        return ret;
    }
    message += feild_len;
    message_len -= feild_len;
    if (remain_len > message_len) {
        mqtt_log_error("remain_len %u > message_len %u", remain_len, message_len);
        return MQTT_PUBLISH_MESSAGE_ERROR;
    }
    
    ret = locate_string_field(message, message_len, topic, topic_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("locate_string_field failed");
        return ret;
    }
    message += (*topic_len + 2);
    message_len -= (*topic_len + 2);
    
    if (*qos > MQTT_QOS_0) {
        memcpy(packet_id, message, sizeof(uint16_t));
        *packet_id = ntohs(*packet_id);
        message += sizeof(uint16_t);
        message_len -= sizeof(uint16_t);
    } else {
        *packet_id = 0;
    }
    
    /* 载荷指针和长度 */
    *payload = message;
    *payload_len = message_len;

    return MQTT_SUCCESS;
}