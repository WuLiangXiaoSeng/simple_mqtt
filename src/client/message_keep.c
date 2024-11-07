#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "message_keep.h"
#include "mqtt.h"

int message_send_add(message_send_t **list, uint16_t packet_id, uint8_t *message, uint32_t message_len, uint8_t qos)
{
    if (list == NULL) {
        return MQTT_INVALID_PARAM;
    }
    
    message_send_t *node = (message_send_t *)malloc(sizeof(message_send_t));
    if (node == NULL) {
        return MQTT_MEMORY_NOBUFFER;
    }

    memset(node, 0, sizeof(message_send_t));

    node->packet_id = packet_id;
    node->message = (uint8_t *)malloc(message_len);
    if (node->message == NULL) {
        free(node);
        return MQTT_MEMORY_NOBUFFER;
    }
    memcpy(node->message, message, message_len);
    node->message_len = message_len;
    node->qos = qos;
    node->next = NULL;

    if (*list == NULL) {
        *list = node;
        return MQTT_SUCCESS;
    }
    message_send_t *curr = *list;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = node;
    return MQTT_SUCCESS;
}


int message_send_find(message_send_t *list, uint16_t packet_id, message_send_t *node)
{
    message_send_t *curr = list;
    while (curr != NULL) {
        if (curr->packet_id == packet_id) {
            node = curr;
            return MQTT_SUCCESS;
        }
        curr = curr->next;
    }

    return MQTT_UNKNOWN;
}


int message_send_remove(message_send_t **list, uint16_t packet_id)
{
    if (list == NULL) {
        return MQTT_INVALID_PARAM;
    }

    message_send_t *curr, *next;
    curr = *list;
    
    if ((*list)->packet_id == packet_id) {
        *list = (*list)->next;
        free(curr);
        return MQTT_SUCCESS;
    }
    
    while (curr->next != NULL) {
        if (curr->next->packet_id == packet_id) {
            next = curr->next;
            curr->next = next->next;
            free(next);
            return MQTT_SUCCESS;
        }
        curr = curr->next;
    }

    return MQTT_UNKNOWN;
}