#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <fcntl.h>


#include "network.h"
#include "log.h"
#include "mqtt.h"


int network_connect(const char *hostname, int port, network_connection_t *conn)
{  
    struct sockaddr_in server_addr;
    
    // ipv4, tcp
    conn->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn->socket_fd < 0) {
        mqtt_log_error("Failed to create socket");
        return MQTT_BROKER_CONNECT_ERROR;
    }
    
    // set server address
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, hostname, &server_addr.sin_addr) <= 0) {
        mqtt_log_error("Invalid hostname or hostname not support.");
        return MQTT_BROKER_HOSTNAME_ERROR;
    }

    // connect to server
    if (connect(conn->socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) {
        mqtt_log_error("Failed to connect to server");
        return MQTT_BROKER_CONNECT_ERROR;
    }

    return MQTT_SUCCESS;
}


int network_close(network_connection_t *conn) 
{
    close(conn->socket_fd);
}


int network_send(network_connection_t *conn, const void *buffer, size_t len)
{
    return send(conn->socket_fd, buffer, len, 0);
}


int network_recv(network_connection_t *conn, void *buffer, size_t len)
{
    return recv(conn->socket_fd, buffer, len, 0);
}

int network_set_timeout(network_connection_t *conn, int seconds)
{
    if (conn == NULL) {
        return MQTT_INVALID_PARAM;
    }
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    setsockopt(conn->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    return MQTT_SUCCESS;
}

int network_clean_timeout(network_connection_t *conn) 
{
    return network_set_timeout(conn, 0);
}

int network_unblock(network_connection_t *conn)
{
    int flags = fcntl(conn->socket_fd, F_GETFL, 0);
    return fcntl(conn->socket_fd, F_SETFL, flags | O_NONBLOCK, 0);
}

int network_block(network_connection_t *conn)
{
    int flags = fcntl(conn->socket_fd, F_GETFL, 0);
    return fcntl(conn->socket_fd, F_SETFL, flags & ~O_NONBLOCK, 0);
}