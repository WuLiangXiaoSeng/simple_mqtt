#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "log.h"
#include "message_keep.h"
#include "mqtt.h"
#include "client.h"
#include "network.h"
#include "msg_connect.h"
#include "msg_connack.h"
#include "msg_subscribe.h"

#define MQTT_CLIENT_CONNECT_RETRY_MAX 3
#define MQTT_CLIENT_NETWORK_TIMEOUT 5 /* seconds*/


extern time_t keep_alive_timestamp;
extern message_send_t *message_send_list;
extern pthread_mutex_t lock;

static int message_send_and_wait_ack(network_connection_t *conn, uint8_t *message, uint32_t message_len, 
                                        uint8_t *ack_buffer, uint32_t *ack_len)
{
    int retry = 0;
    int ret = 0;
    
    network_set_timeout(conn, MQTT_CLIENT_NETWORK_TIMEOUT);
    while (retry < MQTT_CLIENT_CONNECT_RETRY_MAX) {
        mqtt_log_debug("send connect message, retry %d", retry);
        ret = network_send(conn, message, message_len);
        if (ret == -1) {
            mqtt_log_error("network_send failed, errno: %d", errno);
            ret = MQTT_MESSAGE_SEND_ERROR;
            goto exit;
        }

        mqtt_log_debug("connect message sent, waitting for connack...");
        ret = network_recv(conn, ack_buffer, *ack_len);
        if (ret > 0) {
            mqtt_log_debug("recv message len: %d", ret);
            *ack_len = ret;
            break;
        } else if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                mqtt_log_warn("network_recv timeout, retry %d", retry);
                retry++;
            } else {
                mqtt_log_error("network_recv failed, errno: %d", errno);
                ret = MQTT_CLIENT_CONNECT_ERROR;
                goto exit;
            }
        }
    }
    if (retry >= MQTT_CLIENT_CONNECT_RETRY_MAX) {
        mqtt_log_error("network_recv timeout");
        ret = MQTT_CLIENT_CONNECT_ERROR;
        goto exit;
    }

    ret = MQTT_SUCCESS;
exit:
    network_clean_timeout(conn);
    return ret;
}

static int connect_message_send(mqtt_client_t *client, mqtt_client_config_t *config) 
{
    if (config->keep_alive == 0) {
        config->keep_alive = MQTT_KEEP_ALIVE_DEFAULT;
    }
    
    connect_payload_t conn_payload;
    memset(&conn_payload, 0, sizeof(conn_payload));
    conn_payload.client_id = config->client_id;
    conn_payload.client_id_len = config->client_id_len;
    conn_payload.will_topic = config->will_topic;
    conn_payload.will_topic_len = config->will_topic_len;
    conn_payload.will_msg = config->will_msg;
    conn_payload.will_msg_len = config->will_msg_len;
    conn_payload.username = config->username;
    conn_payload.username_len = config->username_len;
    conn_payload.password = config->password;
    conn_payload.password_len = config->password_len;

    int ret;
    uint32_t message_len;
    uint8_t *message;
    uint8_t connack_message[CONNACK_MESSAGE_LEN] = {0};
    uint32_t ack_len = CONNACK_MESSAGE_LEN;
    connack_code_t connack_code;

    ret = connect_message_len_calc(&conn_payload, &message_len);
    message = (uint8_t *)malloc(message_len);
    if (message == NULL) {
        mqtt_log_error("malloc failed, message_len: %u", message_len);
        return MQTT_MEMORY_NOBUFFER;
    }
    
    ret = connect_message_build(&conn_payload, config->keep_alive, config->clean_session,
                                config->will_retain, config->will_qos, message, &message_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("connect_message_build failed");
        goto free;
    }
    
    /* 发送CONNECT报文并等待server端回复 */
    ret = message_send_and_wait_ack(conn, message, message_len, connack_message, &ack_len);
    if (ret != MQTT_SUCCESS) {
        goto free;
    }
    
    ret = connack_message_parse(connack_message, ack_len,
                                &connack_code, &(client->sp));
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("connack_message_parse failed");
        goto free;
    }
    mqtt_log_info("connack_code: %d", connack_code);
    if (connack_code != MQTT_CONNACK_ACCEPTED) {
        mqtt_log_error("connack_code error: %d", connack_code);
        ret = MQTT_CLIENT_CONNECT_ERROR;
        goto free;
    }
    
    client->keep_alive = config->keep_alive;
    keep_alive_timestamp = time(NULL);
    client->client_id = (uint8_t *)malloc(config->client_id_len);
    memcpy(client->client_id, config->client_id, config->client_id_len);
    client->client_id_len = config->client_id_len;
    /* packet id init */
    client->packet_id_next = 1;
    client->message_send_list_size = 0;
    client->message_send_list = NULL;
    
    client->state = MQTT_CLIENT_STATE_CONNECTED;
    
    free(message);
    return MQTT_SUCCESS;

free:
    free(message);
    message = NULL;
    
    return ret;
}

int mqtt_client_create(mqtt_client_t *client, mqtt_client_config_t *config)
{
    if (config == NULL || client == NULL) {
        return MQTT_INVALID_PARAM;
    }
    int ret;
    network_connection_t *conn = &(client->conn);

    ret = network_connect(config->hostname, config->port, conn);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("network_connect failed");
        return ret;
    }
    mqtt_log_debug("network_connect success");

    ret = connect_message_send(client, config);
    if (ret != MQTT_SUCCESS) {
        goto disconnect;
    }

disconnect:
    network_close(conn);
    memset(conn, 0, sizeof(network_connection_t));

    return ret;
}

/**
 * @brief subscribe topic
 *        当前不支持多topic
 * 
 * @param client 
 * @param topic 
 * @param topic_len 
 * @param qos 
 * 
 * @return int 
*/
int mqtt_client_subscribe(mqtt_client_t *client, uint8_t *topic, uint16_t topic_len, uint8_t qos) 
{
    if (client == NULL || topic == NULL || topic_len == 0) {
        return MQTT_INVALID_PARAM;
    }

    int ret = 0;
    uint8_t *message;
    uint32_t message_len;
    topic_filter_t topic_filter;
    memset(&topic_filter, 0, sizeof(topic_filter));
    
    topic_filter.topic = topic;
    topic_filter.topic_len = topic_len;
    topic_filter.qos = qos;
    topic_filter.next = NULL;

    ret = subscribe_message_len_calc(topic_filter, &message_len);

    message = (uint8_t *)malloc(message_len);
    if (message == NULL) {
        mqtt_log_error("malloc failed, message_len: %u", message_len);
        return MQTT_MEMORY_NOBUFFER;
    }

    ret = subscribe_message_build(client->packet_id_next, topic_filter, message, &message_len);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("subscribe_message_build failed");
        goto free;
    }
    
    ret = network_send(&(client->conn), message, message_len);
    if (ret < 0) {
        mqtt_log_error("network_send failed, errno: %d", errno);
        ret = MQTT_MESSAGE_SEND_ERROR;
        goto free;
    }
    
    ret = message_send_add(&message_send_list, client->packet_id_next, message, message_len, qos);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("message_keep_add failed");
        ret = MQTT_MEMORY_NOBUFFER;
        goto free;
    }

    client->packet_id_next = (client->packet_id_next + 1);
    if (client->packet_id_next == 0) { /* 当自增溢出等于0时，重置为1 */
        client->packet_id_next = 1;
    }
    
    ret = MQTT_SUCCESS;
free:
    free(message);
    return ret;
}

int mqtt_client_publish(mqtt_client_t *client, uint8_t *topic, uint16_t topic_len, uint8_t *payload, uint32_t payload_len, uint8_t qos)
{
    if (client == NULL || topic == NULL || topic_len == 0 || payload == NULL || payload_len == 0) {
        return MQTT_INVALID_PARAM;
    }
}

int mqtt_client_destroy(mqtt_client_t *client)
{
    network_connection_t *conn = &(client->conn);

    if (client->client_id_len > 0 && client->client_id != NULL) {
        free(client->client_id);
        client->client_id = NULL;
    }
    
    return network_close(conn);
}