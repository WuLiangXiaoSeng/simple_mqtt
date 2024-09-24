#ifndef _NETORK_H_
#define _NETORK_H_

typedef struct {
    int socket_fd;
} network_connection_t;


int network_connect(const char *hostname, int port, network_connection_t *conn);
int network_close(network_connection_t *conn);
int network_send(network_connection_t *conn, const void *buffer, size_t len);
int network_recv(network_connection_t *conn, void *buffer, size_t len);

#endif