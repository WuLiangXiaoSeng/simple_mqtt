#include <stdlib.h>
#include <stdint.h>

#include "log.h"
#include "mqtt.h"
#include "common.h"
#include "client_deamon.h"
#include "msg_pingresp.h"
#include "msg_publish.h"
#include "msg_puback.h"
#include "msg_pubrec.h"
#include "msg_pubrel.h"
#include "msg_pubcomp.h"
#include "msg_suback.h"
#include "msg_unsuback.h"

int pingreq_msg_sent(deamon_context_t *ctx)
{
    int ret;
    uint8_t message[PINGREQ_MESSAGE_LENGTH];
    uint32_t message_len = PINGREQ_MESSAGE_LENGTH;
    ret = pingreq_message_build(message, &message_len);
    if (ret != 0) {
        mqtt_log_error("build ping request message failed");
        return ret;
    }

    ret = network_send(&ctx->client->conn, message, message_len);
    if (ret == -1) {
        mqtt_log_error("send ping request message failed");
        return MQTT_MESSAGE_SEND_ERROR;
    }
    
    return MQTT_SUCCESS;
}

int puback_msg_sent(deamon_context_t *ctx, uint16_t packet_id)
{
    int ret;
    uint8_t message[PUBACK_MESSAGE_LENGTH];
    uint32_t message_len = PUBACK_MESSAGE_LENGTH;

    ret = puback_message_build(packet_id, message, &message_len);
    if (ret != 0) {
        mqtt_log_error("build puback message failed");
        return ret;
    }

    ret = network_send(&ctx->client->conn, message, message_len);
    if (ret == -1) {
        mqtt_log_error("send puback message failed");
        return MQTT_MESSAGE_SEND_ERROR;
    }

    return MQTT_SUCCESS;
}


int pubrec_msg_sent(deamon_context_t *ctx, uint16_t packet_id)
{
    int ret;
    uint8_t message[PUBREC_MESSAGE_LENGTH];
    uint32_t message_len = PUBREC_MESSAGE_LENGTH;

    ret = pubrec_message_build(packet_id, message, &message_len);
    if (ret != 0) {
        mqtt_log_error("build pubrec message failed");
        return ret;
    }

    ret = network_send(&ctx->client->conn, message, message_len);
    if (ret == -1) {
        mqtt_log_error("send pubrec message failed");
        return MQTT_MESSAGE_SEND_ERROR;
    }

    return MQTT_SUCCESS;
}

int pubrel_msg_sent(deamon_context_t *ctx, uint16_t packet_id)
{
    int ret;
    uint8_t message[PUBREL_MESSAGE_LENGTH];
    uint32_t message_len = PUBREL_MESSAGE_LENGTH;

    ret = pubrel_message_build(packet_id, message, &message_len);
    if (ret!= 0) {
        mqtt_log_error("build pubrel message failed");
        return ret;
    }
    
    ret = network_send(&ctx->client->conn, message, message_len);
    if (ret == -1) {
        mqtt_log_error("send pubrel message failed");
        return MQTT_MESSAGE_SEND_ERROR;
    }
    
    return MQTT_SUCCESS;
}


int pubcomp_msg_sent(deamon_context_t *ctx, uint16_t packet_id)
{
    int ret;
    uint8_t message[PUBCOMP_MESSAGE_LENGTH];
    uint32_t message_len = PUBCOMP_MESSAGE_LENGTH;

    ret = pubcomp_message_build(packet_id, message, &message_len);
    if (ret != 0) {
        mqtt_log_error("build pubcomp message failed");
        return ret;
    }

    ret = network_send(&ctx->client->conn, message, message_len);
    if (ret == -1) {
        mqtt_log_error("send pubcomp message failed");
        return MQTT_MESSAGE_SEND_ERROR;
    }

    return MQTT_SUCCESS;
}


int publish_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len, publish_callback pub_cb)
{
    int ret;
    uint8_t dup, qos, retain;
    uint16_t packet_id;
    uint8_t *topic, *payload;
    uint32_t payload_len, topic_len;

    ret = publish_message_parse(message, message_len, &dup, &qos, &retain, &topic, &topic_len, &packet_id, &payload, &payload_len);
    if (ret != 0) {
        mqtt_log_error("parse publish message failed");
        return ret;
    }

    pub_cb(topic, topic_len, payload, payload_len);
    
    if (qos == MQTT_QOS_1) {
        ret = puback_msg_sent(ctx, packet_id);
    } else if (qos == MQTT_QOS_2) {
        ret = pubrec_msg_sent(ctx, packet_id);
    }

    return ret;
}


int puback_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len)
{
    int ret = MQTT_SUCCESS;
    uint16_t packet_id;
    
    ret = puback_message_parse(message, message_len, &packet_id);
    if (ret != 0) {
        mqtt_log_error("parse puback message failed");
        return ret;
    }
    
    return ret;
}

int pubrec_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len)
{
    int ret = MQTT_SUCCESS;
    uint16_t packet_id;
    
    ret = pubrec_message_parse(message, message_len, &packet_id);
    if (ret != 0) {
        mqtt_log_error("parse pubrec message failed");
        return ret;
    }
    
    ret = pubrel_msg_sent(ctx, packet_id);
    return ret;
}

int pubcomp_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len)
{
    int ret = MQTT_SUCCESS;
    uint16_t packet_id;
    
    ret = pubcomp_message_parse(message, message_len, &packet_id);
    if (ret != 0) {
        mqtt_log_error("parse pubcomp message failed");
        return ret;
    }

    return ret;
}

int suback_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len)
{
    int ret = MQTT_SUCCESS;
    uint16_t packet_id;
    suback_data_t data;
    
    memset(&data, 0, sizeof(suback_data_t));
    
    ret = suback_message_parse(message, message_len, &packet_id, &data);
    if (ret != 0) {
        mqtt_log_error("parse suback message failed");
        suback_data_release(&data);
        return ret;
    }
    /*TODO: process suback data and packet id*/

    return MQTT_SUCCESS;
}

int unsuback_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len)
{
    int ret = MQTT_SUCCESS;
    uint16_t packet_id;
    
    ret = unsuback_message_parse(message, message_len, &packet_id);
    if (ret != 0) {
        mqtt_log_error("parseunsuback message failed");
        return ret;
    }

    return MQTT_SUCCESS;
}


int pingresp_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len)
{
    return MQTT_SUCCESS;
}