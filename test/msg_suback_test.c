#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "msg_suback.h"
#include "mini_test.h"
#include "mqtt.h"
#include "common.h"

#define TEST_BUFFER_LEN 512

int test_suback_message()
{
    int ret = 0;
    uint8_t message[TEST_BUFFER_LEN];
    suback_data_t data;
    uint32_t message_len = TEST_BUFFER_LEN;
    uint16_t packet_id = 0;

    uint8_t qos[10] = {
        MQTT_QOS_0, MQTT_QOS_0, MQTT_QOS_1, MQTT_QOS_2, MQTT_QOS_1, 
        MQTT_QOS_2, MQTT_QOS_0, MQTT_QOS_1, MQTT_QOS_2, MQTT_QOS_REJECT,
    };
    data.qos = qos;
    data.capacity = 10;
    data.size = 10;

    ret = suback_message_build(0x0001, &data, message, &message_len);
    if (ret != MQTT_SUCCESS) {
        mini_printf("suback_message_build failed, ret=%d\n", ret);
        return ret;
    }

    mini_printf("SUBACK message build success\n");
    mini_hexdump(message, message_len);

    ret = suback_message_parse(message, message_len, &packet_id, &data);
    if (ret != MQTT_SUCCESS) {
        mini_printf("suback_message_parse failed, ret=%d\n", ret);
        return ret;
    }

    if (packet_id != 0x0001) {
        mini_printf("packet_id parse failed, expect=0x0001, actual=0x%04x\n", packet_id);
    } else {
        mini_printf("packet_id parse success\n");
    }
    
    for (uint16_t i = 0; i < data.size; i++) {
        if (data.qos[i] != qos[i]) {
            mini_printf("qos[%d] parse failed, expect=%d, actual=%d\n", i, qos[i], data.qos[i]);
        } else {
            mini_printf("qos[%d] parse success\n", i);
        }
    }

    suback_data_release(&data);
    
    mini_printf("suback message test finished.")
    return 0;
}

int main() 
{
    test_suback_message();

    return 0;
}