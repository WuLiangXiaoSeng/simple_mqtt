#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include "network.h"

typedef enum {
    MQTT_CLIENT_STATE_DISCONNECTED,
    MQTT_CLIENT_STATE_CONNECTED,
} mqtt_client_state;

typedef struct mqtt_client_ {
    mqtt_client_state state;
    network_connection_t conn;
    uint16_t keep_alive;
    uint16_t timeout_sec;
    uint8_t *client_id;
    uint16_t client_id_len;
    uint8_t sp;
    uint16_t packet_id_next;
} mqtt_client_t;

typedef struct mqtt_client_config_ {
    char hostname[32];
    uint16_t port;
    uint16_t keep_alive;
    uint8_t clean_session;
    /* 暂时不支持（全设置成 0）*/
    uint8_t will_retain;
    uint8_t will_qos;
    /* payload */
    uint8_t *client_id;
    uint16_t client_id_len;
    uint8_t *username;
    uint16_t username_len;
    uint8_t *password;
    uint16_t password_len;
    
    /* 暂时不支持 */
    uint8_t *will_topic;
    uint16_t will_topic_len;
    uint8_t *will_msg;
    uint16_t will_msg_len;
} mqtt_client_config_t;

#endif