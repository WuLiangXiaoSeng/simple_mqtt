#include <stdlib.h>
#include <string.h>

#include "msg_connect.h"
#include "mini_test.h"
#include "mqtt.h"


int test_connect_payload_release(void) {
    return 0;
}

int test_connect_message(void) {
    uint32_t message_len = 0;
    connect_payload_t conn_payload = {0};
    uint16_t keep_alive = 60;
    uint8_t clean_session = 0;
    uint8_t will_retain = 0;
    uint8_t will_qos = 2;

    message_len = 5 + sizeof(connect_variable_header_t);
    conn_payload.client_id = "client_id";
    conn_payload.client_id_len = strlen(conn_payload.client_id);
    message_len += conn_payload.client_id_len;
    conn_payload.will_topic = "will_topic";
    conn_payload.will_topic_len = strlen(conn_payload.will_topic);
    message_len += conn_payload.will_topic_len;
    conn_payload.will_msg = "will_msg";
    conn_payload.will_msg_len = strlen(conn_payload.will_msg);
    message_len += conn_payload.will_msg_len;
    conn_payload.username = "username";
    conn_payload.username_len = strlen(conn_payload.username);
    message_len += conn_payload.username_len;
    conn_payload.password = "password";
    conn_payload.password_len = strlen(conn_payload.password);
    message_len += conn_payload.password_len;
    
    minitest_printf("message_len: %u\n", message_len);
    uint8_t *message = (uint8_t *)malloc(message_len);
    if (message == NULL) {
        minitest_printf("malloc failed\n");
        return -1;
    }
    int ret = connect_message_build(&conn_payload, keep_alive, clean_session, will_retain, will_qos, message, &message_len);
    if (ret != MQTT_SUCCESS) {
        minitest_printf("connect_message_build failed\n");
        free(message);
        return -1;
    }
    minitest_printf("message_len: %u\n", message_len);
    minitest_hexdump(message, message_len);

    connect_payload_t conn_payload_parse = {0};
    ret = connect_message_parse(message, message_len, &conn_payload_parse, &keep_alive, &clean_session, &will_retain, &will_qos);
    if (ret != MQTT_SUCCESS) {
        minitest_printf("connect_message_parse failed\n");
        free(message);
        return -1;
    }
    minitest_printf("keep_alive: %u\n", keep_alive);
    minitest_printf("clean_session: %u\n", clean_session);
    minitest_printf("will_retain: %u\n", will_retain);
    minitest_printf("will_qos: %u\n", will_qos);
    minitest_printf("client_id: %s, len: %u\n", conn_payload_parse.client_id, conn_payload_parse.client_id_len);
    minitest_printf("will_topic: %s, len: %u\n", conn_payload_parse.will_topic, conn_payload_parse.will_topic_len);
    minitest_printf("will_msg: %s, len: %u\n", conn_payload_parse.will_msg, conn_payload_parse.will_msg_len);
    minitest_printf("username: %s, len: %u\n", conn_payload_parse.username, conn_payload_parse.username_len);
    minitest_printf("password: %s, len: %u\n", conn_payload_parse.password, conn_payload_parse.password_len);
    
    connect_payload_release(&conn_payload_parse);
    free(message);
    return 0;
}


int main() {
    getchar();
    test_connect_message();

    return 0;
}