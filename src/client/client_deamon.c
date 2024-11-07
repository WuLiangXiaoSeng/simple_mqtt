/**
 *  client deamon
 * 
 *  在CONNECT、CONNACK流程后，连接建立之后
 *  主要任务是 处理收到的消息，对已发送消息检查是否超时重发、维持心跳
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "log.h"
#include "mqtt.h"
#include "network.h"
#include "message.h"
#include "message_keep.h"
#include "client_deamon.h"
#include "msg_pingreq.h"
#include "client_message_process.h"


#define MAX_EVENTS 3
#define TIME_STEP_SEC 1
#define BUFFER_SIZE 1024
#define MESSAGE_HEADER_LENGTH 5

time_t keep_alive_timestamp;
pthread_mutex_t lock;
message_send_t *message_send_list;

pthread_mutex_t buffer_lock;
uint8_t buffer[BUFFER_SIZE];

void publish_cb_default(uint8_t *topic, uint16_t topic_len, uint8_t *payload, uint16_t payload_len)
{
    mqtt_log_debug("recieve publish message from: %s, length: %u", topic, payload_len);
    printf("[%s]: %s\n", topic, payload);
    return;
}

void timeout_cb_default(message_send_t *msg)
{
    mqtt_log_debug("timeout for packet id: %u", msg->packet_id);
    return;
}

// 回调函数指针
// timeout_callback timeout_cb = NULL;
// publish_callback publish_cb = NULL;
timeout_callback timeout_cb = publish_cb_default;
publish_callback publish_cb = timeout_cb_default;

int publish_callback_register(publish_callback pc)
{
    publish_cb = pc;
    return 0;
}

int timeout_callback_register(timeout_callback tc)
{
    timeout_cb = tc;
    return 0;
}


int init_global_timer(time_t timeout)
{
    struct itimerspec ts;
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd == -1) {
        perror("timerfd_create");
        return -1;
    }

    ts.it_interval.tv_sec = timeout;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = timeout;
    ts.it_value.tv_nsec = 0;

    if (timerfd_settime(timer_fd, 0, &ts, NULL) == -1) {
        perror("timerfd_settime");
        close(timer_fd);
        return -1;
    }

    return timer_fd;
}

int process_socket_event(deamon_context_t *ctx) 
{
    int ret;
    int recv_len = 0;
    uint32_t remain_len, message_len;
    mqtt_message_type_t message_type;
    
    do {
        memset(buffer, 0, BUFFER_SIZE);
        recv_len = network_recv(&ctx->client->conn, buffer, MESSAGE_HEADER_LENGTH);
        if (recv_len < 0) {
            mqtt_log_error("recieve message failed, errno: %d", errno);
            return MQTT_MESSAGE_RECV_ERROR;
        }

        message_len = recv_len;
        ret = message_fixed_header_decode(buffer, &message_len, &message_type, &remain_len);
        if (ret < 0) {
            mqtt_log_error("remain length decode failed");
            return ret;
        }

        /* 如果报文长度大于recv_len，继续接受剩余部分 */
        if (message_len > recv_len) {
            ret = network_recv(&ctx->client->conn, buffer + recv_len, message_len - recv_len);
            if (ret < 0) {
                mqtt_log_error("recieve message failed, errno: %d", errno);
                return MQTT_MESSAGE_RECV_ERROR;
            }
        }

        switch (message_type) {
            case MQTT_MESSAGE_TYPE_CONNECT:
                mqtt_log_warn("receive a connect message, ignore");
                break;
            case MQTT_MESSAGE_TYPE_CONNACK:
                mqtt_log_warn("receive a connack message, ignore."); /*client deamon 创建时，已经和服务端建立连接 */
                break;
            case MQTT_MESSAGE_TYPE_PUBLISH:
                ret = publish_msg_process(ctx, buffer, message_len, publish_cb);
                break;
            case MQTT_MESSAGE_TYPE_PUBACK:
                ret = puback_msg_process(ctx, buffer, message_len);
                break;
            case MQTT_MESSAGE_TYPE_PUBREC:
                ret = pubrec_msg_process(ctx, buffer, message_len);
                break;
            case MQTT_MESSAGE_TYPE_PUBREL:
                ret = pubrel_msg_process(ctx, buffer, message_len);
                break;
            case MQTT_MESSAGE_TYPE_PUBCOMP:
                ret = pubcomp_msg_process(ctx, buffer, message_len);
                break;
            case MQTT_MESSAGE_TYPE_SUBSCRIBE:
                mqtt_log_warn("receive a subscribe message, ignore");
                break;
            case MQTT_MESSAGE_TYPE_SUBACK:
                ret = suback_msg_process(ctx, buffer, message_len);
                break;
            case MQTT_MESSAGE_TYPE_UNSUBSCRIBE:
                mqtt_log_warn("receive a unsubscribe message, ignore");
                break;
            case MQTT_MESSAGE_TYPE_UNSUBACK:
                ret = unsuback_msg_process(ctx, buffer, message_len);
                break;
            case MQTT_MESSAGE_TYPE_PINGREQ:
                mqtt_log_warn("receive a ping request message, ignore");
                break;
            case MQTT_MESSAGE_TYPE_PINGRESP:
                ret = pingresp_msg_process(ctx, buffer, message_len);
                break;
            case MQTT_MESSAGE_TYPE_DISCONNECT:
                mqtt_log_warn("receive a disconnect message, ignore");
                break;
            default:
                mqtt_log_error("unknown message type: %d", message_type);
                return MQTT_MESSAGE_TYPE_ERROR;
        }
        
    } while (ret <= BUFFER_SIZE);
    
    
    
    return 0;
}

int process_timer_event(deamon_context_t *ctx)
{
    int ret;
    message_send_t *curr;
    uint64_t exp;
    ssize_t n = read(ctx->timer_fd, &exp, sizeof(uint64_t));
    if (n == -1) {
        perror("read");
        return -1;
    }

    mqtt_log_debug("timer event, exp: %lu", exp);
    time_t now = time(NULL);
    pthread_mutex_lock(&lock);

    /* 检查keep_alive_timestamp是否超时 */
    if (now - keep_alive_timestamp >= ctx->client->keep_alive) {
        mqtt_log_debug("keep alive timeout");
        ret = pingreq_msg_sent(ctx);
        if (ret != MQTT_SUCCESS) {
            goto error;
        }
        keep_alive_timestamp = time(NULL);
    }
    
    curr = message_send_list;
    while (curr != NULL) {
        if (curr->timestamp + ctx->client->timeout_sec < now) {
            /* 超时 */
            mqtt_log_debug("message timeout, packet id: %u", curr->packet_id);
            timeout_cb(curr);
            message_send_remove(&message_send_list, curr->packet_id);
        }
        curr = curr->next;
    }
    
    ret = MQTT_SUCCESS;

error:
    pthread_mutex_unlock(&lock);
    return ret;
}

void *client_deamon_thread(void *arg)
{
    deamon_context_t *ctx = (deamon_context_t *)arg;
    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int nfds = epoll_wait(ctx->epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue; // 被信号中断，重试
            }
            perror("epoll_wait");
            break;
        }
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == ctx->sock_fd) {
                /* 处理socket事件 */
                process_socket_event(ctx);
            } else if (events[i].data.fd == ctx->timer_fd) {
                process_timer_event(ctx);
            }
        }
    }
    
    return NULL;
}

/**
 *  client deamon 创建
 * 
*/
int client_deamon_create(mqtt_client_t *client, pthread_t *ph)
{
    if (ph == NULL) {
        mqtt_log_error("ph is NULL");
        return MQTT_INVALID_PARAM;
    }
    int ret = 0;
    struct epoll_event event;
    deamon_context_t *ctx = (deamon_context_t *)malloc(sizeof(deamon_context_t));
    if (ctx == NULL) {
        mqtt_log_error("malloc failed");
        return MQTT_MEMORY_NOBUFFER;
    }

    ctx->sock_fd = client->conn.socket_fd;
    /* 初始化全局定时器 */
    ctx->timer_fd = init_global_timer(TIME_STEP_SEC);
    if (ctx->timer_fd == -1) {
        ret = -1;
        goto free;
    }

    ctx->epoll_fd = epoll_create1(0);
    if (ctx->epoll_fd == -1) {
        perror("epoll_create1");
        ret = -1;
        goto close1;
    }

    /* 将套接字加入epoll监听队列 */
    event.data.fd = ctx->sock_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, ctx->sock_fd, &event) == -1) {
        perror("epoll_ctl: socket fd");
        ret = -1;
        goto close2;
    }

    /* 将定时器加入epoll监听队列 */
    event.data.fd = ctx->timer_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, ctx->timer_fd, &event) == -1) {
        perror("epoll_ctl: timer fd");
        ret = -1;
        goto close2;
    }

    /* 初始化线程锁 */
    pthread_mutex_init(&lock, NULL);
    ctx->client = client;
    
    if (pthread_create(ph, NULL, client_deamon_thread, ctx) != 0) {
        mqtt_log_error("pthread_create failed");
        ret = -1;
        goto close2;
    }

    ret = MQTT_SUCCESS;
    
close2:
    close(ctx->epoll_fd);
close1:
    close(ctx->timer_fd);
free:
    free(ctx);
    return ret;
}
