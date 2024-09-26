#include <stdlib.h>
#include <string.h>

#include "msg_connack.h"
#include "mini_test.h"
#include "mqtt.h"

int test_connack_message()
{
    connack_code_t ack_code = MQTT_CONNACK_NOT_AUTHORIZED;
    uint8_t sp = 1;
    uint8_t message[10];  // ack message 最多只会有4字节
    uint32_t message_len = 10;

    minitest_printf("test_connack_message\n");
    int ret;

    ret = connack_message_build(sp, ack_code, message, &message_len);
    if (ret != MQTT_SUCCESS) {
        minitest_printf("connack_message_build failed\n");
        return -1;
    }
    minitest_printf("connack_message_build success\n");
    minitest_hexdump(message, message_len);
    
    connack_code_t ack_code_get;
    uint8_t sp_get = 0;
    ret = connack_message_parse(message, message_len, &ack_code_get, &sp_get);
    if (ret != MQTT_SUCCESS) {
        minitest_printf("connack_message_parse failed\n");
        return -1;
    }
    minitest_printf("connack_message_parse success\n");
    minitest_printf("ack code: %u", ack_code_get);
    minitest_printf("sp: %u", sp_get);

    return 0;
}


int main()
{
    test_connack_message();

    return 0;
}