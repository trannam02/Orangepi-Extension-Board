/*
 * queue_utils.c
 *
 *  Created on: Mar 17, 2026
 *      Author: Nam
 */
#include "queue_utils.h"

void Queue_Init(MessageQueue_t *q, uint16_t max_size) {
    q->head = 0; q->tail = 0; q->count = 0;
    q->max_size = max_size;
}

// Thêm gói tin vào Queue (trả về false nếu đầy)
bool Queue_Push(MessageQueue_t *q, uint8_t *data, uint8_t len) {
    if (q->count >= q->max_size) return false;
    memcpy(q->buffer[q->head], data, len);
    q->head = (q->head + 1) % q->max_size;
    q->count++;
    return true;
}

// Lấy gói tin ra khỏi Queue
bool Queue_Pop(MessageQueue_t *q, uint8_t *out_data) {
    if (q->count == 0) return false;
    // Giả định out_data đủ lớn (MAX_BUFFER_LEN)
    memcpy(out_data, q->buffer[q->tail], MAX_BUFFER_LEN);
    q->tail = (q->tail + 1) % q->max_size;
    q->count--;
    return true;
}
