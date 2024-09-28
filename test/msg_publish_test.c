#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "msg_publish.h"
#include "mini_test.h"
#include "mqtt.h"
#include "common.h"


#define TEST_BUFFER_LEN 512
int test_publish_message()
{
    char *topic = "test_topic";
    uint16_t topic_len = strlen(topic);
    char *payload = "Hello MQTT!";
    uint32_t payload_len = strlen(payload);
    uint8_t message[TEST_BUFFER_LEN];
    uint32_t message_len = TEST_BUFFER_LEN;
    int ret;

    ret = publish_message_build(0, (uint8_t)MQTT_QOS_0, 0, 
                (uint8_t *)topic, topic_len, 
                NULL, 
                (uint8_t *)payload, payload_len, 
                message, &message_len);
    if(ret != MQTT_SUCCESS) {
        mini_printf("publish_message_build failed");
        return -1;
    }
    
    mini_hexdump(message, message_len);
    
    uint8_t dup;
    uint8_t qos;
    uint8_t retain;
    uint16_t packet_id;
    ret = publish_message_parse(message, message_len, 
                &dup, &qos, &retain,
                (uint8_t **)&topic, &topic_len,
                &packet_id,
                (uint8_t **)&payload, &payload_len);
    if (ret != MQTT_SUCCESS) {
        mini_printf("publish_message_parse failed");
        return -1;
    }
    
    mini_printf("parse result: \n\tdup:%d, \n\tqos:%d, \n\tretain:%d, \n\ttopic:%s, \n\ttopic_len:%d, \n\tpacket_id:%d, \n\tpayload:%s, \n\tpayload_len:%d\n",
            dup, qos, retain, topic, topic_len, packet_id, payload, payload_len);
    
    return 0;
}

int main()
{
    test_publish_message();

    return 0;
}