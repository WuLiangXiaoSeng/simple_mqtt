#ifndef _NETORK_H_
#define _NETORK_H_

#define MQTT_TCP_PORT 1883
// #define MQTT_SSL_PORT 8883

typedef struct {
    int socket_fd;
} network_connection_t;


int network_connect(const char *hostname, int port, network_connection_t *conn);
int network_close(network_connection_t *conn);
int network_send(network_connection_t *conn, const void *buffer, size_t len);
int network_recv(network_connection_t *conn, void *buffer, size_t len);
int network_set_timeout(network_connection_t *conn, int seconds);
int network_clean_timeout(network_connection_t *conn);

int network_unblock(network_connection_t *conn);
int network_block(network_connection_t *conn);

#endif