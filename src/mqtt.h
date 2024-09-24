


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