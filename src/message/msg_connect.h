#ifndef _MQTT_MESS_CONN_H_
#define _MQTT_MESS_CONN_H_

#include "common.h"
#include "message.h"

/* 暂时只支持最大23字节的id */
#define MQTT_CLIENT_ID_LEN_MAX 23

#define CONECT_PROTOCOL_NAME "MQTT"
#define CONECT_PROTOCOL_NAME_LEN 0x04
#define CONECT_PROTOCOL_LEVEL 0x04    // 3.1.1
/* 连接报文标志位索引 */
#define CONNECT_FLAG_INDEX_CLEAN_SESSION 1
#define CONNECT_FLAG_INDEX_WILL_FLAG 2
#define CONNECT_FLAG_INDEX_WILL_QOS_LOW 3
#define CONNECT_FLAG_INDEX_WILL_QOS_HIGH 4
#define CONNECT_FLAG_INDEX_WILL_RETAIN 5
#define CONNECT_FLAG_INDEX_PASSWD 6
#define CONNECT_FLAG_INDEX_USERNAME 7

/* 连接报文可变报文头 */
typedef struct connect_variable_header_ {
    uint8_t protocol_name_length[2];    /* 协议名长度，固定为 4, 即0x0004*/
    uint8_t protocol_name[4];           /* 协议名，固定为 "MQTT", utf-8编码 */
    uint8_t protocol_level;             /* mqtt 3.1.1 为 4， 即0x04 */
    uint8_t flags;                      /* 连接标志位： username | password | will_retain | will_qos | will_qos | will_flag | clean_session | reserved(0) */
    uint8_t keep_alive[2];              /* keep alive，单位为秒，客户端发送心跳的时间间隔，服务器端响应客户端的时间间隔，默认为 60s; 大端序！！！*/
}__attribute__((packed)) connect_variable_header_t;
 
/*
 * connect 报文的载荷部分
 * 其中某个字段可以没有，但如果有，必须按照顺序构建，客户端表示符 | 遗嘱主题 | 遗嘱消息 | 用户名 | 密码
 * 例如，如果flag字段为：0b11000000，表示由用户名密码，那么载荷部分应该为：用户端标识符 | 用户名 | 密码 
*/
typedef struct connect_payload_ {
    uint16_t client_id_len;
    uint8_t *client_id;
    uint16_t will_topic_len;
    uint8_t *will_topic;
    uint16_t will_msg_len;
    uint8_t *will_msg;
    uint16_t username_len;
    uint8_t *username;
    uint16_t password_len;
    uint8_t *password;
} connect_payload_t;

int connect_message_len_calc(connect_payload_t *conn_payload,
                                       uint16_t keep_alive,
                                       uint8_t clean_session,
                                       uint8_t will_retain,
                                       uint8_t will_qos,
                                       uint32_t *message_len);


/**
 * @brief 构建CONNECT报文
 * 
 * @param conn_payload->client_id           用户端ID [可选，为空时为默认值]；
 * @param conn_payload->client_id_len       用户端ID长度 [必须，]
 * @param conn_payload->will_topic          遗嘱主题 [可选]
 * @param conn_payload->will_topic_len      WilTopic长度 [will_topic 不为空时必需]
 * @param conn_payload->will_msg            遗嘱内容 [可选]
 * @param conn_payload->will_msg_len        WilMsg长度 [will_msg 不为空时必需]
 * @param conn_payload->username            用户名 [可选]
 * @param conn_payload->username_len        用户名长度 [username 不为空时必需]
 * @param conn_payload->password            密码 [可选]
 * @param conn_payload->password_len        密码长度 [password 不为空时必需]
 * @param keep_alive                        KeepAlive时间 [可选]
 * @param clean_session                     是否清除会话 [可选]
 * @param will_retain                       是否设置遗嘱保留 [可选]
 * @param will_qos                          遗嘱QOS [可选]
 * @param message                           报文指针
 * @param message_len                       报文长度指针
 * 
 * @return 成功返回0，失败返回错误码
*/
int connect_message_build(connect_payload_t *conn_payload,
                           uint16_t keep_alive,
                           uint8_t clean_session,
                           uint8_t will_retain,
                           uint8_t will_qos,
                           uint8_t *message, uint32_t *message_len);


int connect_payload_release(connect_payload_t *conn_payload);

/**
 * @brief 解析 CONNECT 报文
 * 
 * @param message       原始报文
 * @param message_len   原始报文长度
 * @param conn_payload  解析出的连接信息(参考 connect_payload_t 结构体定义)
 *                      注意结构体中所有的指针都需要在其生命周期结束时检查释放（conn_payload_release)
 * @param keep_alive    连接保活时间
 * @param clean_session 是否清除会话
 * @param will_retain   遗嘱保留标志
 * @param will_qos      遗嘱QOS等级（0， 1， 2）
 * 
 * @return 0 成功，其他失败
*/
int connect_message_parse(uint8_t *message, uint32_t message_len,
                          connect_payload_t *conn_payload,
                          uint16_t *keep_alive,
                          uint8_t *clean_session,
                          uint8_t *will_retain,
                          uint8_t *will_qos);
#endif