#ifndef __MQTT_CLIENT_DEAMON_H__
#define __MQTT_CLIENT_DEAMON_H__

#include "client.h"
#include "message_keep.h"

typedef void (*publish_callback)(uint8_t *topic, uint16_t topic_len, uint8_t *payload, uint16_t payload_len);
typedef void (*timeout_callback)(message_send_t *msg);

typedef struct deamon_context_ {
    mqtt_client_t *client;
    int sock_fd;
    int timer_fd;
    int epoll_fd;
} deamon_context_t;

// typedef struct deamon_shared_data_ {
//     int 
// } deamon_shared_data_t;
#endif