#ifndef _MQTT_H_
#define _MQTT_H_


/* 错误返回值 */
#define MQTT_SUCCESS                    0       /* 成功 */
#define MQTT_UNKNOWN                    1       /* 未知错误 */
#define MQTT_INVALID_PARAM              2       /* 错误的参数 */
#define MQTT_MEMORY_NOBUFFER            3       /* 内存不足 */
#define MQTT_MESSAGE_TYPE_ERROR         4       /* 错误的消息类型 */
#define MQTT_PROTOCOL_UNSUPPORT         5       /* 不支持的协议 */
#define MQTT_PROTOCOL_LEVEL_UNSUPPORT   6       /* 不支持的协议版本 */
#define MQTT_CONNECT_PARSE_ERROR        7       /* CONNECT 报文解析错误 */
#define MQTT_CONNECT_MESSAGE_ERROR      8       /* CONNECT 报文错误 */
#define MQTT_CLIENT_ID_LENGTH_UNSUPPORT 9       /* 客户端ID长度不支持 */
#define MQTT_CONNECT_FLAGS_ERROR        10      /* 错误的连接标志位 */
#define MQTT_CONNACK_MESSAGE_ERROR      11      /* CONNACK 报文错误 */
#define MQTT_CONNACK_CODE_ERROR         12      /* CONNACK CODE错误 */
#define MQTT_PUBLISH_MESSAGE_ERROR      13
#define MQTT_PUBLISH_QOS_ERROR          14      /* PUBLISH QOS错误 */
#define MQTT_PUBACK_MESSAGE_ERROR       15
#define MQTT_PUBREC_MESSAGE_ERROR       16
#define MQTT_PUBREL_MESSAGE_ERROR       17
#define MQTT_PUBCOMP_MESSAGE_ERROR      18
#define MQTT_SUBSCRIBE_MESSAGE_ERROR    19
#define MQTT_SUBACK_QOS_ERROR           20
#define MQTT_SUBACK_MESSAGE_ERROR       21
#define MQTT_UNSUBSCRIBE_MESSAGE_ERROR  22
#define MQTT_FIXED_HEADER_RESERVED_ERROR 23
#define MQTT_BROKER_HOSTNAME_ERROR      24
#define MQTT_BROKER_CONNECT_ERROR       25
#define MQTT_MESSAGE_SEND_ERROR         26
#define MQTT_CLIENT_CONNECT_ERROR       27
#define MQTT_MESSAGE_RECV_ERROR          28




#define MQTT_KEEP_ALIVE_DEFAULT 60

#endif

