#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include <arpa/inet.h>


#include "msg_connect.h"
#include "common.h"
#include "log.h"
#include "mqtt.h"
#include "message.h"

/* 暂时只支持最大23字节的id */
#define MQTT_CLIENT_ID_LEN_MAX 23

#define MQTT_CONNECT_KEEPALIVE_MAX 0xFFFF
#define MQTT_CONNECT_KEEPALIVE_DEFAULT 60

#define bit_set(flags, index) \
    flags |= (1 << index);

#define bit_clean(flags, index) \
    flags &= ~(1 << index);

#define bit_is_set(flags, index) \
    ((flags >> index) & 1)

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
                           uint8_t *message, uint32_t *message_len)
{
    int ret = MQTT_SUCCESS;
    
    if (conn_payload->client_id_len > MQTT_CLIENT_ID_LEN_MAX) {
        mqtt_log_error("Invalid client id len %d", conn_payload->client_id_len);
        return MQTT_INVALID_PARAM;
    }
    if (conn_payload->client_id != NULL && conn_payload->client_id_len > MQTT_CLIENT_ID_LEN_MAX)
    {
        mqtt_log_error("Invalid client id");
        return MQTT_INVALID_PARAM;
    }
    
    /* 构建可变头信息 */
    connect_variable_header_t var_header;
    memset(&var_header, 0, sizeof(var_header));
    var_header.protocol_name_length[1] = 4;
    memcpy(var_header.protocol_name, "MQTT", 4);
    var_header.protocol_level = 4;
    /* keep alive: 0~65535； 当keep_alive为0，代表关闭保持连接功能 */
    keep_alive = htons(keep_alive);
    memcpy(var_header.keep_alive, &keep_alive, 2);

    uint8_t *payload;
    uint32_t payload_len = 0;
    uint32_t field_len;
    uint32_t remain_length;
    payload_len += (conn_payload->client_id_len + 2);
    uint8_t message_type_with_reversed;
    
    if (clean_session) {
        // var_header.flags |= 0x01 << 1;
        bit_set(var_header.flags, CONNECT_FLAG_INDEX_CLEAN_SESSION);
    }
    
    if (conn_payload->will_topic != NULL && conn_payload->will_topic_len > 0 && \
            conn_payload->will_msg != NULL && conn_payload->will_msg_len > 0) {
        // var_header.flags |= 0x01 << 2;
        bit_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_FLAG);
        payload_len += (conn_payload->will_topic_len + 2);
        payload_len += (conn_payload->will_msg_len + 2);
    }
    
    /* 遗嘱QoS：0, 1, 2, 只有当遗嘱标志置为1时，才需要设置 */
    if (bit_is_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_FLAG)) {
        switch (will_qos) {
            case 0:
                // var_header.flags |= 0b00 << 3;
                bit_clean(var_header.flags, CONNECT_FLAG_INDEX_WILL_QOS_LOW);
                bit_clean(var_header.flags, CONNECT_FLAG_INDEX_WILL_QOS_HIGH);
                break;
            case 1:
                // var_header.flags |= 0b01 << 3;
                bit_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_QOS_LOW);
                bit_clean(var_header.flags, CONNECT_FLAG_INDEX_WILL_QOS_HIGH)
                break;
            case 2:
                // var_header.flags |= 0b10 << 3;
                bit_clean(var_header.flags, CONNECT_FLAG_INDEX_WILL_QOS_LOW);
                bit_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_QOS_HIGH)
                break;
            default:
                mqtt_log_debug("invalid will qos %d", will_qos)
                break;
        }
    }
    
    /* 遗嘱保留标志：只有当遗嘱标志置为1时，才需要设置*/
    if (will_retain && bit_is_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_FLAG)) {
        // var_header.flags |= 0x01 << 5;
        bit_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_RETAIN);
    }
        
    if (conn_payload->username != NULL && conn_payload->username_len > 0 && \
            conn_payload->password != NULL && conn_payload->password_len > 0) {
        // var_header.flags |= 0x01 << 6; 
        bit_set(var_header.flags, CONNECT_FLAG_INDEX_PASSWD);
        payload_len += (conn_payload->password_len + 2);
    }
    
    if (conn_payload->username != NULL && conn_payload->username_len > 0) {
        // var_header.flags |= 0x01 << 7;
        bit_set(var_header.flags, CONNECT_FLAG_INDEX_USERNAME);
        payload_len += (conn_payload->username_len + 2);
    }

    mqtt_log_debug("payload len %u", payload_len);

    payload = (uint8_t *)malloc(payload_len);
    if (payload == NULL) {
        mqtt_log_error("connect payload malloc failed");
        return MQTT_MEMORY_NOBUFFER;
    }
    uint8_t *ptr = payload;
    payload_len = 0; /* 载荷长度清零，上面各个字段最后不一定能够放到最后的载荷中，因此需要重新计算载荷的长度 */

    /* 构建client id字段 */
    ret = build_string_field(conn_payload->client_id, conn_payload->client_id_len, payload, &field_len);
    if (ret != MQTT_SUCCESS)
    {
        mqtt_log_error("Build client id payload failed");
        goto exit;
        // return ret;
    }
    ptr += field_len;
    payload_len += field_len;
    
    
    if (bit_is_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_FLAG)) {
        ret = build_string_field(conn_payload->will_topic, conn_payload->will_topic_len, ptr, &field_len);
        if (ret != MQTT_SUCCESS)
        {
            mqtt_log_error("Build will topic payload failed");
            goto exit;
            // return ret;
        }
        ptr += field_len;
        payload_len += field_len;
    }
    
    if (bit_is_set(var_header.flags, CONNECT_FLAG_INDEX_WILL_FLAG)) {
        ret = build_string_field(conn_payload->will_msg, conn_payload->will_msg_len, ptr, &field_len);
        if (ret != MQTT_SUCCESS)
        {
            mqtt_log_error("Build will msg payload failed");
            goto exit;
            // return ret;
        }
        ptr += field_len;
        payload_len += field_len;
    }
    
    if (bit_is_set(var_header.flags, CONNECT_FLAG_INDEX_USERNAME)) {
        ret = build_string_field(conn_payload->username, conn_payload->username_len, ptr, &field_len);
        if (ret != MQTT_SUCCESS)
        {
            mqtt_log_error("Build username payload failed");
            goto exit;
            // return ret;
        }
        ptr += field_len;
        payload_len += field_len;
    }
    

    if (bit_is_set(var_header.flags, CONNECT_FLAG_INDEX_PASSWD)) {
        ret = build_string_field(conn_payload->password, conn_payload->password_len, ptr, &field_len);
        if (ret != MQTT_SUCCESS)
        {
            mqtt_log_error("Build password payload failed");
            goto exit;
            // return ret;
        }
        ptr += field_len;
        payload_len += field_len;
    }

    /* 构建 CONNECT 报文 */
    ptr = message;

    message_type_with_reversed = (uint8_t)message_type_encode(MQTT_MESSAGE_TYPE_CONNECT);
    memcpy(ptr, message_type_with_reversed, 1);
    ptr += 1;
    *message_len += 1;

    remain_length = payload_len + sizeof(var_header);
    
    ret = remain_length_encode(remain_length, ptr, &field_len);
    if (ret != MQTT_SUCCESS || field_len > MQTT_REMAIN_LEN_ENCODE_MAX_LENGTH) {
        mqtt_log_error("remain length encode failed, remain length %u", remain_length);
        goto exit;
    }
    ptr += field_len;
    *message_len += field_len;

    memcpy(ptr, &var_header, sizeof(var_header));
    ptr += sizeof(var_header);
    *message_len += sizeof(var_header);

    memcpy(ptr, payload, payload_len);
    ptr += payload_len;
    *message_len += payload_len;
    
    // return MQTT_SUCCESS;
    ret = MQTT_SUCCESS;
    
exit:
    free(payload);
    return ret;
}


int connect_message_parse(uint8_t *message, uint32_t message_len,
                          connect_payload_t *conn_payload,
                          uint16_t *keep_alive,
                          uint8_t *clean_session,
                          uint8_t *will_retain,
                          uint8_t *will_qos)
{
    uint8_t *ptr;
    uint32_t remain_length;
    uint32_t field_len;
    int ret;
    
    ptr = message;
    
    if (*ptr != message_type_encode(MQTT_MESSAGE_TYPE_CONNECT)) {
        mqtt_log_error("this message is not a CONNECT message.");
        return MQTT_MESSAGE_TYPE_ERROR;
    }
    ptr += 1;
    message_len -= 1;
    field_len = message_len;
    
    ret = remain_length_decode(ptr, &field_len, &remain_length);
    if (ret != MQTT_SUCCESS) {
        mqtt_log_error("remain length decode failed");
        return ret;
    }
    ptr += field_len;
    message_len -= field_len;
    
    mqtt_log_debug("remain length: %u, message_len: %u", remain_length, message_len);
    if (remain_length != message_len) {
        mqtt_log_error("remain length error");
        return MQTT_INVALID_PARAM;
    }

    /* 解析/检查可变报文头 */
    /**
     * 以下对于协议名称长度、协议名称的检查，按照3.1.1文档要求，不符合时，是可以接受的
     * 但为了简便，此处一律拒绝。
    */
    connect_variable_header_t *var_header = (connect_variable_header_t *)ptr;
    if (var_header->protocol_name_length[0] != 0x00 || var_header->protocol_name_length[1] != 0x04) {
        mqtt_log_error("protocol name length error");
        return MQTT_PROTOCOL_UNSUPPORT;
    }
    
    if (memcmp(var_header->protocol_name, CONECT_PROTOCOL_NAME, CONECT_PROTOCOL_NAME_LEN) != 0) {
        mqtt_log_error("protocol name is not %s", CONECT_PROTOCOL_NAME);
        return MQTT_PROTOCOL_UNSUPPORT;
    }

    if (var_header->protocol_level != CONECT_PROTOCOL_LEVEL) {
        mqtt_log_error("protocol level is not 4");
        return MQTT_PROTOCOL_LEVEL_UNSUPPORT;
    }
    
    uint8_t  username_flag, password_flag, will_flag;
    username_flag = password_flag = *will_retain = will_flag = *clean_session = 0;
    
    username_flag = bit_is_set(var_header->flags, CONNECT_FLAG_INDEX_USERNAME);
    password_flag = bit_is_set(var_header->flags, CONNECT_FLAG_INDEX_PASSWD);
    *will_retain = bit_is_set(var_header->flags, CONNECT_FLAG_INDEX_WILL_RETAIN);
    *will_qos = bit_is_set(var_header->flags, CONNECT_FLAG_INDEX_WILL_QOS_HIGH) << 1;
    *will_qos += bit_is_set(var_header->flags, CONNECT_FLAG_INDEX_WILL_QOS_LOW);
    if (*will_qos > 2) {
        mqtt_log_error("will qos error, %u", *will_qos);
        *will_qos = 0;
    }
    will_flag = bit_is_set(var_header->flags, CONNECT_FLAG_INDEX_WILL_FLAG);
    *clean_session = bit_is_set(var_header->flags, CONNECT_FLAG_INDEX_CLEAN_SESSION);
    
    *keep_alive = 0;
    memcpy(keep_alive, var_header->keep_alive, 2);
    *keep_alive = ntohs(*keep_alive);
    
    mqtt_log_debug("username_flag: %u, password_flag: %u, will_retain: %u, will_qos: %u, will_flag: %u, clean_session: %u, keep_alive: %u",
                    username_flag, password_flag, *will_retain, *will_qos, will_flag, *clean_session, *keep_alive);

    /* 检查一下标志位是否有冲突 */
    if (!will_flag && (will_retain || will_qos)) {
        mqtt_log_error("will flag is not set, but will retain or will qos is set");
        return MQTT_CONNECT_FLAGS_ERROR;
    }
    if (!username_flag && password_flag) {
        mqtt_log_error("username flag is not set, but password flag is set");
        return MQTT_CONNECT_FLAGS_ERROR;
    }
    
    ptr += sizeof(var_header);
    message_len -= sizeof(var_header);
    if (message_len < 2) {
        mqtt_log_error("message len is too short");
        return MQTT_CONNECT_MESSAGE_ERROR;
    }
    
    /**
     * payload 部分解析，每个字段都调用两次parse_string_field，第一次去该字段长度，第二次解析得到值
    */
    memset(conn_payload, 0, sizeof(connect_payload_t));
   
    /* client id */
    ret = parse_string_field(ptr, message_len, NULL, &conn_payload->client_id_len);
    if (MQTT_SUCCESS != ret) {
        mqtt_log_error("CONNECT payload parse client id failed");
        return ret;
    }
    /**
     * 根据3.1.1文档
     * - 可以接受长度为0的client id，服务器为其分配一个唯一的id，但此处拒绝此类型报文。
     * - 可以接受超过23字节的client id，但此处拒绝此类型报文。
    */
    if (conn_payload->client_id_len == 0 || conn_payload->client_id_len > 23) {
        mqtt_log_error("client id length is %u", conn_payload->client_id_len);
        return MQTT_CLIENT_ID_LENGTH_UNSUPPORT;
    }
    /* 报文剩余长度不够 client id 字段长度 */
    if (message_len - 2 < conn_payload->client_id_len) {
        mqtt_log_error("message len is too short");
        return MQTT_CONNECT_MESSAGE_ERROR;
    }
    
    conn_payload->client_id = (uint8_t *)malloc(conn_payload->client_id_len);
    if (NULL == conn_payload->client_id) {
        mqtt_log_error("malloc failed");
        ret = MQTT_MEMORY_NOBUFFER;
        goto error;
    }
    ret = parse_string_field(ptr, message_len, conn_payload->client_id, &conn_payload->client_id_len);
    if (MQTT_SUCCESS != ret) {
        mqtt_log_error("CONNECT payload parse client id failed");
        // return ret;
        goto error;
    }
    ptr += (conn_payload->client_id_len + 2);
    message_len -= (conn_payload->client_id_len + 2);
    
    /* 当遗嘱标记位位1时，下面两个字段应该是遗嘱主题和遗嘱消息 */
    if (will_flag) {
        /* will topic */
        ret = parse_string_field(ptr, message_len, NULL, &conn_payload->will_topic_len);
        if (MQTT_SUCCESS != ret) {
            mqtt_log_error("CONNECT payload parse client id failed");
            goto error;
            // return ret;
        }
        if (conn_payload->will_topic_len == 0 || conn_payload->will_topic_len > (message_len - 2)) {
            mqtt_log_error("will topic length is %u, message len: %u", conn_payload->will_topic_len, message_len);
            ret = MQTT_CONNECT_MESSAGE_ERROR;
            goto error;
            // return MQTT_CONNECT_MESSAGE_ERROR;
        }
        conn_payload->will_topic = (uint8_t *)malloc(conn_payload->will_topic_len);
        if (NULL == conn_payload->will_topic) {
            mqtt_log_error("malloc failed");
            ret = MQTT_MEMORY_NOBUFFER;
            goto error;
            // return MQTT_MEMORY_NOBUFFER;
        }
        ret = parse_string_field(ptr, message_len, conn_payload->will_topic, &conn_payload->will_topic_len);
        if (MQTT_SUCCESS != ret) {
            mqtt_log_error("CONNECT payload parse will topic failed");
            goto error;
            // return ret;
        }
        ptr += (conn_payload->will_topic_len + 2);
        message_len -= (conn_payload->will_topic_len + 2);

        /* will message */
        ret = parse_string_field(ptr, message_len, NULL, &conn_payload->will_msg_len);
        if (MQTT_SUCCESS != ret) {
            mqtt_log_error("CONNECT payload parse will message failed");
            goto error;
            // return ret;
        }
        if (conn_payload->will_msg_len == 0 || conn_payload->will_msg_len > (message_len - 2)) {
            mqtt_log_error("will message length is %u, message len: %u", conn_payload->will_msg_len, message_len);
            ret = MQTT_CONNECT_MESSAGE_ERROR;
            goto error;
            // return MQTT_CONNECT_MESSAGE_ERROR;
        }
        conn_payload->will_msg = (uint8_t *)malloc(conn_payload->will_msg_len);
        if (NULL == conn_payload->will_msg) {
            mqtt_log_error("malloc failed");
            ret = MQTT_MEMORY_NOBUFFER;
            goto error;
            // return MQTT_MEMORY_NOBUFFER;
        }
        ret = parse_string_field(ptr, message_len, conn_payload->will_msg, &conn_payload->will_msg_len);
        if (MQTT_SUCCESS != ret) {
            mqtt_log_error("CONNECT payload parse will message failed");
            goto error;
            // return ret;
        }
        ptr += (conn_payload->will_msg_len + 2);
        message_len -= (conn_payload->will_msg_len + 2);
    }

    if (username_flag) {
        /* username */
        ret = parse_string_field(ptr, message_len, NULL, &conn_payload->username_len);
        if (MQTT_SUCCESS != ret) {
            mqtt_log_error("CONNECT payload parse username failed");
            goto error;
            // return ret;
        }
        if (conn_payload->username_len == 0 || conn_payload->username_len > (message_len - 2)) {
            mqtt_log_error("username length is %u, message len: %u", conn_payload->username_len, message_len);
            ret = MQTT_CONNECT_MESSAGE_ERROR;
            goto error;
            // return MQTT_CONNECT_MESSAGE_ERROR;
        }
        conn_payload->username = (uint8_t *)malloc(conn_payload->username_len);
        if (NULL == conn_payload->username) {
            mqtt_log_error("malloc failed");
            ret = MQTT_MEMORY_NOBUFFER;
            goto error;
            // return MQTT_MEMORY_NOBUFFER;
        }
        ret = parse_string_field(ptr, message_len, conn_payload->username, &conn_payload->username_len);
        if (MQTT_SUCCESS != ret) {
            mqtt_log_error("CONNECT payload parse username failed");
            goto error;
            // return ret;
        }
        ptr += (conn_payload->username_len + 2);
        message_len -= (conn_payload->username_len + 2);
    }

    if (password_flag) {
        /* password */
        ret = parse_string_field(ptr, message_len, NULL, &conn_payload->password_len);
        if (MQTT_SUCCESS != ret) {
            mqtt_log_error("CONNECT payload parse password failed");
            goto error;
            // return ret;
        }
        if (conn_payload->password_len > (message_len - 2)) { /* 根据3.1.1文档，密码长度可以是0 */
            mqtt_log_error("password length is %u, message len: %u", conn_payload->password_len, message_len);
            ret = MQTT_CONNECT_MESSAGE_ERROR;
            goto error;
            // return MQTT_CONNECT_MESSAGE_ERROR;
        }
        if (conn_payload->password_len != 0) {
            conn_payload->password = (uint8_t *)malloc(conn_payload->password_len);
            if (NULL == conn_payload->password) {
                mqtt_log_error("malloc failed");
                ret = MQTT_MEMORY_NOBUFFER;
                goto error;
                // return MQTT_MEMORY_NOBUFFER;
            }
            ret = parse_string_field(ptr, message_len, conn_payload->password, &conn_payload->password_len);
            if (MQTT_SUCCESS != ret) {
                mqtt_log_error("CONNECT payload parse password failed");
                goto error;
                // return ret;
            }
        }
        ptr += (conn_payload->password_len + 2);
        message_len -= (conn_payload->password_len + 2);
    }

    if (message_len != 0) {
        mqtt_log_error("CONNECT payload parse failed, message len: %u", message_len);
        ret = MQTT_CONNECT_MESSAGE_ERROR;
        goto error;
        // return MQTT_CONNECT_MESSAGE_ERROR;
    }
    
    return MQTT_SUCCESS;
    
error:

    if ((conn_payload->password_len != 0) && (conn_payload->password != NULL)) {
        free(conn_payload->password);
    }

    if ((conn_payload->username_len != 0) && (conn_payload->username != NULL)) {
        free(conn_payload->username);
    }
    
    if ((conn_payload->will_msg_len != 0) && (conn_payload->will_msg != NULL)) {
        free(conn_payload->will_msg);
    }
    
    if ((conn_payload->will_topic_len != 0) && (conn_payload->will_topic != NULL)) {
        free(conn_payload->will_topic);
    }
    
    if ((conn_payload->client_id_len != 0) && (conn_payload->client_id != NULL)) {
        free(conn_payload->client_id);
    }
    
    return ret;
}