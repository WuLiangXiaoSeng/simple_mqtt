#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "mini_test.h"
#include "mqtt.h"



int test_remain_length_encode()
{
    uint32_t length[] = {
        0, 127, 128, 16383, 16384, 2097151, 2097152, 268435455
    };
    uint8_t encode[][4] = {
        {0x00},
        {0x7F},
        {0x80, 0x01},
        {0xFF, 0x7F},
        {0x80, 0x80, 0x01},
        {0xFF, 0xFF, 0x7F},
        {0x80, 0x80, 0x80, 0x01},
        {0xFF, 0xFF, 0xFF, 0x7F}
    };
    uint32_t encode_len[] = {
        1, 1, 2, 2, 3, 3, 4, 4
    };
    int ret;
    uint8_t buffer[4];
    uint32_t buffer_len;
    uint32_t failed = 0;

    for (int i = 0; i < 8; i++) {
        ret = remain_length_encode(length[i], buffer, &buffer_len);
        if (ret != MQTT_SUCCESS) {
            mini_printf("remain_length_encode %u failed\n", length[i]);
            failed += 1;
            continue;
        }
        mini_hexdump(buffer, buffer_len);
        if (buffer_len != encode_len[i] || memcmp(buffer, encode[i], buffer_len)) {
            mini_printf("remain_length_encode %u failed\n", length[i]);
            failed += 1;
        }
    }

    if (failed != 0) {
        mini_printf(RED "FAILED %u/8" END, failed);
    } else {
        mini_printf(GREEN "PASSED" END);
    }
    
    return 0;
}


int test_remain_length_decode()
{
    uint32_t length[] = {
        0, 127, 128, 16383, 16384, 2097151, 2097152, 268435455
    };
    int ret;
    uint8_t buffer[4];
    uint32_t buffer_len_e;
    uint32_t buffer_len_d;
    uint32_t len;
    uint32_t failed = 0;

    for (int i = 0; i < 8; i++) {
        remain_length_encode(length[i], buffer, &buffer_len_e);
        mini_hexdump(buffer, buffer_len_e);
        buffer_len_d = buffer_len_e;
        ret = remain_length_decode(buffer, &buffer_len_d, &len);
        if (ret != MQTT_SUCCESS) {
            mini_printf("remain_length_decode %u failed\n", length[i]);
            failed += 1;
            continue;
        }

        if (len != length[i] || buffer_len_d != buffer_len_e) {
            mini_printf("remain_length_decode %u failed, result: %u\n", length[i], len);
            failed += 1;
        } else {
            mini_printf("remain_length_decode %u success, result: %u\n", length[i], len);
        }
    }

    if (failed != 0) {
        mini_printf(RED "FAILED %u/8" END, failed);
    } else {
        mini_printf(GREEN "PASSED" END);
    }

    return 0;
}

int main()
{
    getchar();
    // test_remain_length_encode();
    test_remain_length_decode();
    
    return 0;
}