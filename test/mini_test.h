#ifndef _SELF_TEST_H_
#define _SELF_TEST_H_

#include <stdio.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define END "\033[0m"

#define MINITEST_PREFIX "[MINITEST %s:%d] "

#define minitest_printf(format, args...) \
    printf(MINITEST_PREFIX format "\n", __func__, __LINE__, ##args);

#define minitest_hexdump(data, len) \
    do { \
        int i; \
        for (i = 0; i < len; i++) { \
            printf("%02X ", data[i]); \
            if ((i + 1) % 16 == 0) printf("\n"); \
        } \
        printf("\n"); \
    } while(0);
#endif