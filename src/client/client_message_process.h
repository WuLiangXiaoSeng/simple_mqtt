#ifndef __CLIENT_MESSAGE_PROCESS_H__
#define __CLIENT_MESSAGE_PROCESS_H__

int pingreq_msg_sent(deamon_context_t *ctx);
int puback_msg_sent(deamon_context_t *ctx, uint16_t packet_id);
int pubrec_msg_sent(deamon_context_t *ctx, uint16_t packet_id);
int pubrel_msg_sent(deamon_context_t *ctx, uint16_t packet_id);
int pubcomp_msg_sent(deamon_context_t *ctx, uint16_t packet_id);

int publish_msg_process(deamon_context_t *ctx, 
                        uint8_t *message, 
                        uint32_t message_len,
                        publish_callback pub_cb);
int puback_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len);
int pubrec_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len);
int pubcomp_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len);
int suback_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len);
int unsuback_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len);
int pingresp_msg_process(deamon_context_t *ctx, uint8_t *message, uint32_t message_len);


#endif