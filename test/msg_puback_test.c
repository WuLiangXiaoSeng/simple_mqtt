#include <stdint.h>

#include "msg_puback.h"
#include "mini_test.h"
int test_puback_message() 
{
    uint8_t message[10];
    uint32_t message_len = 10;
    uint16_t packet_id = 0x00F0;

    int ret = puback_message_build(packet_id, message, &message_len);
    if (ret != 0) {
        return ret;
    }

    mini_hexdump(message, message_len);

    ret = puback_message_parse(message, message_len, &packet_id);
    if (ret != 0) {
        return ret;
    }
    mini_printf("packet_id: 0x%04X\n", packet_id);
    
    return 0;
}


int main()
{
    test_puback_message();

    return 0;
}