#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "msg_subscribe.h"
#include "mini_test.h"

#define TEST_SUBSCRIBE_MESSAGE_LEN 512
int test_subscribe_message()
{
    char *topic[3] = {
        "test_topic_1",
        "test_topic_2",
        "test_topic_3",
    };
    topic_filter_t filter_list[3] = {
        {
            .topic_filter = (uint8_t *)topic[0],
            .topic_filter_len = strlen(topic[0]),
            .qos = 0,
            .next = NULL,
        },
        {
            .topic_filter = (uint8_t *)topic[1],
            .topic_filter_len = strlen(topic[1]),
            .qos = 1,
            .next = NULL,
        },
        {
            .topic_filter = (uint8_t *)topic[2],
            .topic_filter_len = strlen(topic[2]),
            .qos = 2,
            .next = NULL,
        }
    };
    // for (int i = 0; i < 3; i++) {
    //     filter_list[i].topic_filter = (uint8_t *)topic[i];
    //     filter_list[i].topic_filter_len = strlen(topic[i]);
    //     filter_list[i].qos = i;
    // }
    filter_list[2].next = NULL;
    filter_list[1].next = &filter_list[2];
    filter_list[0].next = &filter_list[1];
    
    uint16_t packet_id = 0x00F0;
    uint8_t message[TEST_SUBSCRIBE_MESSAGE_LEN];
    uint32_t message_len = TEST_SUBSCRIBE_MESSAGE_LEN;

    int ret = subscribe_message_build(packet_id, filter_list, message, &message_len);
    if (ret != 0) {
        return -1;
    }
    mini_hexdump(message, message_len);

    topic_filter_t *filter;
    ret = subscribe_message_parse(message, message_len, &packet_id, &filter);
    if (ret != 0) {
        return -1;
    }
    mini_printf("packet_id: 0x%04X", packet_id);

    topic_filter_t *cur = filter;
    while (cur != NULL) {
        mini_printf("topic_filter:%s, length: %u, qos: %u", cur->topic_filter, cur->topic_filter_len, cur->qos);
        cur = cur->next;
    }
    
    topic_filter_release(filter);
    
    return 0;
}

int main() 
{
    test_subscribe_message();


    return 0;
}