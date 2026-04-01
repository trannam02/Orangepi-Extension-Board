/*
 * queue_utils.h
 *
 *  Created on: Mar 17, 2026
 *      Author: Nam
 */

#ifndef INC_QUEUE_UTILS_H_
#define INC_QUEUE_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_BUFFER_LEN 64 // Thay đổi theo kích thước RX_MAX_BUFFER_SIZE của bạn

typedef struct {
    uint8_t buffer[10][MAX_BUFFER_LEN]; // 32 là MAX_QUEUE_SIZE, có thể tùy chỉnh
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint16_t max_size;
} MessageQueue_t;

// Khởi tạo queue
void Queue_Init(MessageQueue_t *q, uint16_t max_size);

// Thêm gói tin vào Queue (trả về false nếu đầy)
bool Queue_Push(MessageQueue_t *q, uint8_t *data, uint8_t len);

// Lấy gói tin ra khỏi Queue
bool Queue_Pop(MessageQueue_t *q, uint8_t *out_data);

#endif /* INC_QUEUE_UTILS_H_ */
